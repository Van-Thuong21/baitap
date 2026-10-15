#include "stm32f1xx.h"

#define VECT_TAB_OFFSET  0x00    /* Vector Table offset register value */

uint32_t SystemCoreClock = 72000000U;

const uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
const uint8_t APBPrescTable[8]  = {0, 0, 0, 0, 1, 2, 3, 4};

void SystemInit(void) {
    /* Reset thanh ghi RCC về trạng thái mặc định ban đầu */
    RCC->CR |= (uint32_t)0x00000001; // Bật HSI
    RCC->CFGR &= (uint32_t)0xF8FF0000; // Reset thanh ghi CFGR
    RCC->CR &= (uint32_t)0xFEF6FFFF; // Reset HSEON, CSSON, PLLON
    RCC->CR &= (uint32_t)0xFFFBFFFF; // Reset HSEBYP
    RCC->CFGR &= (uint32_t)0xFF80FFFF; // Reset PLL source, multipliers...
    
    /* Tắt toàn bộ ngắt trong NVIC */
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET;
}

void SystemCoreClockUpdate(void) {
    SystemCoreClock = 72000000U;
}