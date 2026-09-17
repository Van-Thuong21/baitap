#include "stm32f1xx_hal.h"

extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart1;

void NMI_Handler(void) {
}

void HardFault_Handler(void) {
    while (1) {
    }
}

void MemManage_Handler(void) {
    while (1) {
    }
}

void BusFault_Handler(void) {
    while (1) {
    }
}

void UsageFault_Handler(void) {
    while (1) {
    }
}

void SVC_Handler(void) {
}

void DebugMon_Handler(void) {
}

void PendSV_Handler(void) {
}

void SysTick_Handler(void) {
    HAL_IncTick();
}

void USART1_IRQHandler(void) {
    HAL_UART_IRQHandler(&huart1);
}

void ADC1_2_IRQHandler(void) {
    HAL_ADC_IRQHandler(&hadc1);
}