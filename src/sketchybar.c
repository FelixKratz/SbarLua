#include "mach.h"
#include "parsing.h"
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdio.h>
#include <unistd.h>
#include "stack.h"

#define CMD_SUCCESS 1
#define CMD_FAILURE 0

lua_State* g_state;

#define BAR       "--bar"
#define SET       "--set"
#define ADD       "--add"
#define ANIMATE   "--animate"
#define DEFAULT   "--default"
#define SUBSCRIBE "--subscribe"
#define UPDATE    "--update"
#define QUERY     "--query"

#define MACH_HELPER_FMT "git.relay.sketchybar%d"

struct callback {
  int callback_ref;
  char* name;
  char* event;
};

struct callbacks {
  struct callback** callbacks;
  uint32_t num_callbacks;
};

struct callbacks g_callbacks;
static char* g_cmd = NULL;
static uint32_t g_cmd_len = 0;
static char g_bootstrap_name[64];
mach_port_t g_port = 0;

static inline void event_server_init(char* bootstrap_name) {
  mach_server_register(&g_mach_server, bootstrap_name);
}

static inline void event_server_run(mach_handler event_handler) {
  mach_server_begin(&g_mach_server, event_handler);
}
static char* sketchybar(struct stack* stack) {
  uint32_t message_length;
  char* message = stack_flatten_ttb(stack, &message_length);

  if (!message && !g_cmd) return NULL;
  if (g_cmd && stack) {
    g_cmd = realloc(g_cmd, g_cmd_len + message_length);
    memcpy(g_cmd + g_cmd_len, message, message_length);
    g_cmd_len = g_cmd_len + message_length;
    free(message);
    return NULL;
  } else if (g_cmd && !stack) {
    message = g_cmd;
    message_length = g_cmd_len;
  }

  if (!g_port) g_port = mach_get_bs_port("git.felix.sketchybar");
  char message_format[message_length + 1];
  memcpy(message_format, message, message_length);
  message_format[message_length] = '\0';
  char* response = mach_send_message(g_port, message_format, message_length + 1);
  if (!response) {
    g_port = mach_get_bs_port("git.felix.sketchybar");
    response = mach_send_message(g_port, message_format, message_length + 1);
  }
  return response;
}

static void transaction_create() {
  if (!g_cmd) {
    g_cmd = malloc(1);
    g_cmd_len = 0;
    *g_cmd = '\0';
  }
}

static char* transaction_commit() {
  char* response = NULL;
  if (g_cmd) {
    response = sketchybar(NULL);
    free(g_cmd);
    g_cmd_len = 0;
    g_cmd = NULL;
  }
  return response;
}

int animate(lua_State* state) {
  if (lua_gettop(state) < 3
      || lua_type(state, -1) != LUA_TFUNCTION
      || lua_type(state, -2) != LUA_TNUMBER
      || lua_type(state, -3) != LUA_TSTRING   ) {
    char error[] = "[Lua] Error: expecting a string "
                   "a number and a function as arguments for 'animate'";
    printf("%s\n", error);
    return 0;
  }
  const int duration = lua_tointeger(state, -2);
  char duration_str[4];
  snprintf(duration_str, 4, "%d", duration);
  const char* interp = lua_tostring(state, -3);

  transaction_create();

  struct stack* stack = stack_create();
  stack_init(stack);
  stack_push(stack, duration_str);
  stack_push(stack, interp);
  stack_push(stack, ANIMATE);

  char* response = sketchybar(stack);
  stack_destroy(stack);

  if (response) free(response);
  int error = lua_pcall(state, 0, 0, 0);

  if (error && lua_gettop(state)) {
    printf("[!] Lua: %s\n", lua_tostring(state, -1));
  }
  response = transaction_commit();
  if (response) free(response);
  return 0;
}

int set(lua_State* state) {
  if (lua_gettop(state) < 2
      || lua_type(state, -1) != LUA_TTABLE
      || lua_type(state, -2) != LUA_TSTRING) {
    char error[] = "[Lua] Error: expecting a string "
                   "and a table as arguments for 'set'";
    printf("%s\n", error);
    return 0;
  }
  struct stack* stack = stack_create();
  stack_init(stack);

  parse_kv_table(state, NULL, stack);
  const char* name = lua_tostring(state, -2);

  stack_push(stack, name);
  stack_push(stack, SET);
  char* response = sketchybar(stack);
  if (response) free(response);
  stack_destroy(stack);
  return 0;
}

