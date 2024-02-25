#include <mach/mach.h>
#include <mach/message.h>
#include <bootstrap.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <CoreFoundation/CoreFoundation.h>

typedef char* env;

#define MACH_HANDLER(name) void name(char* message, size_t size)
typedef MACH_HANDLER(mach_handler);

struct mach_message {
  mach_msg_header_t header;
  mach_msg_size_t msgh_descriptor_count;
  mach_msg_ool_descriptor_t descriptor;
};

struct mach_buffer {
  struct mach_message message;
  mach_msg_trailer_t trailer;
};

struct mach_server {
  mach_port_name_t task;
  mach_port_t port;
  mach_port_t bs_port;
  mach_handler* handler;
};

struct key_value_pair {
  char* key;
  char* value;
};

static struct mach_server g_mach_server;
static mach_port_t g_mach_port = 0;

static inline char* env_get_value_for_key(env env, char* key) {
  uint32_t caret = 0;
  for(;;) {
    if (!env[caret]) break;
    if (strcmp(&env[caret], key) == 0)
      return &env[caret + strlen(&env[caret]) + 1];

    caret += strlen(&env[caret])
             + strlen(&env[caret + strlen(&env[caret]) + 1])
             + 2;
  }
  return (char*)"";
}

static inline struct key_value_pair env_get_next_key_value_pair(env env, struct key_value_pair prev) {
  uint32_t caret = 0;
  if (prev.key != NULL) {
    caret = (prev.key - env) + strlen(&env[(prev.key - env)])
                             + strlen(&env[(prev.key - env)
                                      + strlen(&env[(prev.key - env)])
                                      + 1                             ])
                             + 2;
  }

  if (!env[caret]) return (struct key_value_pair) { NULL, NULL };

  return (struct key_value_pair) { env + caret,
                                   env + caret + strlen(&env[caret]) +1 };
}

static inline mach_port_t mach_get_bs_port(char* name) {
  mach_port_name_t task = mach_task_self();

  mach_port_t bs_port;
  if (task_get_special_port(task,
                            TASK_BOOTSTRAP_PORT,
                            &bs_port            ) != KERN_SUCCESS) {
    return 0;
  }

  mach_port_t port;
  if (bootstrap_look_up(bs_port,
                        name,
                        &port   ) != KERN_SUCCESS) {
    return 0;
  }

  return port;
}

static inline void mach_receive_message(mach_port_t port, struct mach_buffer* buffer, bool timeout) {
  *buffer = (struct mach_buffer) { 0 };
  mach_msg_return_t msg_return;
  if (timeout)
    msg_return = mach_msg(&buffer->message.header,
                          MACH_RCV_MSG | MACH_RCV_TIMEOUT,
                          0,
                          sizeof(struct mach_buffer),
                          port,
                          1000,
                          MACH_PORT_NULL                  );
  else 
    msg_return = mach_msg(&buffer->message.header,
                          MACH_RCV_MSG,
                          0,
                          sizeof(struct mach_buffer),
                          port,
                          MACH_MSG_TIMEOUT_NONE,
                          MACH_PORT_NULL             );

  if (msg_return != MACH_MSG_SUCCESS) {
    buffer->message.descriptor.address = NULL;
  }
}

