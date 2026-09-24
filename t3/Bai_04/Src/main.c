#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

#define SAMPLE_RATE 100                 // Tần số lấy mẫu 100Hz = 100 mẫu/giây
#define HALF_BUF_SIZE (SAMPLE_RATE / 2) // 50 mẫu

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
TIM_HandleTypeDef htim3;
UART_HandleTypeDef huart1;

// Bộ đệm tròn lưu 100 mẫu ADC trong 1 giây
uint16_t adc_buffer[SAMPLE_RATE];

// Cờ báo dữ liệu sẵn sàng gửi
volatile uint8_t flag_half_ready = 0;
volatile uint8_t flag_full_ready = 0;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
void Send_Data_UART(uint16_t *data, uint16_t length);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_DMA_Init();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_ADC1_Init();
    MX_TIM3_Init();

    char init_msg[] = "\r\n=== STM32 ADC TIMER TRIGGER DMA RUNNING ===\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t*)init_msg, strlen(init_msg), 1000);

    // Kích hoạt ADC ở chế độ DMA Circular
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, SAMPLE_RATE);

    // Khởi động Timer 2 phát xung nhịp Trigger 100Hz
    HAL_TIM_Base_Start(&htim3);

    while (1) {
        // Gửi an toàn nửa đầu bộ đệm (mẫu 0 - 49) khi ngắt Half-Transfer kích hoạt
        if (flag_half_ready) {
            flag_half_ready = 0;
            Send_Data_UART(&adc_buffer[0], HALF_BUF_SIZE);
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        }

        // Gửi an toàn nửa sau bộ đệm (mẫu 50 - 99) khi ngắt Transfer-Complete kích hoạt
        if (flag_full_ready) {
            flag_full_ready = 0;
            Send_Data_UART(&adc_buffer[HALF_BUF_SIZE], HALF_BUF_SIZE);
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        }
    }
}

// Truyền mảng dữ liệu số qua UART ngắt nhau bởi \n\r
void Send_Data_UART(uint16_t *data, uint16_t length) {
    char tx_line[32];
    for (uint16_t i = 0; i < length; i++) {
        snprintf(tx_line, sizeof(tx_line), "%u\n\r", data[i]);
        HAL_UART_Transmit(&huart1, (uint8_t*)tx_line, strlen(tx_line), 100);
    }
}

// Callback khi DMA nạp đầy 50% mảng (Half-Transfer Complete)
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
    if (hadc->Instance == ADC1) {
        flag_half_ready = 1;
    }
}

// Callback khi DMA nạp đầy 100% mảng (Transfer-Complete)
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if (hadc->Instance == ADC1) {
        flag_full_ready = 1;
    }
}

// Cấu hình TIM2: tần số 100Hz, tạo xung Trigger Out (TRGO) mỗi chu kỳ
static void MX_TIM3_Init(void) {
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    __HAL_RCC_TIM3_CLK_ENABLE();

    // Clock hệ thống = 64MHz -> Prescaler = 639 (10us/tick) -> Period = 999 (10ms = 100Hz)
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 639;
htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 999;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim3);

    // Kích hoạt phát Trigger TRGO từ sự kiện Update của TIM3
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);
}

// Cấu hình ADC1 kích hoạt qua tín hiệu Trigger từ TIM2 TRGO
static void MX_ADC1_Init(void) {
    ADC_ChannelConfTypeDef sConfig = {0};

    __HAL_RCC_ADC1_CLK_ENABLE();

    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;                     // Chuyển đổi theo nhịp Timer
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO; // Timer 2 TRGO
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    HAL_ADC_Init(&hadc1);

    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    // Kênh DMA1 Channel 1 lưu trữ dữ liệu ADC
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;                          // Vòng lặp liên tục
    hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&hdma_adc1);

    __HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1);
}

static void MX_DMA_Init(void) {
    __HAL_RCC_DMA1_CLK_ENABLE();

    // Kích hoạt ngắt NVIC cho DMA1 Channel 1
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

static void MX_USART1_UART_Init(void) {
    __HAL_RCC_USART1_CLK_ENABLE();

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200; // Tốc độ 115200 baud
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);
}

static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // LED PC13
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    // Chân PA0 (ADC Input)
GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Chân PA9 (USART1 TX)
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Chân PA10 (USART1 RX)
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
}
