#include "lcd/lcd.h"
#include "gd32v_pjt_include.h"
#include <stdio.h>
#include <string.h>
 
void init_uart0(void)
{   
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);
    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);
 
    /* connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    /* connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
 
    /* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200U);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
 
}

#define BUF_SIZE    17
int main(void) {
    uint16_t c, cnt, i;
    u8 dma_buf[BUF_SIZE];
    uint32_t buf[16];
    u8 buffer[1024];
    u8 x = 0, y = 0;

    // LED output setting
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1|GPIO_PIN_2);

    // initialize USART0
    init_uart0();
    // initialize OLED
    Lcd_Init();
    // lights LEDs 
    LEDR(1); LEDG(1); LEDB(1);
    // clear OLED
    LCD_Clear(BLACK);
    // enable read buffer not empty interrupt flag of USART0
    usart_interrupt_enable(USART0, USART_INT_FLAG_RBNE);

    // fill dma_buf with #
    for (i = 0; i < BUF_SIZE; i++) {
        dma_buf[i] = '#';
    }
    dma_buf[BUF_SIZE - 1] = '\0';

#define DMA_PERIPH  DMA0
#define CHANNELX    DMA_CH4

    // enable DMA clock
    rcu_periph_clock_enable(RCU_DMA0);

    // set DMA parameters
    dma_parameter_struct dma_param;
    dma_struct_para_init(&dma_param);
    dma_param.periph_addr = &USART_DATA(USART0);
    dma_param.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_param.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_param.memory_addr = dma_buf;
    dma_param.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_param.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_param.number = 16U;
    dma_param.priority = DMA_PRIORITY_HIGH;
    dma_param.direction = DMA_PERIPHERAL_TO_MEMORY;
    // disable and de-init DMA channel
    dma_channel_disable(DMA_PERIPH, CHANNELX);
    dma_deinit(DMA_PERIPH, CHANNELX);
    // initialize DMA channel
    dma_init(DMA_PERIPH, CHANNELX, &dma_param);
    // enable DMA interrupt
    dma_interrupt_enable(DMA_PERIPH, CHANNELX, DMA_INT_FLAG_FTF);


    while (1) {
        cnt = 0;
        // if USART interrupt occurs
        if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE) == SET) {
            // enable USART DMA transfer
            usart_dma_receive_config(USART0, USART_DENR_ENABLE);
            // enable DMA channel
            dma_channel_enable(DMA_PERIPH, CHANNELX);
            // while DMA interrupt flag is set
            if (dma_interrupt_flag_get(DMA_PERIPH, CHANNELX, DMA_INT_FLAG_FTF) == SET) {
                // show characters
                LCD_ShowString(x, y, (u8 *)dma_buf, WHITE);
                usart_printf((u8 *)dma_buf);
                dma_interrupt_flag_clear(DMA_PERIPH, CHANNELX, DMA_INT_FLAG_FTF);
                // disable DMA channel
                dma_channel_disable(DMA_PERIPH, CHANNELX);
                // initialize DMA channel
                dma_init(DMA_PERIPH, CHANNELX, &dma_param);
                // disable USART DMA transfer
                usart_dma_receive_config(USART0, USART_DENR_DISABLE);
            }
        }

        // flip LEDs
        LEDR_TOG; delay_1ms(20);
        LEDG_TOG; delay_1ms(20);
        LEDB_TOG; delay_1ms(20); 
    }
}

// put a character to USART0
int _put_char(int ch)
{
    usart_data_transmit(USART0, (uint8_t) ch );
    while ( usart_flag_get(USART0, USART_FLAG_TBE)== RESET){
    }
 
    return ch;
}
 
#include <stdarg.h>
void usart_printf(const char *fmt, ...) {
    char buf[100];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
 
    char *p = buf;
    while( *p != '\0' ) {
        _put_char(*p);
        p++;
    }
}