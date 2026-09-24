#include "stm32f1xx_hal.h"

extern DMA_HandleTypeDef hdma_usart1_tx;
extern UART_HandleTypeDef huart1; // Thêm dòng khai báo biến UART từ main.c

void SysTick_Handler(void) {
    HAL_IncTick();
}

// Ngắt DMA1 Channel 4 phục vụ truyền UART bằng DMA
void DMA1_Channel4_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma_usart1_tx);
}

// Thêm ngắt USART1 để reset trạng thái của thư viện HAL sau khi DMA truyền xong
void USART1_IRQHandler(void) {
    HAL_UART_IRQHandler(&huart1);
}

void HardFault_Handler(void) {
    while (1);
}