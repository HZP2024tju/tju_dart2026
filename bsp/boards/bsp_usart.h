#ifndef BSP_USART_H
#define BSP_USART_H
#include "struct_typedef.h"
#include "usart.h"


extern void usart6_init(uint8_t *rx1_buf, uint8_t *rx2_buf, uint16_t dma_buf_num);


extern UART_HandleTypeDef huart6;
extern DMA_HandleTypeDef hdma_usart6_rx;

#endif
