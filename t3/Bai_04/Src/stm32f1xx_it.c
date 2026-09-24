#include "stm32f1xx_hal.h"

extern DMA_HandleTypeDef hdma_adc1;

void SysTick_Handler(void) {
    HAL_IncTick();
}

// Trình phục vụ ngắt DMA1 Channel 1 phục vụ Half-Transfer và Transfer-Complete cho ADC1
void DMA1_Channel1_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma_adc1);
}

void HardFault_Handler(void) {
    while (1);
}