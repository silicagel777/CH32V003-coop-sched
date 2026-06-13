#include "debug.h"
#include "sched.h"
#include "time.h"

#include <setjmp.h>
#include <string.h>

#define LED1_RCC RCC_APB2Periph_GPIOC
#define LED1_PORT GPIOC
#define LED1_PIN GPIO_Pin_5

#define LED2_RCC RCC_APB2Periph_GPIOC
#define LED2_PORT GPIOC
#define LED2_PIN GPIO_Pin_6

void led1_task(void *ctx) {
  GPIO_InitTypeDef GPIO_InitStructure = {0};
  RCC_APB2PeriphClockCmd(LED1_RCC, ENABLE);
  GPIO_InitStructure.GPIO_Pin = LED1_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LED1_PORT, &GPIO_InitStructure);

  for (uint8_t i = 0; i < 8; i++) {
    GPIO_SetBits(LED1_PORT, LED1_PIN);
    sched_sleep(500);
    GPIO_ResetBits(LED1_PORT, LED1_PIN);
    sched_sleep(500);
  }
}

void led2_task(void *ctx) {
  GPIO_InitTypeDef GPIO_InitStructure = {0};
  RCC_APB2PeriphClockCmd(LED2_RCC, ENABLE);
  GPIO_InitStructure.GPIO_Pin = LED2_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LED2_PORT, &GPIO_InitStructure);

  for (;;) {
    GPIO_SetBits(LED2_PORT, LED2_PIN);
    sched_sleep(1000);
    GPIO_ResetBits(LED2_PORT, LED2_PIN);
    sched_sleep(1000);
  }
}

int main(void) {
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  time_init();
  sched_init();

  static uint32_t led_task_stack[80];
  sched_add_task(led1_task, NULL, led_task_stack, sizeof(led_task_stack));
  static uint32_t led2_task_stack[80];
  sched_add_task(led2_task, NULL, led2_task_stack, sizeof(led2_task_stack));

  for (;;) {
    sched_run();
  }
}
