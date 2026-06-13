#include "sched.h"
#include "debug.h"
#include "time.h"

#include <setjmp.h>

typedef enum {
  TASK_STATE_RESUME = 0,
  TASK_STATE_YIELD = 1,
  TASK_STATE_DONE = 2,
} sched_state_t;

typedef struct {
  jmp_buf jmp;
  sched_cb_t cb;
  void *ctx;
} sched_task_t;

static sched_task_t g_tasks[SCHED_MAX_TASKS];
static uint8_t g_task_count = 0;
static uint8_t g_task_index = 0;
static bool g_in_task = false;
static jmp_buf g_sched_jmp;
static void *g_prev_sp;

// #define SCHED_READ_SP(VALUE) asm volatile("mv %0, sp" : "=r"(VALUE))
// #define SCHED_WRITE_SP(VALUE) asm volatile("mv sp, %0" : : "r"(VALUE))
register void *g_sp asm("sp");

void sched_init(void) {}

void sched_add_task(sched_cb_t cb, void *ctx, uint32_t *stack, uint32_t stack_bytes) {
  if (g_task_count == SCHED_MAX_TASKS) {
    return;
  }

  // Change stack pointer
  g_prev_sp = g_sp;
  g_sp = stack + stack_bytes / sizeof(stack[0]);

  if (!setjmp(g_tasks[g_task_count].jmp)) {
    // `jmp_buf` for task is set up, restore stack pointer
    g_sp = g_prev_sp;
    // Finish adding the new task
    g_tasks[g_task_count].cb = cb;
    g_tasks[g_task_count].ctx = ctx;
    g_task_count++;
  } else {
    // Stack pointer changed here, can't use local variables
    // Start task
    g_tasks[g_task_index].cb(g_tasks[g_task_index].ctx);
    // Task finished, go back to `sched_run` one last time
    longjmp(g_sched_jmp, TASK_STATE_DONE);
  }
}

void sched_yield(void) {
  if (g_in_task && !setjmp(g_tasks[g_task_index].jmp)) {
    longjmp(g_sched_jmp, TASK_STATE_YIELD);
  }
}

void sched_sleep(uint32_t delay) {
  uint32_t start_time = time_ms();
  while (time_ms() - start_time < delay) {
    sched_yield();
  }
}

void sched_run(void) {
  if (g_task_count > 0) {
    switch (setjmp(g_sched_jmp)) {
    case TASK_STATE_RESUME:
      g_in_task = true;
      longjmp(g_tasks[g_task_index].jmp, 1);
      break;
    case TASK_STATE_YIELD:
      g_in_task = false;
      g_task_index++;
      break;
    case TASK_STATE_DONE:
      for (uint8_t i = g_task_index; i < g_task_count - 1; i++) {
        g_tasks[i] = g_tasks[i + 1];
      }
      g_task_count--;
      break;
    }
    if (g_task_index >= g_task_count) {
      g_task_index = 0;
    }
  }
}
