#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

void LED_01Hz(void *pv){
    for(;;){
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
        vTaskDelay(5000);
    }
}

void LED_1Hz(void *pv){
    for(;;){
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_2);
        vTaskDelay(500);
    }
}

void LED_10Hz(void *pv){
    for(;;){
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3);
        vTaskDelay(50);
    }
}

int main(void)
{
    SystemCoreClockUpdate();    
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    xTaskCreate(LED_01Hz, "LED_01Hz", 128, NULL, 1, NULL);
    xTaskCreate(LED_1Hz, "LED_1Hz", 128, NULL, 1, NULL);
    xTaskCreate(LED_10Hz, "LED_10Hz", 128, NULL, 1, NULL);

    vTaskStartScheduler();

    for(;;) {}
}