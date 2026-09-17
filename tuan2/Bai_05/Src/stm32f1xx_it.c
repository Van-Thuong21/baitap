#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart1;

void SysTick_Handler(void) {
    HAL_IncTick();
}

void USART1_IRQHandler(void) {
    HAL_UART_IRQHandler(&huart1);
}