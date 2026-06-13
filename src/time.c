#include "time.h"
#include "ch32v00x.h"

static volatile uint32_t milliseconds = 0;

void time_init(void) {
  NVIC_EnableIRQ(SysTicK_IRQn);
  SysTick->SR &= ~(1 << 0);
  SysTick->CMP = SystemCoreClock / 1000 - 1;
  SysTick->CNT = 0;
  SysTick->CTLR = (1 << 3) | (1 << 2) | (1 << 1) | (1 << 0);
}

uint32_t time_ms(void) {
  return milliseconds;
}

void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void SysTick_Handler(void) {
  milliseconds++;
  SysTick->SR = 0;
}
