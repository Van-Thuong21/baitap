#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>

// Địa chỉ BMP280 khi chân SDO nối GND
#define BMP280_ADDR             (0x76 << 1) // 0xEC
#define BMP280_REG_ID           0xD0
#define BMP280_REG_RESET        0xE0
#define BMP280_REG_CTRL_MEAS    0xF4
#define BMP280_REG_CONFIG       0xF5
#define BMP280_REG_DATA         0xF7

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;

// Các hệ số hiệu chuẩn lưu trong ROM của BMP280
uint16_t dig_T1;
int16_t  dig_T2, dig_T3;
uint16_t dig_P1;
int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
int32_t  t_fine;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
void I2C_Bus_Reset(void);

uint8_t BMP280_Init(void);
void BMP280_ReadRaw(int32_t *raw_temp, int32_t *raw_press);
float BMP280_Compensate_Temperature(int32_t adc_T);
float BMP280_Compensate_Pressure(int32_t adc_P);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    
    // Giải phóng bus I2C trước khi khởi động ngoại vi
    I2C_Bus_Reset();
    
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_I2C1_Init();

    char msg[128];
    snprintf(msg, sizeof(msg), "\r\n=== BMP280 I2C INITIALIZATION ===\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);

    // Thử kết nối cảm biến
    while (!BMP280_Init()) {
        snprintf(msg, sizeof(msg), "[ERROR] BMP280 not found! Retrying in 1s...\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(1000);
    }

    snprintf(msg, sizeof(msg), "[SUCCESS] BMP280 Connected Successfully!\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);

    while (1) {
        int32_t raw_temp = 0, raw_press = 0;
        BMP280_ReadRaw(&raw_temp, &raw_press);

        float temperature = BMP280_Compensate_Temperature(raw_temp);
        float pressure    = BMP280_Compensate_Pressure(raw_press);

        // Tách phần nguyên và thập phân để in an toàn không phụ thuộc printf_float
        int32_t t_int  = (int32_t)temperature;
        int32_t t_frac = (int32_t)((temperature - t_int) * 100);
        if (t_frac < 0) t_frac = -t_frac;

        int32_t p_int  = (int32_t)pressure;
        int32_t p_frac = (int32_t)((pressure - p_int) * 100);
        if (p_frac < 0) p_frac = -p_frac;

        snprintf(msg, sizeof(msg), "Temp: %ld.%02ld C | Press: %ld.%02ld hPa\r\n", 
                 t_int, t_frac, p_int, p_frac);
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);

        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(1000);
    }
}

// Khôi phục trạng thái đường truyền bus I2C bị kẹt mức thấp
void I2C_Bus_Reset(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    for (int i = 0; i < 9; i++) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(1);
    }
}

uint8_t BMP280_Init(void) {
    uint8_t chip_id = 0;
    
    // Đọc thanh ghi ID (0xD0)
    if (HAL_I2C_Mem_Read(&hi2c1, BMP280_ADDR, BMP280_REG_ID, 1, &chip_id, 1, 200) != HAL_OK) {
        return 0;
    }

    if (chip_id != 0x58 && chip_id != 0x60) {
        return 0;
    }

    // Reset mềm cảm biến
    uint8_t rst_cmd = 0xB6;
    HAL_I2C_Mem_Write(&hi2c1, BMP280_ADDR, BMP280_REG_RESET, 1, &rst_cmd, 1, 100);
    HAL_Delay(50);

    // Đọc mảng 24 byte hệ số bù từ 0x88
    uint8_t calib[24] = {0};
    if (HAL_I2C_Mem_Read(&hi2c1, BMP280_ADDR, 0x88, 1, calib, 24, 300) != HAL_OK) {
        return 0;
    }

    dig_T1 = (uint16_t)((calib[1] << 8) | calib[0]);
    dig_T2 = (int16_t)((calib[3] << 8) | calib[2]);
    dig_T3 = (int16_t)((calib[5] << 8) | calib[4]);

    dig_P1 = (uint16_t)((calib[7] << 8) | calib[6]);
    dig_P2 = (int16_t)((calib[9] << 8) | calib[8]);
    dig_P3 = (int16_t)((calib[11] << 8) | calib[10]);
    dig_P4 = (int16_t)((calib[13] << 8) | calib[12]);
    dig_P5 = (int16_t)((calib[15] << 8) | calib[14]);
    dig_P6 = (int16_t)((calib[17] << 8) | calib[16]);
    dig_P7 = (int16_t)((calib[19] << 8) | calib[18]);
    dig_P8 = (int16_t)((calib[21] << 8) | calib[20]);
    dig_P9 = (int16_t)((calib[23] << 8) | calib[22]);

    // Thanh ghi 0xF5: Standby time 500ms, bộ lọc IIR filter x16
    uint8_t config = 0x90;
    HAL_I2C_Mem_Write(&hi2c1, BMP280_ADDR, BMP280_REG_CONFIG, 1, &config, 1, 100);

    // Thanh ghi 0xF4: Nhiệt độ x2, Áp suất x16, Chế độ Normal
    uint8_t ctrl_meas = 0x57;
    HAL_I2C_Mem_Write(&hi2c1, BMP280_ADDR, BMP280_REG_CTRL_MEAS, 1, &ctrl_meas, 1, 100);

    HAL_Delay(100);
    return 1;
}

void BMP280_ReadRaw(int32_t *raw_temp, int32_t *raw_press) {
    uint8_t data[6] = {0};
    
    // Đọc liên tiếp 6 byte từ thanh ghi 0xF7
    if (HAL_I2C_Mem_Read(&hi2c1, BMP280_ADDR, BMP280_REG_DATA, 1, data, 6, 200) == HAL_OK) {
        *raw_press = (int32_t)((((uint32_t)data[0]) << 12) | (((uint32_t)data[1]) << 4) | (data[2] >> 4));
        *raw_temp  = (int32_t)((((uint32_t)data[3]) << 12) | (((uint32_t)data[4]) << 4) | (data[5] >> 4));
    } else {
        *raw_temp  = 0;
        *raw_press = 0;
    }
}

float BMP280_Compensate_Temperature(int32_t adc_T) {
    if (adc_T == 0 || adc_T == 0x800000) return 0.0f;
    int32_t var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    int32_t var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    return (float)((t_fine * 5 + 128) >> 8) / 100.0f;
}

float BMP280_Compensate_Pressure(int32_t adc_P) {
    if (adc_P == 0 || adc_P == 0x800000 || t_fine == 0) return 0.0f;
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
    if (var1 == 0) return 0.0f;

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    return (float)p / 256.0f / 100.0f;
}

static void MX_I2C1_Init(void) {
    __HAL_RCC_I2C1_CLK_ENABLE();

    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
}

static void MX_USART1_UART_Init(void) {
    __HAL_RCC_USART1_CLK_ENABLE();

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
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
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // LED PC13
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    // Chân I2C1 PB6 (SCL) và PB7 (SDA): Bắt buộc Open Drain và Pull-up
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Chân UART1: PA9 (TX)
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Chân UART1: PA10 (RX)
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}