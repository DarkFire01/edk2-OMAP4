/** @file

  Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __OMAP3530_H__
#define __OMAP3530_H__

#include "Omap3530Prcm.h"

#define UART1_BASE  (0x0)
#define UART2_BASE  (0x0)
#define UART3_BASE  (0x48020000)

#define UART_DLL_REG  (0x0000)
#define UART_RBR_REG  (0x0000)
#define UART_THR_REG  (0x0000)
#define UART_DLH_REG  (0x0004)
#define UART_FCR_REG  (0x0008)
#define UART_LCR_REG  (0x000C)
#define UART_MCR_REG  (0x0010)
#define UART_LSR_REG  (0x0014)
#define UART_MDR1_REG (0x0020)

#define UART_FCR_TX_FIFO_CLEAR          BIT2
#define UART_FCR_RX_FIFO_CLEAR          BIT1
#define UART_FCR_FIFO_ENABLE            BIT0

#define UART_LCR_DIV_EN_ENABLE          BIT7
#define UART_LCR_DIV_EN_DISABLE         (0UL << 7)
#define UART_LCR_CHAR_LENGTH_8          (BIT1 | BIT0)

#define UART_MCR_RTS_FORCE_ACTIVE       BIT1
#define UART_MCR_DTR_FORCE_ACTIVE       BIT0

#define UART_LSR_TX_FIFO_E_MASK         BIT5
#define UART_LSR_TX_FIFO_E_NOT_EMPTY    (0UL << 5)
#define UART_LSR_TX_FIFO_E_EMPTY        BIT5
#define UART_LSR_RX_FIFO_E_MASK         BIT0
#define UART_LSR_RX_FIFO_E_NOT_EMPTY    BIT0
#define UART_LSR_RX_FIFO_E_EMPTY        (0UL << 0)

// BIT2:BIT0
#define UART_MDR1_MODE_SELECT_DISABLE   (7UL)
#define UART_MDR1_MODE_SELECT_UART_16X  (0UL)


//CONTROL_PBIAS_LITE
#define CONTROL_PBIAS_LITE    0x48002520
#define PBIASLITEVMODE0       BIT0
#define PBIASLITEPWRDNZ0      BIT1
#define PBIASSPEEDCTRL0       BIT2
#define PBIASLITEVMODE1       BIT8
#define PBIASLITEWRDNZ1       BIT9

#endif // __OMAP3530_H__