int add(lua_State* state) {
  if (lua_gettop(state) < 3
      || lua_type(state, 1) != LUA_TSTRING
      || lua_type(state, 2) != LUA_TSTRING) {
    char error[] = "[Lua] Error: expecting at least two string arguments"
                   "for 'add'";
    printf("%s\n", error);
    return 0;
  }

  struct stack* stack = stack_create();
  stack_init(stack);
  const char* type = lua_tostring(state, 1);
  const char* name = lua_tostring(state, 2);

  if (strcmp(type,"item") == 0
      || strcmp(type, "alias") == 0
      || strcmp(type, "space") == 0
      || strcmp(type, "slider") == 0) {
    // "Regular" items with name an position
    const char* position = { "left" };
    stack_push(stack, position);
  } else if (strcmp(type, "bracket") == 0) {
    // A bracket takes a list of member items instead of a position
    if (lua_type(state, 3) != LUA_TTABLE) {
      char error[] = "[Lua] Error: expecting a lua table as third argument"
                     "for 'add', when the type is 'bracket'";
      printf("%s\n", error);
      stack_destroy(stack);
      return 0;
    }

    lua_pushnil(state);
    while (lua_next(state, 3)) {
      const char* member = lua_tostring(state, -1);
      stack_push(stack, member);
      lua_pop(state, 1);
    }
  } else {
    char error[] = "[Lua] Error: Item type not supported yet, create case for"
                    " it in src/sketchybar.c";
    printf("%s\n", error);
    stack_destroy(stack);
    return 0;
  }

  stack_push(stack, name);
  stack_push(stack, type);
  stack_push(stack, ADD);
  char* response = sketchybar(stack);
  if (response) free(response);
  stack_destroy(stack);

  // If a table is presented as the last argument, we parse it as if it
  // was passed to the set domain.
  if (lua_type(state, -1) == LUA_TTABLE) {
    lua_pushstring(state, name);
    lua_insert(state, -2);
    set(state);
  }
  lua_pushstring(state, name);
  return 1;
}

int defaults(lua_State* state) {
  if (lua_gettop(state) < 1 || lua_type(state, -1) != LUA_TTABLE) {
    char error[] = "[Lua] Error: expecting a table as an"
                   "argument to function 'defaults'";
    printf("%s\n", error);
    return 0;
  }

  struct stack* stack = stack_create();
  stack_init(stack);
  parse_kv_table(state, NULL, stack);
  stack_push(stack, DEFAULT);

  char* response = sketchybar(stack);
  stack_destroy(stack);
  if (response) free(response);
  return 0;
}

int bar(lua_State* state) {
  if (lua_gettop(state) < 1 || lua_type(state, -1) != LUA_TTABLE) {
    char error[] = "[Lua] Error: expecting a table as an "
                   "argument to function 'bar'";
    printf("%s\n", error);
    return 0;
  }
  struct stack* stack = stack_create();
  stack_init(stack);
  parse_kv_table(state, NULL, stack);
  stack_push(stack, BAR);

  char* response = sketchybar(stack);
  stack_destroy(stack);
  if (response) free(response);
  return 0;
}

void callback_function(env env) {
  char* name = env_get_value_for_key(env, "NAME");
  char* sender = env_get_value_for_key(env, "SENDER");

  for (int i = 0; i < g_callbacks.num_callbacks; i++) {
    if (strcmp(g_callbacks.callbacks[i]->name, name) == 0
        && strcmp(g_callbacks.callbacks[i]->event, sender) == 0) {

      lua_rawgeti(g_state, LUA_REGISTRYINDEX,
                           g_callbacks.callbacks[i]->callback_ref);

      lua_newtable(g_state);
      struct key_value_pair kv = { NULL, NULL };
      do {
        kv = env_get_next_key_value_pair(env, kv);
        if (kv.key && kv.value) {
          lua_pushstring(g_state, kv.key);
          if (!json_to_lua_table(g_state, kv.value)) {
            lua_pushstring(g_state, kv.value);
          }

          lua_settable(g_state,-3);
        }
      } while(kv.key && kv.value);

      transaction_create();
      int error = lua_pcall(g_state, 1, 0, 0);

      if (error && lua_gettop(g_state)) {
        printf("[!] Lua: %s\n", lua_tostring(g_state, -1));
      }
      char* response = transaction_commit();
      if (response) free(response);
      break;
    }
  }
}

