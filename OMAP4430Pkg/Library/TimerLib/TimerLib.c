/** @file

  Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>
  
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/TimerLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <OMAP4430/OMAP4430.h>

UINT32
TimerBase (
  IN  UINTN Timer
  )
{
  switch (Timer) {
  case  1: return GPTIMER1_BASE;
  case  2: return GPTIMER2_BASE;
  case  3: return GPTIMER3_BASE;
  case  4: return GPTIMER4_BASE;
  case  5: return GPTIMER5_BASE;
  case  6: return GPTIMER6_BASE;
  case  7: return GPTIMER7_BASE;
  case  8: return GPTIMER8_BASE;
  case  9: return GPTIMER9_BASE;
  case 10: return GPTIMER10_BASE;
  case 11: return GPTIMER11_BASE;
  case 12: return GPTIMER12_BASE;
  default: return 0;
  }
}

#ifdef MDE_CPU_ARM
#define MULT_U64_X_N  MultU64x32
#else
#define MULT_U64_X_N  MultU64x64
#endif

RETURN_STATUS
EFIAPI
TimerConstructor (
  VOID
  )
{
  UINTN  Timer            = PcdGet32(PcdOmap44xxFreeTimer );
  UINT32 TimerBaseAddress = TimerBase(Timer);
	
  // If the DMTIMER3 and DMTIMER4 are not enabled it is probably because it is the first call to TimerConstructor
  if ((MmioRead32 (0x4A009440) & 0x3) == 0x0) {
    // Enable DMTIMER3 with SYS_CLK source
    MmioOr32(0x4A009440, 0x2);

    // Enable DMTIMER4 with SYS_CLK source
    MmioOr32(0x4A009448, 0x2);
  }

  if ((MmioRead32 (TimerBaseAddress + GPTIMER_TCLR) & TCLR_ST_ON) == 0) {
    // Set count & reload registers
    MmioWrite32 (TimerBaseAddress + GPTIMER_TCRR, 0x00000000);
    MmioWrite32 (TimerBaseAddress + GPTIMER_TLDR, 0x00000000);

    // Disable interrupts
    MmioWrite32 (TimerBaseAddress + GPTIMER_TIER, TIER_TCAR_IT_DISABLE | TIER_OVF_IT_DISABLE | TIER_MAT_IT_DISABLE);

    // Start Timer
    MmioWrite32 (TimerBaseAddress + GPTIMER_TCLR, TCLR_AR_AUTORELOAD | TCLR_ST_ON);

    /* Sending first command to turn off watchdog */
    MmioWrite32 (WDTIMER2_BASE + WSPR, 0xAAAA);

    /* Wait for write to complete */
	while( MmioBitFieldRead32(WDTIMER2_BASE + WWPS, 4, 5) );

    /* Sending second command to turn off watchdog */
    MmioWrite32 (WDTIMER2_BASE + WSPR, 0x5555);

    /* Wait for write to complete */
	while( MmioBitFieldRead32(WDTIMER2_BASE + WWPS, 4, 5) );
  }
  return EFI_SUCCESS;
}

UINTN
EFIAPI
MicroSecondDelay (
  IN  UINTN MicroSeconds
  )
{
  UINT64  NanoSeconds;
  
  NanoSeconds = MultU64x32(MicroSeconds, 1000);

  while (NanoSeconds > (UINTN)-1) { 
    NanoSecondDelay((UINTN)-1);
    NanoSeconds -= (UINTN)-1;
  }

  NanoSecondDelay(NanoSeconds);

  return MicroSeconds;
}

UINTN
EFIAPI
NanoSecondDelay (
  IN  UINTN NanoSeconds
  )
{
  UINT32  Delay;
  UINT32  StartTime;
  UINT32  CurrentTime;
  UINT32  ElapsedTime;
  UINT32  TimerCountRegister;

  Delay = (NanoSeconds / PcdGet32(PcdEmbeddedPerformanceCounterPeriodInNanoseconds)) + 1;
  
  TimerCountRegister = TimerBase(PcdGet32(PcdOmap44xxFreeTimer)) + GPTIMER_TCRR;

  StartTime = MmioRead32 (TimerCountRegister);

  do 
  {
    CurrentTime = MmioRead32 (TimerCountRegister);
    ElapsedTime = CurrentTime - StartTime;
  } while (ElapsedTime < Delay);

  NanoSeconds = ElapsedTime * PcdGet32(PcdEmbeddedPerformanceCounterPeriodInNanoseconds);

  return NanoSeconds;
}

UINT64
EFIAPI
GetPerformanceCounter (
  VOID
  )
{ 
  return (UINT64)MmioRead32 (TimerBase(PcdGet32(PcdOmap44xxFreeTimer)) + GPTIMER_TCRR);
}

UINT64
EFIAPI
GetPerformanceCounterProperties (
  OUT UINT64  *StartValue,  OPTIONAL
  OUT UINT64  *EndValue     OPTIONAL
  )
{
  if (StartValue != NULL) {
    // Timer starts with the reload value
    *StartValue = (UINT64)MmioRead32 (TimerBase(PcdGet32(PcdOmap44xxFreeTimer)) + GPTIMER_TLDR);
  }
  
  if (EndValue != NULL) {
    // Timer counts up to 0xFFFFFFFF
    *EndValue = 0xFFFFFFFF;
  }
  
  return PcdGet64(PcdEmbeddedPerformanceCounterFrequencyInHz);
}

UINT64
EFIAPI
GetTimeInNanoSecond (
  IN      UINT64  Ticks
  )
{
  UINT64  NanoSeconds;
  UINT32  Remainder;
  UINT32  TimerFreq;

  TimerFreq = 38400000;
  //
  //          Ticks
  // Time = --------- x 1,000,000,000
  //        Frequency
  //
  NanoSeconds = MULT_U64_X_N (
                  DivU64x32Remainder (
                    Ticks,
                    TimerFreq,
                    &Remainder
                    ),
                  1000000000U
                  );

  //
  // Frequency < 0x100000000, so Remainder < 0x100000000, then (Remainder * 1,000,000,000)
  // will not overflow 64-bit.
  //
  NanoSeconds += DivU64x32 (
                   MULT_U64_X_N (
                     (UINT64)Remainder,
                     1000000000U
                     ),
                   TimerFreq
                   );

  return NanoSeconds;
}
