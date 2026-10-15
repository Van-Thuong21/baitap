#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Định nghĩa phần cứng Timer & Chân cắm
TIM_HandleTypeDef htim2;

#define BTN_SELECT_PIN    GPIO_PIN_0
#define BTN_SELECT_PORT   GPIOB
#define BTN_UP_PIN        GPIO_PIN_1
#define BTN_UP_PORT       GPIOB
#define BTN_DOWN_PIN      GPIO_PIN_10
#define BTN_DOWN_PORT     GPIOB

// 3 Queue độc lập gửi tới 3 Task LED
QueueHandle_t q_led1 = NULL;
QueueHandle_t q_led2 = NULL;
QueueHandle_t q_led3 = NULL;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);

// Task 1: Điều khiển LED 1
void vLed1Task(void *pvParameters) {
    uint8_t duty = 0;
    for (;;) {
        // Chờ dữ liệu mới từ Queue (chặn hoàn toàn, không tốn CPU)
        if (xQueueReceive(q_led1, &duty, portMAX_DELAY) == pdTRUE) {
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty * 10);
        }
    }
}

// Task 2: Điều khiển LED 2
void vLed2Task(void *pvParameters) {
    uint8_t duty = 0;
    for (;;) {
        if (xQueueReceive(q_led2, &duty, portMAX_DELAY) == pdTRUE) {
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, duty * 10);
        }
    }
}

// Task 3: Điều khiển LED 3
void vLed3Task(void *pvParameters) {
    uint8_t duty = 0;
    for (;;) {
        if (xQueueReceive(q_led3, &duty, portMAX_DELAY) == pdTRUE) {
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, duty * 10);
        }
    }
}

// Task 4: Đọc 3 nút nhấn và dispatch dữ liệu sang các Queue
void vButtonTask(void *pvParameters) {
    uint8_t current_selected_led = 0; // 0: LED1, 1: LED2, 2: LED3
    uint8_t led_duty[3] = {20, 20, 20}; // Khởi tạo ban đầu sáng 20%

    // Gửi giá trị khởi tạo ban đầu cho cả 3 LED
    xQueueSend(q_led1, &led_duty[0], 0);
    xQueueSend(q_led2, &led_duty[1], 0);
    xQueueSend(q_led3, &led_duty[2], 0);

    for (;;) {
        // Nút 1: Chuyển LED được chọn (PB0)
        if (HAL_GPIO_ReadPin(BTN_SELECT_PORT, BTN_SELECT_PIN) == GPIO_PIN_RESET) {
            vTaskDelay(pdMS_TO_TICKS(20)); // Chống rung phím
            if (HAL_GPIO_ReadPin(BTN_SELECT_PORT, BTN_SELECT_PIN) == GPIO_PIN_RESET) {
                current_selected_led = (current_selected_led + 1) % 3;
                while (HAL_GPIO_ReadPin(BTN_SELECT_PORT, BTN_SELECT_PIN) == GPIO_PIN_RESET) {
                    vTaskDelay(pdMS_TO_TICKS(20));
                }
            }
        }

        // Nút 2: Tăng độ sáng (PB1)
        if (HAL_GPIO_ReadPin(BTN_UP_PORT, BTN_UP_PIN) == GPIO_PIN_RESET) {
            vTaskDelay(pdMS_TO_TICKS(20));
            if (HAL_GPIO_ReadPin(BTN_UP_PORT, BTN_UP_PIN) == GPIO_PIN_RESET) {
                if (led_duty[current_selected_led] <= 90) {
                    led_duty[current_selected_led] += 10;
                } else {
                    led_duty[current_selected_led] = 100;
                }

                // Gửi giá trị mới vào Queue tương ứng
                if (current_selected_led == 0) xQueueSend(q_led1, &led_duty[0], 0);
                else if (current_selected_led == 1) xQueueSend(q_led2, &led_duty[1], 0);
                else xQueueSend(q_led3, &led_duty[2], 0);

                while (HAL_GPIO_ReadPin(BTN_UP_PORT, BTN_UP_PIN) == GPIO_PIN_RESET) {
                    vTaskDelay(pdMS_TO_TICKS(20));
                }
            }
        }

        // Nút 3: Giảm độ sáng (PB10)
        if (HAL_GPIO_ReadPin(BTN_DOWN_PORT, BTN_DOWN_PIN) == GPIO_PIN_RESET) {
            vTaskDelay(pdMS_TO_TICKS(20));
            if (HAL_GPIO_ReadPin(BTN_DOWN_PORT, BTN_DOWN_PIN) == GPIO_PIN_RESET) {
                if (led_duty[current_selected_led] >= 10) {
                    led_duty[current_selected_led] -= 10;
                } else {
                    led_duty[current_selected_led] = 0;
                }

                // Gửi giá trị mới vào Queue tương ứng
                if (current_selected_led == 0) xQueueSend(q_led1, &led_duty[0], 0);
                else if (current_selected_led == 1) xQueueSend(q_led2, &led_duty[1], 0);
                else xQueueSend(q_led3, &led_duty[2], 0);

                while (HAL_GPIO_ReadPin(BTN_DOWN_PORT, BTN_DOWN_PIN) == GPIO_PIN_RESET) {
                    vTaskDelay(pdMS_TO_TICKS(20));
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20)); // Chu kỳ quét lặp lại 20ms
    }
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init();

    // Bắt đầu phát xung PWM trên cả 3 kênh
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);

    // Khởi tạo các hàng đợi (độ dài 5 phần tử, mỗi phần tử 1 byte uint8_t)
    q_led1 = xQueueCreate(5, sizeof(uint8_t));
    q_led2 = xQueueCreate(5, sizeof(uint8_t));
    q_led3 = xQueueCreate(5, sizeof(uint8_t));

    if (q_led1 != NULL && q_led2 != NULL && q_led3 != NULL) {
        // Tạo 3 Task điều khiển 3 LED
        xTaskCreate(vLed1Task, "LED1_Task", 128, NULL, 1, NULL);
        xTaskCreate(vLed2Task, "LED2_Task", 128, NULL, 1, NULL);
        xTaskCreate(vLed3Task, "LED3_Task", 128, NULL, 1, NULL);

        // Tạo Task đọc nút nhấn (ưu tiên mức 2 cao hơn để phản hồi nhạy)
        xTaskCreate(vButtonTask, "Button_Task", 128, NULL, 2, NULL);

        // Khởi động FreeRTOS Scheduler
        vTaskStartScheduler();
    }

    while (1) {}
}

static void MX_TIM2_Init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 63;                  // 64MHz / (63+1) = 1MHz
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 999;                   // 1MHz / (999+1) = 1kHz PWM
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_PWM_Init(&htim2);

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1); // PA0
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2); // PA1
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3); // PA2
}

static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Cấu hình PA0, PA1, PA2 làm chân Alternate Function xuất xung PWM
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Cấu hình PB0, PB1, PB10 làm chân Input Pull-Up đọc nút nhấn
    GPIO_InitStruct.Pin = BTN_SELECT_PIN | BTN_UP_PIN | BTN_DOWN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16; // 64MHz
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}