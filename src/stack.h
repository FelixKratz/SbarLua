#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define m_clone(dst, src) { \
  dst = malloc(strlen(src) + 1); \
  memcpy(dst, src, strlen(src) + 1); \
}

struct stack {
  char** value;
  uint32_t num_values;
};

static inline struct stack* stack_create() {
  return malloc(sizeof(struct stack));
}

static inline void stack_init(struct stack* stack) {
  memset(stack, 0, sizeof(struct stack));
}

static inline void stack_push(struct stack* stack, const char* value) {
  stack->value = realloc(stack->value, (sizeof(char*) * ++stack->num_values));
  m_clone(stack->value[stack->num_values - 1], value);
}

static inline void stack_copy(struct stack* stack, struct stack* copy) {
  stack_init(copy);

  for (int i = 0; i < stack->num_values; i++)
    stack_push(copy, stack->value[i]);
}


static inline void stack_pop(struct stack* stack) {
  if (stack->num_values == 0) return;

  if (stack->value[stack->num_values - 1])
    free(stack->value[stack->num_values - 1]);

  stack->value[stack->num_values - 1] = NULL;
  stack->num_values--;
}

static inline void stack_clean(struct stack* stack) {
  for (uint32_t i = 0; i < stack->num_values; i++) {
    if (stack->value[i]) free(stack->value[i]);
  }

  if (stack->value) free(stack->value);
  stack_init(stack);
}

static inline void stack_destroy(struct stack* stack) {
  stack_clean(stack);
  free(stack);
}

static inline char* stack_flatten_ttb(struct stack* stack, uint32_t* length) {
  if (!stack) return NULL;
  if (stack->num_values == 0) return NULL;
  *length = 0;
  for (int i = stack->num_values - 1; i >= 0; i--) {
    *length += strlen(stack->value[i]) + 1;
  }
  char* out = malloc(*length);
  uint32_t caret = 0;

  for (int i = stack->num_values - 1; i >= 0; i--) {
    uint32_t entry_size = strlen(stack->value[i]) + 1;
    memcpy(out + caret, stack->value[i], entry_size);
    caret += entry_size;
  }

  return out;
}

static inline void stack_print(struct stack* stack) {
  for (uint32_t i = 0; i < stack->num_values - 1; i++)
    printf("%s -> ", stack->value[i]);
  if (stack->num_values > 0)
    printf("%s\n", stack->value[stack->num_values - 1]);
}
