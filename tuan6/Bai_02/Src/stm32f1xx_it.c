#include "stm32f1xx_hal.h"

// Trình phục vụ ngắt cho đường EXTI Line 0 (chân PB0)
void EXTI0_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void HardFault_Handler(void) {
    while (1);
}