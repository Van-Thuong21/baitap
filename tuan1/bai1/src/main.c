#include "stm32f1xx.h"

void delay(void) {
    volatile uint16_t i,j;
    for( i = 0; i < 1000; i++){
        for(j = 0; j < 1000; j++);
    }
}

int main(){
    // Cấp Clock cho GPIO
    RCC -> APB2ENR |= 0xFC;

    GPIOC -> CRH &= ~0x00F00000;
    GPIOC -> CRH |= 0x00300000;   // Output push pull, 50Mhz

    while(1){
        GPIOC -> ODR ^= (1<<13);
        delay();
    }
}
