// Pi.c: Entry point for SEC(Security).

#include "Pi.h"

#include <Pi/PiBootMode.h>
#include <Pi/PiHob.h>
#include <PiDxe.h>
#include <PiPei.h>

#include <Guid/LzmaDecompress.h>
#include <Ppi/GuidedSectionExtraction.h>

#include <Library/ArmLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/PerformanceLib.h>
#include <Library/PrePiHobListPointerLib.h>
#include <Library/PrePiLib.h>
#include <Library/SerialPortLib.h>
#include <OMAP4430/OMAP4430.h>
VOID EFIAPI ProcessLibraryConstructorList(VOID);
UINT32* SerialAddrTwl = (UINT32*)0x48020000;


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


VOID
TimerInit (
  VOID
  )
{
  UINTN  Timer            = 4;
  UINT32 TimerBaseAddress = TimerBase(Timer);

  // Set count & reload registers
  MmioWrite32 (TimerBaseAddress + GPTIMER_TCRR, 0x00000000);
  MmioWrite32 (TimerBaseAddress + GPTIMER_TLDR, 0x00000000);

  // Disable interrupts
  MmioWrite32 (TimerBaseAddress + GPTIMER_TIER, TIER_TCAR_IT_DISABLE | TIER_OVF_IT_DISABLE | TIER_MAT_IT_DISABLE);

  // Start Timer
  MmioWrite32 (TimerBaseAddress + GPTIMER_TCLR, TCLR_AR_AUTORELOAD | TCLR_ST_ON);

}

STATIC VOID UartInit(VOID)
{
  DEBUG((EFI_D_INFO, "\nTianoCore on OMAP4 (ARM)\n"));
  DEBUG(
      (EFI_D_INFO, "Firmware version %s built %a\n\n",
       (CHAR16 *)PcdGetPtr(PcdFirmwareVersionString), __TIME__, __DATE__));
}

void loaddisplay();

VOID Main(IN VOID *StackBase, IN UINTN StackSize)
{
  EFI_HOB_HANDOFF_INFO_TABLE *HobList;
  EFI_STATUS                  Status;

  UINTN MemoryBase     = 0;
  UINTN MemorySize     = 0;
  UINTN UefiMemoryBase = 0;
  UINTN UefiMemorySize = 0;

  // Architecture-specific initialization
  // Enable Floating Point
  ArmEnableVFP();

  /* Enable program flow prediction, if supported */
  ArmEnableBranchPrediction();

  // Initialize (fake) UART.
  UartInit();

  // Declare UEFI region
  MemoryBase     = FixedPcdGet32(PcdSystemMemoryBase);
  MemorySize     = FixedPcdGet32(PcdSystemMemorySize);
  UefiMemoryBase = (UINTN)0x80C40000;
  UefiMemorySize = FixedPcdGet32(PcdUefiMemPoolSize);
  StackBase      = (VOID*)0x80C00000;

  DEBUG(
      (EFI_D_INFO | EFI_D_LOAD,
       "UEFI Memory Base = 0x%X, Size = 0x%X, Stack Base = 0x%X, Stack "
       "Size = 0x%X\n",
       UefiMemoryBase, UefiMemorySize, StackBase, StackSize));
  TimerInit();
  DEBUG((EFI_D_INFO, "\nSetting up Hob COnstructor\n"));
  // Set up HOB
  HobList = HobConstructor(
      (VOID *)UefiMemoryBase, UefiMemorySize, (VOID *)UefiMemoryBase,
      StackBase);
  DEBUG((EFI_D_INFO, "\nSetting up HobstLI\n"));
  PrePeiSetHobList(HobList);

  // Invalidate cache
  InvalidateDataCacheRange(
      (VOID *)(UINTN)PcdGet64(PcdFdBaseAddress), PcdGet32(PcdFdSize));
  DEBUG((EFI_D_INFO, "\nSetting up MMU\n"));
  // Initialize MMU
  Status = MemoryPeim(UefiMemoryBase, UefiMemorySize);

  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to configure MMU\n"));
    CpuDeadLoop();
  }

  DEBUG((EFI_D_LOAD | EFI_D_INFO, "MMU configured from device config\n"));


  // Add HOBs
  BuildStackHob((UINTN)StackBase, StackSize);

  // TODO: Call CpuPei as a library
  BuildCpuHob(ArmGetPhysicalAddressBits(), PcdGet8(PcdPrePiCpuIoSize));

  // Set the Boot Mode
  SetBootMode(BOOT_WITH_FULL_CONFIGURATION);

  // Initialize Platform HOBs (CpuHob and FvHob)
  Status = PlatformPeim();
  ASSERT_EFI_ERROR(Status);

  // Install SoC driver HOBs
  //InstallPlatformHob();

  // Now, the HOB List has been initialized, we can register performance
  // information PERF_START (NULL, "PEI", NULL, StartTimeStamp);

  // SEC phase needs to run library constructors by hand.
  ProcessLibraryConstructorList();

  // Assume the FV that contains the PI (our code) also contains a compressed
  // FV.
  Status = DecompressFirstFv();
  ASSERT_EFI_ERROR(Status);

  // Load the DXE Core and transfer control to it
  Status = LoadDxeCoreFromFv(NULL, 0);
  ASSERT_EFI_ERROR(Status);

  // We should never reach here
  CpuDeadLoop();
}

VOID CEntryPoint(IN VOID *StackBase, IN UINTN StackSize)
{
  Main(StackBase, StackSize);
}