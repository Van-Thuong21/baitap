#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

UART_HandleTypeDef huart1;

#define RX_BUFFER_SIZE 128

char rx_buffer[RX_BUFFER_SIZE];
uint8_t rx_byte;

volatile uint8_t rx_index = 0;
volatile uint8_t data_ready = 0;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

int main(void)
{
    HAL_Init();

    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();

    char init_msg[] =
        "\r\n=== STM32 USART1 READY ===\r\n";

    HAL_UART_Transmit(&huart1,
                      (uint8_t*)init_msg,
                      strlen(init_msg),
                      1000);

    // Kích hoạt ngắt nhận 1 byte đầu tiên
    HAL_UART_Receive_IT(&huart1,
                        &rx_byte,
                        1);

    uint32_t last_blink = 0;

    while (1)
    {
        // Nháy LED PC13 mỗi 500ms
        if (HAL_GetTick() - last_blink >= 500)
        {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

            last_blink = HAL_GetTick();
        }

        // Xử lý khi nhận được ký tự kết thúc '!'
        if (data_ready)
        {
            char response[256];

            // Gửi lại bản tin với mã nhóm DTVM02_N04
            snprintf(response,
                     sizeof(response),
                     "DTVM02_N04: %s\n\r",
                     rx_buffer);

            HAL_UART_Transmit(&huart1,
                              (uint8_t*)response,
                              strlen(response),
                              1000);

            // Reset bộ đệm nhận
            memset(rx_buffer,
                   0,
                   RX_BUFFER_SIZE);

            rx_index = 0;
            data_ready = 0;
        }
    }
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        // Nhận diện ký tự kết thúc là dấu '!'
        if (rx_byte == '!')
        {
            if (rx_index > 0)
            {
                rx_buffer[rx_index] = '\0';

                data_ready = 1;
            }
        }

        else if (rx_byte != '\r' && rx_byte != '\n')
        {
            if (rx_index < RX_BUFFER_SIZE - 1)
            {
                rx_buffer[rx_index++] = rx_byte;
            }
        }

        // Tiếp tục lắng nghe byte tiếp theo
        HAL_UART_Receive_IT(&huart1,
                            &rx_byte,
                            1);
    }
}


static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;

    huart1.Init.BaudRate = 9600;

    huart1.Init.WordLength = UART_WORDLENGTH_8B;

    huart1.Init.StopBits = UART_STOPBITS_1;

    huart1.Init.Parity = UART_PARITY_NONE;

    huart1.Init.Mode = UART_MODE_TX_RX;

    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;

    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_UART_Init(&huart1);
}


static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();

    __HAL_RCC_USART1_CLK_ENABLE();


    // =========================
    // LED PC13
    // =========================

    HAL_GPIO_WritePin(GPIOC,
                      GPIO_PIN_13,
                      GPIO_PIN_SET);

    GPIO_InitStruct.Pin = GPIO_PIN_13;

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;

    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOC,
                  &GPIO_InitStruct);


    // =========================
    // PA9 - USART1 TX
    // =========================

    GPIO_InitStruct.Pin = GPIO_PIN_9;

    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;

    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOA,
                  &GPIO_InitStruct);


    // =========================
    // PA10 - USART1 RX
    // =========================

    GPIO_InitStruct.Pin = GPIO_PIN_10;

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;

    GPIO_InitStruct.Pull = GPIO_NOPULL;

    HAL_GPIO_Init(GPIOA,
                  &GPIO_InitStruct);


    // =========================
    // USART1 Interrupt
    // =========================

    HAL_NVIC_SetPriority(USART1_IRQn,
                         0,
                         0);

    HAL_NVIC_EnableIRQ(USART1_IRQn);
}


void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};


    // =========================
    // HSI + PLL
    // =========================

    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_HSI;

    RCC_OscInitStruct.HSIState =
        RCC_HSI_ON;

    RCC_OscInitStruct.HSICalibrationValue =
        RCC_HSICALIBRATION_DEFAULT;

    RCC_OscInitStruct.PLL.PLLState =
        RCC_PLL_ON;

    RCC_OscInitStruct.PLL.PLLSource =
        RCC_PLLSOURCE_HSI_DIV2;

    RCC_OscInitStruct.PLL.PLLMUL =
        RCC_PLL_MUL16;

    HAL_RCC_OscConfig(&RCC_OscInitStruct);


    // =========================
    // Clock configuration
    // =========================

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource =
        RCC_SYSCLKSOURCE_PLLCLK;

    RCC_ClkInitStruct.AHBCLKDivider =
        RCC_SYSCLK_DIV1;

    RCC_ClkInitStruct.APB1CLKDivider =
        RCC_HCLK_DIV2;

    RCC_ClkInitStruct.APB2CLKDivider =
        RCC_HCLK_DIV1;

    HAL_RCC_ClockConfig(&RCC_ClkInitStruct,
                        FLASH_LATENCY_2);
}
