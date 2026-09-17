#include "stm32f1xx_hal.h"

// Khai báo các biến đếm thời gian
volatile uint32_t count_led1 = 0; // Đếm cho LED 0.1Hz
volatile uint32_t count_led2 = 0; // Đếm cho LED 1Hz
volatile uint32_t count_led3 = 0; // Đếm cho LED 10Hz
/*
void SysTick_Handler(void) {
    HAL_IncTick(); // Đảm bảo giữ nhịp HAL chuẩn 1ms

    // LED 3 (10Hz): Chu kỳ 100ms -> Đảo trạng thái mỗi 50ms
    count_led3++;
    if (count_led3 >= 50) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3);
        count_led3 = 0;
    }

    // LED 2 (1Hz): Chu kỳ 1000ms -> Đảo trạng thái mỗi 500ms
    count_led2++;
    if (count_led2 >= 500) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_2);
        count_led2 = 0;
    }

    // LED 1 (0.1Hz): Chu kỳ 10000ms (10s) -> Đảo trạng thái mỗi 5000ms (5s)
    count_led1++;
    if (count_led1 >= 5000) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
        count_led1 = 0;
    }
}
*/
void HardFault_Handler(void) {
    while (1);
}