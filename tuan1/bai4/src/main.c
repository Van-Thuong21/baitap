#include "stm32f1xx.h"

void delay(uint16_t t) {
    volatile uint16_t i,j;
    for( i = 0 ; i < t; i++){
        for( j = 0; j < 2000; j++);
    }
}

void config(void){
    RCC -> APB2ENR |= 0xFC;

    GPIOC -> CRH &= ~0x00FF0000;
    GPIOC -> CRH |= 0x00380000;

    GPIOC -> ODR |= (1 << 12);  // mặc định PC12 mực high khi chưa nhấn
}

int main(){
    config();
    while(1){
        if((GPIOC -> IDR & (1 << 12)) == 0){
            delay(100);
            GPIOC -> ODR ^= (1 << 13);
        }
        while((GPIOC -> IDR & (1 << 12)) == 0);
    }
}