int subscribe(lua_State* state) {
  if (lua_gettop(state) < 3
      || lua_type(state, -1) != LUA_TFUNCTION
      || lua_type(state, -2) != LUA_TSTRING
      || lua_type(state, -3) != LUA_TSTRING  ) {
    char error[] = "[Lua] Error: expecting two strings and a function as "
                   "arguments for 'subscribe'";

    printf("%s\n", error);
    return 0;
  }
  const char* name = lua_tostring(state, -3);
  const char* event = lua_tostring(state, -2);

  char mach_helper[strlen(g_bootstrap_name) + 16];
  snprintf(mach_helper, strlen(g_bootstrap_name) + 16, "mach_helper=%s",
                                                        g_bootstrap_name);
  char empy_script[] = { "script=" };
  char event_op[] = { "event" };

  struct stack* stack = stack_create();
  stack_init(stack);
  stack_push(stack, mach_helper);
  stack_push(stack, empy_script);
  stack_push(stack, name);
  stack_push(stack, SET);
  stack_push(stack, event);
  stack_push(stack, event_op);
  stack_push(stack, ADD);
  char* response = sketchybar(stack);
  stack_destroy(stack);
  if (response) free(response);

  int index = -1;
  for (int i = 0; i < g_callbacks.num_callbacks; i++) {
    if (strcmp(g_callbacks.callbacks[i]->name, name) == 0
        && strcmp(g_callbacks.callbacks[i]->event, event) == 0) {
      index = i;
      break;
    }
  }

  if (index < 0) {
    g_callbacks.callbacks = realloc(g_callbacks.callbacks,
                                    sizeof(struct callback*)
                                    * ++g_callbacks.num_callbacks);

    struct callback* callback = malloc(sizeof(struct callback));
    m_clone(callback->name, name);
    m_clone(callback->event, event);
    callback->callback_ref = luaL_ref(state, LUA_REGISTRYINDEX);

    g_callbacks.callbacks[g_callbacks.num_callbacks - 1] = callback;
  } else {
    g_callbacks.callbacks[index]->callback_ref = luaL_ref(state,
                                                          LUA_REGISTRYINDEX);
  }

  stack = stack_create();
  stack_init(stack);
  stack_push(stack, event);
  stack_push(stack, name);
  stack_push(stack, SUBSCRIBE);

  response = sketchybar(stack);
  stack_destroy(stack);
  if (response) free(response);
  return 0;
}

int query(lua_State* state) {
  if (lua_gettop(state) < 1
      || lua_type(state, -1) != LUA_TSTRING  ) {
    char error[] = "[Lua] Error: expecting a strings as the only argument for 'query'";
    printf("%s\n", error);
    return 0;
  }

  const char* query = lua_tostring(state, -1);
  struct stack* stack = stack_create();
  stack_init(stack);
  stack_push(stack, query);
  stack_push(stack, QUERY);
  transaction_commit();
  char* response = sketchybar(stack);
  stack_destroy(stack);
  if (response) {
    json_to_lua_table(state, response);
    free(response);
    return 1;
  }

  return 0;
}

int event_loop(lua_State* state) {
  g_state = state;
  struct stack* stack = stack_create();
  stack_init(stack);
  stack_push(stack, UPDATE);
  char* response = sketchybar(stack);
  stack_destroy(stack);
  if (response) free(response);
  alarm(0);
  event_server_run(callback_function);
  return 0;
}

static const struct luaL_Reg functions[] = {
    { "add", add },
    { "set", set },
    { "bar", bar },
    { "default", defaults },
    { "animate", animate },
    { "subscribe", subscribe },
    { "query", query },
    { "event_loop", event_loop },
    {NULL, NULL}
};

int luaopen_sketchybar(lua_State* L) {
  memset(&g_callbacks, 0, sizeof(g_callbacks));
  snprintf(g_bootstrap_name, sizeof(g_bootstrap_name), MACH_HELPER_FMT,
                                                       (int)(intptr_t)L);

  event_server_init(g_bootstrap_name);
  luaL_newlib(L, functions);

  lua_pushcfunction(L, subscribe);
  lua_setfield(L, -2, "subscribe");

  lua_pushcfunction(L, animate);
  lua_setfield(L, -2, "animate");

  lua_pushcfunction(L, defaults);
  lua_setfield(L, -2, "default");

  lua_pushcfunction(L, bar);
  lua_setfield(L, -2, "bar");

  lua_pushcfunction(L, set);
  lua_setfield(L, -2, "set");

  lua_pushcfunction(L, add);
  lua_setfield(L, -2, "add");

  lua_pushcfunction(L, event_loop);
  lua_setfield(L, -2, "update");
  return 1;
}
