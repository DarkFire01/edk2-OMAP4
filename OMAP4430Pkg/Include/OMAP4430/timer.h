/** @file

  Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __OMAP4TIMER_H__
#define __OMAP4TIMER_H__

/* Watchdog */
#define WDTIMER2_BASE   (0x4A314000)
#define WWPS              (0x034)
#define WSPR              (0x048)

/* DMTIMERS*/

#define GPTIMER1_BASE   (0x48313000)
#define GPTIMER2_BASE   (0x48032000)
#define GPTIMER3_BASE   (0x48034000)
#define GPTIMER4_BASE   (0x48036000)
#define GPTIMER5_BASE   (0x49038000)
#define GPTIMER6_BASE   (0x4903A000)
#define GPTIMER7_BASE   (0x4903C000)
#define GPTIMER8_BASE   (0x4903E000)
#define GPTIMER9_BASE   (0x4803E000)
#define GPTIMER10_BASE  (0x48086000)
#define GPTIMER11_BASE  (0x48088000)
#define GPTIMER12_BASE  (0x48304000)

// TODO: review below timer IP register offsets changing from OMAP3 to OMAP4

#define GPTIMER_TIOCP_CFG (/*0x0010*/)
#define GPTIMER_TISTAT    (/*0x0014*/)
#define GPTIMER_TISR      (/*0x0018 => IRQSTATUS*/0x28) 
#define GPTIMER_TIER      (/*0x001C => IRQSTATUS_SET*/0x2C)
#define GPTIMER_TWER      (/*0x0020*/)
#define GPTIMER_TCLR      (/*0x0024*/0x38)
#define GPTIMER_TCRR      (/*0x0028*/0x3C)
#define GPTIMER_TLDR      (/*0x002C*/0x40)
#define GPTIMER_TTGR      (/*0x0030*/0x44)
#define GPTIMER_TWPS      (/*0x0034*/0x48)
#define GPTIMER_TMAR      (/*0x0038*/0x4C)
#define GPTIMER_TCAR1     (/*0x003C*/0x50)
#define GPTIMER_TSICR     (/*0x0040*/0x54)
#define GPTIMER_TCAR2     (/*0x0044*/0x58)
#define GPTIMER_TPIR      (/*0x0048*/)
#define GPTIMER_TNIR      (/*0x004C*/)
#define GPTIMER_TCVR      (/*0x0050*/)
#define GPTIMER_TOCR      (/*0x0054*/)
#define GPTIMER_TOWR      (/*0x0058*/)

#define TISR_TCAR_IT_FLAG_MASK  BIT2
#define TISR_OVF_IT_FLAG_MASK   BIT1
#define TISR_MAT_IT_FLAG_MASK   BIT0
#define TISR_ALL_INTERRUPT_MASK (TISR_TCAR_IT_FLAG_MASK | TISR_OVF_IT_FLAG_MASK | TISR_MAT_IT_FLAG_MASK)

#define TISR_TCAR_IT_FLAG_NOT_PENDING   (0UL << 2)
#define TISR_OVF_IT_FLAG_NOT_PENDING    (0UL << 1)
#define TISR_MAT_IT_FLAG_NOT_PENDING    (0UL << 0)
#define TISR_NO_INTERRUPTS_PENDING      (TISR_TCAR_IT_FLAG_NOT_PENDING | TISR_OVF_IT_FLAG_NOT_PENDING | TISR_MAT_IT_FLAG_NOT_PENDING)

#define TISR_TCAR_IT_FLAG_CLEAR BIT2
#define TISR_OVF_IT_FLAG_CLEAR  BIT1
#define TISR_MAT_IT_FLAG_CLEAR  BIT0
#define TISR_CLEAR_ALL          (TISR_TCAR_IT_FLAG_CLEAR | TISR_OVF_IT_FLAG_CLEAR | TISR_MAT_IT_FLAG_CLEAR)

#define TCLR_AR_AUTORELOAD      BIT1
#define TCLR_AR_ONESHOT         (0UL << 1)
#define TCLR_ST_ON              BIT0
#define TCLR_ST_OFF             (0UL << 0)

#define TIER_TCAR_IT_ENABLE     (BIT2
#define TIER_TCAR_IT_DISABLE    (0UL << 2)
#define TIER_OVF_IT_ENABLE      BIT1
#define TIER_OVF_IT_DISABLE     (0UL << 1)
#define TIER_MAT_IT_ENABLE      BIT0
#define TIER_MAT_IT_DISABLE     (0UL << 0)

#endif // __OMAP4TIMER_H__