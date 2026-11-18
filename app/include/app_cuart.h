#ifndef __BSP_CUART_H
#define __BSP_CUART_H


#include <stdint.h>
#include <stdbool.h>

void c_uart_pro(uint8_t *bPtr,int32_t len);
void UartProcess(void);
extern uint8_t WriteMobileNumFlag;
#endif

