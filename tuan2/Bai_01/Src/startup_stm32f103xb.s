.syntax unified
.cpu cortex-m3
.thumb

.global g_pfnVectors
.global Default_Handler

/* Thiết lập kích thước Stack và Heap */
.equ Stack_Size, 0x00000400
.equ Heap_Size,  0x00000200

.section .stack
.align 3
Stack_Mem:
.space Stack_Size
__initial_sp:

.section .heap
.align 3
Heap_Mem:
.space Heap_Size
__heap_limit:

.section .text.isr_vector
.align 2
g_pfnVectors:
    g_pfnVectors:
    .word   0x20005000
    .word   Reset_Handler
    .word   NMI_Handler
    .word   HardFault_Handler
    .word   MemManage_Handler
    .word   BusFault_Handler
    .word   UsageFault_Handler
    .word   0
    .word   0
    .word   0
    .word   0
    .word   SVC_Handler
    .word   DebugMon_Handler
    .word   0
    .word   PendSV_Handler
    .word   SysTick_Handler
    
    /* Ngoại vi STM32F103 */
    .word   WWDG_IRQHandler
    .word   PVD_IRQHandler
    .word   TAMPER_IRQHandler
    .word   RTC_IRQHandler
    .word   FLASH_IRQHandler
    .word   RCC_IRQHandler
    .word   EXTI0_IRQHandler
    .word   EXTI1_IRQHandler
    .word   EXTI2_IRQHandler
    .word   EXTI3_IRQHandler
    .word   EXTI4_IRQHandler
    .word   DMA1_Channel1_IRQHandler
    .word   DMA1_Channel2_IRQHandler
    .word   DMA1_Channel3_IRQHandler
    .word   DMA1_Channel4_IRQHandler
    .word   DMA1_Channel5_IRQHandler
    .word   DMA1_Channel6_IRQHandler
    .word   DMA1_Channel7_IRQHandler
    .word   ADC1_2_IRQHandler
    .word   USB_HP_CAN1_TX_IRQHandler
    .word   USB_LP_CAN1_RX0_IRQHandler
    .word   CAN1_RX1_IRQHandler
    .word   CAN1_SCE_IRQHandler
    .word   EXTI9_5_IRQHandler
    .word   TIM1_BRK_IRQHandler
    .word   TIM1_UP_IRQHandler
    .word   TIM1_TRG_COM_IRQHandler
    .word   TIM1_CC_IRQHandler
    .word   TIM2_IRQHandler
    .word   TIM3_IRQHandler
    .word   TIM4_IRQHandler
    .word   I2C1_EV_IRQHandler
    .word   I2C1_ER_IRQHandler
    .word   I2C2_EV_IRQHandler
    .word   I2C2_ER_IRQHandler
    .word   SPI1_IRQHandler
    .word   SPI2_IRQHandler
    .word   USART1_IRQHandler
    .word   USART2_IRQHandler
    .word   USART3_IRQHandler
    .word   EXTI15_10_IRQHandler
    .word   RTC_Alarm_IRQHandler
    .word   USBWakeUp_IRQHandler

.text
.thumb_func
.weak Reset_Handler
.type Reset_Handler, %function
Reset_Handler:
    /* Sao chép dữ liệu từ Flash sang SRAM (.data) */
    ldr   r0, =_sdata
    ldr   r1, =_edata
    ldr   r2, =_sidata
    b     LoopCopyDataInit

CopyDataInit:
    ldr   r3, [r2], #4
    str   r3, [r0], #4

LoopCopyDataInit:
    cmp   r0, r1
    bcc   CopyDataInit

    /* Xóa vùng nhớ BSS (.bss) về 0 */
    ldr   r0, =_sbss
    ldr   r1, =_ebss
    movs  r2, #0
    b     LoopFillZerobss

FillZerobss:
    str   r2, [r0], #4

LoopFillZerobss:
    cmp   r0, r1
    bcc   FillZerobss

    /* Gọi SystemInit và hàm main */
    bl    SystemInit
    bl    main
    bx    lr

.size Reset_Handler, .-Reset_Handler

/* Các hàm ngắt mặc định */
.macro def_handler    handler_name
.weak \handler_name
.thumb_func
\handler_name:
    b .
.endm

def_handler NMI_Handler
def_handler HardFault_Handler
def_handler MemManage_Handler
def_handler BusFault_Handler
def_handler UsageFault_Handler
def_handler SVC_Handler
def_handler DebugMon_Handler
def_handler PendSV_Handler
def_handler SysTick_Handler
def_handler WWDG_IRQHandler
def_handler PVD_IRQHandler
def_handler TAMPER_IRQHandler
def_handler RTC_IRQHandler
def_handler FLASH_IRQHandler
def_handler RCC_IRQHandler
def_handler EXTI0_IRQHandler
def_handler EXTI1_IRQHandler
def_handler EXTI2_IRQHandler
def_handler EXTI3_IRQHandler
def_handler EXTI4_IRQHandler
def_handler DMA1_Channel1_IRQHandler
def_handler DMA1_Channel2_IRQHandler
def_handler DMA1_Channel3_IRQHandler
def_handler DMA1_Channel4_IRQHandler
def_handler DMA1_Channel5_IRQHandler
def_handler DMA1_Channel6_IRQHandler
def_handler DMA1_Channel7_IRQHandler
def_handler ADC1_2_IRQHandler
def_handler USB_HP_CAN1_TX_IRQHandler
def_handler USB_LP_CAN1_RX0_IRQHandler
def_handler CAN1_RX1_IRQHandler
def_handler CAN1_SCE_IRQHandler
def_handler EXTI9_5_IRQHandler
def_handler TIM1_BRK_IRQHandler
def_handler TIM1_UP_IRQHandler
def_handler TIM1_TRG_COM_IRQHandler
def_handler TIM1_CC_IRQHandler
def_handler TIM2_IRQHandler
def_handler TIM3_IRQHandler
def_handler TIM4_IRQHandler
def_handler I2C1_EV_IRQHandler
def_handler I2C1_ER_IRQHandler
def_handler I2C2_EV_IRQHandler
def_handler I2C2_ER_IRQHandler
def_handler SPI1_IRQHandler
def_handler SPI2_IRQHandler
def_handler USART1_IRQHandler
def_handler USART2_IRQHandler
def_handler USART3_IRQHandler
def_handler EXTI15_10_IRQHandler
def_handler RTC_Alarm_IRQHandler
def_handler USBWakeUp_IRQHandler
def_handler Default_Handler