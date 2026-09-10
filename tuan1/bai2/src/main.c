#include "stm32f1xx.h"

void delay(uint16_t t) {
    for (volatile uint32_t i = 0; i < t * 4000; i++);
}

void config(void) {
  
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    
  
    GPIOA->CRL &= ~0xFFFFFFFF;
    
    GPIOA->CRL |= 0x33333333;
}


void ledcr(void) {
    GPIOA->ODR &= ~0xFF;
    for (uint8_t i = 0; i < 8; i++) {
        GPIOA->ODR |= (1 << i);
        delay(1000);
    }
}


void ledtd(void) {
    for (int8_t i = 7; i >= 0; i--) {
        GPIOA->ODR &= ~(1 << i);
        delay(1000);
    }
}

int main(void) {
    config();
    while (1) {
        ledcr();   
        delay(1000);
        
        ledtd();   
        delay(1000);
    }
}
