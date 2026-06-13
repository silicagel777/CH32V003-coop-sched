#pragma once

#include <stdbool.h>
#include <stdint.h>

#define SCHED_MAX_TASKS 8

typedef void (*sched_cb_t)(void *ctx);

void sched_init(void);
void sched_add_task(sched_cb_t cb, void *ctx, uint32_t *stack, uint32_t stack_bytes);
void sched_yield(void);
void sched_sleep(uint32_t delay);
void sched_run(void);
