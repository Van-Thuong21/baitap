#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

UART_HandleTypeDef huart1;
TIM_HandleTypeDef htim2;

#define RX_BUFFER_SIZE 64

char rx_buffer[RX_BUFFER_SIZE];
uint8_t rx_byte;

volatile uint8_t rx_index = 0;
volatile uint8_t command_ready = 0;

uint8_t led_state = 0;
uint32_t current_duty = 50;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_PWM_Init(void);

void Process_Command(char *cmd);

int main(void)
{
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_TIM2_PWM_Init();

    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);

    char init_msg[] =
        "\r\n=== STM32 PWM UART CONTROL READY ===\r\n";

    HAL_UART_Transmit(&huart1,
                      (uint8_t *)init_msg,
                      strlen(init_msg),
                      1000);

    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);

    while (1)
    {
        if (command_ready)
        {
            Process_Command(rx_buffer);

            memset(rx_buffer, 0, RX_BUFFER_SIZE);

            rx_index = 0;

            command_ready = 0;
        }
    }
}


void Process_Command(char *cmd)
{
    char response[128];

    if (strcmp(cmd, "ON") == 0)
    {
        led_state = 1;

        __HAL_TIM_SET_COMPARE(&htim2,
                              TIM_CHANNEL_1,
                              current_duty * 10);

        snprintf(response,
                 sizeof(response),
                 "LED turned ON at %lu%%\r\n",
                 current_duty);

        HAL_UART_Transmit(&huart1,
                          (uint8_t *)response,
                          strlen(response),
                          1000);
    }

    else if (strcmp(cmd, "OFF") == 0)
    {
        led_state = 0;

        __HAL_TIM_SET_COMPARE(&htim2,
                              TIM_CHANNEL_1,
                              0);

        snprintf(response,
                 sizeof(response),
                 "LED turned OFF\r\n");

        HAL_UART_Transmit(&huart1,
                          (uint8_t *)response,
                          strlen(response),
                          1000);
    }

    else if (strncmp(cmd, "PWM:", 4) == 0)
    {
        char *ptr = cmd + 4;

        char *end_ptr = strchr(ptr, '%');

        if (end_ptr != NULL)
        {
            *end_ptr = '\0';

            int val = atoi(ptr);

            if (val >= 0 && val <= 100)
            {
                current_duty = (uint32_t)val;

                if (led_state == 1)
                {
                    __HAL_TIM_SET_COMPARE(&htim2,
                                          TIM_CHANNEL_1,
                                          current_duty * 10);

                    snprintf(response,
                             sizeof(response),
                             "PWM updated to %lu%%\r\n",
                             current_duty);
                }
                else
                {
                    snprintf(response,
                             sizeof(response),
                             "Configured PWM to %lu%% (LED currently OFF)\r\n",
                             current_duty);
                }

                HAL_UART_Transmit(&huart1,
                                  (uint8_t *)response,
                                  strlen(response),
                                  1000);
            }
        }
    }

    else if (strcmp(cmd, "Status") == 0)
    {
        snprintf(response,
                 sizeof(response),
                 "Status -> State: %s | Configured PWM: %lu%%\r\n",
                 led_state ? "ON" : "OFF",
                 current_duty);

        HAL_UART_Transmit(&huart1,
                          (uint8_t *)response,
                          strlen(response),
                          1000);
    }

    else
    {
        snprintf(response,
                 sizeof(response),
                 "Unknown command '%s'\r\n",
                 cmd);

        HAL_UART_Transmit(&huart1,
                          (uint8_t *)response,
                          strlen(response),
                          1000);
    }
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        if (rx_byte == '!')
        {
            if (rx_index > 0)
            {
                rx_buffer[rx_index] = '\0';

                command_ready = 1;
            }
        }

        else if (rx_byte != '\r' && rx_byte != '\n')
        {
            if (rx_index < RX_BUFFER_SIZE - 1)
            {
                rx_buffer[rx_index++] = rx_byte;
            }
        }

        HAL_UART_Receive_IT(&huart1,
                            &rx_byte,
                            1);
    }
}


static void MX_TIM2_PWM_Init(void)
{
    TIM_OC_InitTypeDef sConfigOC = {0};

    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance = TIM2;

    htim2.Init.Prescaler = 63;

    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;

    htim2.Init.Period = 999;

    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

    htim2.Init.AutoReloadPreload =
        TIM_AUTORELOAD_PRELOAD_ENABLE;

    HAL_TIM_PWM_Init(&htim2);

    sConfigOC.OCMode = TIM_OCMODE_PWM1;

    sConfigOC.Pulse = 0;

    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;

    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    HAL_TIM_PWM_ConfigChannel(&htim2,
                              &sConfigOC,
                              TIM_CHANNEL_1);
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

    __HAL_RCC_GPIOA_CLK_ENABLE();

    __HAL_RCC_USART1_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_0;

    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;

    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOA,
                  &GPIO_InitStruct);


    GPIO_InitStruct.Pin = GPIO_PIN_9;

    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;

    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOA,
                  &GPIO_InitStruct);


    GPIO_InitStruct.Pin = GPIO_PIN_10;

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;

    GPIO_InitStruct.Pull = GPIO_NOPULL;

    HAL_GPIO_Init(GPIOA,
                  &GPIO_InitStruct);


    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);

    HAL_NVIC_EnableIRQ(USART1_IRQn);
}


void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};


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
