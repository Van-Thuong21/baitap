#include "stm32f1xx_hal.h"

volatile uint16_t cnt1 = 0;
volatile uint16_t cnt2 = 0;
volatile uint16_t cnt3 = 0;

void SysTick_Handler(void) {
    HAL_IncTick();
    
    cnt1++;
    cnt2++;
    cnt3++;

    if (cnt1 >= 5000) {
        cnt1 = 0;
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0); // 0.1 Hz
    }
    if (cnt2 >= 500) {
        cnt2 = 0;
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1); // 1 Hz
    }
    if (cnt3 >= 50) {
        cnt3 = 0;
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_2); // 10 Hz
    }
}

void SystemClock_Config(void);
void GPIOA_Init(void);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    GPIOA_Init();

    while (1) {
        // Toàn bộ logic nhấp nháy do ngắt SysTick điều khiển
    }
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;  
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9; // 8MHz * 9 = 72MHz
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

void GPIOA_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}