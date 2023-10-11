#pragma once
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h> 
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cJSON.h"
#include "stack.h"

void parse_kv_table(lua_State* state, char* prefix, struct stack* stack);
void parse_table_values_to_stack(lua_State* state, int index, struct stack* stack);
bool json_to_lua_table(lua_State* state, const char* json_str);