static inline char* mach_send_message(mach_port_t port, char* message, uint32_t len, bool response) {
  if (!message || !port) {
    return NULL;
  }

  mach_port_t response_port;
  mach_port_name_t task = 0;
  if (response) {
    task = mach_task_self();
    if (mach_port_allocate(task, MACH_PORT_RIGHT_RECEIVE,
                                 &response_port          ) != KERN_SUCCESS) {
      return NULL;
    }

    if (mach_port_insert_right(task, response_port,
                                     response_port,
                                     MACH_MSG_TYPE_MAKE_SEND)!= KERN_SUCCESS) {
      return NULL;
    }
  }

  struct mach_message msg = { 0 };
  msg.header.msgh_remote_port = port;
  if (response) {
    msg.header.msgh_local_port = response_port;
    msg.header.msgh_id = response_port;
  }
  msg.header.msgh_bits = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_COPY_SEND,
                                            MACH_MSG_TYPE_MAKE_SEND,
                                            0,
                                            MACH_MSGH_BITS_COMPLEX  );

  msg.header.msgh_size = sizeof(struct mach_message);
  msg.msgh_descriptor_count = 1;
  msg.descriptor.address = message;
  msg.descriptor.size = len * sizeof(char);
  msg.descriptor.copy = MACH_MSG_VIRTUAL_COPY;
  msg.descriptor.deallocate = false;
  msg.descriptor.type = MACH_MSG_OOL_DESCRIPTOR;

  mach_msg_return_t ret = mach_msg(&msg.header,
                                   MACH_SEND_MSG,
                                   sizeof(struct mach_message),
                                   0,
                                   MACH_PORT_NULL,
                                   MACH_MSG_TIMEOUT_NONE,
                                   MACH_PORT_NULL              );
  
  if (ret != KERN_SUCCESS) return NULL;

  char* rsp = NULL;
  if (response) {
    struct mach_buffer buffer = { 0 };
    mach_receive_message(response_port, &buffer, true);

    if (buffer.message.descriptor.address) {
      rsp = malloc(strlen(buffer.message.descriptor.address) + 1);
      memcpy(rsp, buffer.message.descriptor.address,
                  strlen(buffer.message.descriptor.address) + 1);
    } else {
      rsp = malloc(1);
      *rsp = '\0';
    }

    mach_msg_destroy(&buffer.message.header);
    mach_port_mod_refs(task, response_port, MACH_PORT_RIGHT_RECEIVE, -1);
    mach_port_deallocate(task, response_port);
  }

  return rsp;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static inline bool mach_server_register(struct mach_server* mach_server, char* bootstrap_name) {
  mach_server->task = mach_task_self();

  if (mach_port_allocate(mach_server->task,
                         MACH_PORT_RIGHT_RECEIVE,
                         &mach_server->port      ) != KERN_SUCCESS) {
    return false;
  }

  struct mach_port_limits limits = {};
  limits.mpl_qlimit = MACH_PORT_QLIMIT_LARGE;

  if (mach_port_set_attributes(mach_server->task,
                               mach_server->port,
                               MACH_PORT_LIMITS_INFO,
                               (mach_port_info_t)&limits,
                               MACH_PORT_LIMITS_INFO_COUNT) != KERN_SUCCESS) {
    return false;
  }

  if (mach_port_insert_right(mach_server->task,
                             mach_server->port,
                             mach_server->port,
                             MACH_MSG_TYPE_MAKE_SEND) != KERN_SUCCESS) {
    return false;
  }

  if (task_get_special_port(mach_server->task,
                            TASK_BOOTSTRAP_PORT,
                            &mach_server->bs_port) != KERN_SUCCESS) {
    return false;
  }

  if (bootstrap_register(mach_server->bs_port,
                         bootstrap_name,
                         mach_server->port    ) != KERN_SUCCESS) {
    return false;
  }

  return true;
}

void mach_message_callback(CFMachPortRef port, void* message, CFIndex size, void* context) {
  struct mach_server* mach_server = context;
  struct mach_buffer buffer;
  buffer.message = *(struct mach_message*)message;

  if (!buffer.message.descriptor.address) return;
  if (*(char*)buffer.message.descriptor.address == 'k'
      && buffer.message.descriptor.size == 2) {
    exit(0);
  }

  mach_server->handler(buffer.message.descriptor.address,
                       buffer.message.descriptor.size    );

  mach_msg_destroy(&buffer.message.header);
}

static inline bool mach_server_begin(struct mach_server* mach_server, mach_handler handler) {
  mach_server->handler = handler;

  CFMachPortContext context = {0, (void*)mach_server};
  CFMachPortRef cf_mach_port = CFMachPortCreateWithPort(NULL,
                                                        mach_server->port,
                                                        mach_message_callback,
                                                        &context,
                                                        false                );

  CFRunLoopSourceRef source = CFMachPortCreateRunLoopSource(NULL,
                                                            cf_mach_port,
                                                            0            );

  CFRunLoopAddSource(CFRunLoopGetMain(), source, kCFRunLoopDefaultMode);
  CFRelease(source);
  CFRelease(cf_mach_port);

  return true;
}
#pragma clang diagnostic pop
