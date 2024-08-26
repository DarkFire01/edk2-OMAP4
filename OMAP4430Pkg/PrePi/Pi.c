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
 

STATIC VOID UartInit(VOID)
{
  DEBUG((EFI_D_INFO, "\nTianoCore on OMAP4 (ARM)\n"));
  DEBUG(
      (EFI_D_INFO, "Firmware version %s built %a\n\n",
       (CHAR16 *)PcdGetPtr(PcdFirmwareVersionString), __TIME__, __DATE__));
}



VOID
GpmcConfiguration (
  VOID
  )
{
  // Make sure all chip selects are disabled
  // Kernel makes a wrong assumption about CS0 being already configured by ROM
  MmioWrite32 (0x50000078, 0xF00);

}

#define PUBLIC_API_BASE                                 (0x28400)

#define PUBLIC_API_IRQ_REGISTER                         (0x44)
#define PUBLIC_API_IRQ_UNREGISTER                       (0x48)
#define PUBLIC_API_CM_ENABLEMODULECLOCKS                (0xA0)
#define PUBLIC_API_WDTIMER_DISABLE                      (0x54)
#define PUBLIC_API_CTRL_CONFIGUREPADS                   (0xA8)

//PUBLIC_API_IRQ_REGISTER
typedef UINT32 (** const IRQ_Register_pt)( UINT32,
                                           UINT32,
                                           UINT32 );
#define RomIrqRegister(a, b, c) \
   (*(IRQ_Register_pt) ((PUBLIC_API_BASE+PUBLIC_API_IRQ_REGISTER)&0xFFFFFFFE))(a, b, c);

//PUBLIC_API_IRQ_UNREGISTER
typedef UINT32 (** const IRQ_UnRegister_pt)( UINT32 );
#define RomIrqUnRegister(a) \
   (*(IRQ_UnRegister_pt) (PUBLIC_API_BASE+PUBLIC_API_IRQ_UNREGISTER))(a);

// PUBLIC_API_WDTIMER_DISABLE
typedef void (** const HAL_WDTIMER_Disable_pt)( void );
#define RomWdtimerDisable() \
  (*(HAL_WDTIMER_Disable_pt) ((PUBLIC_API_BASE+PUBLIC_API_WDTIMER_DISABLE)&0xFFFFFFFE))();

//PUBLIC_API_CM_ENABLEMODULECLOCKS
typedef UINT32 (** const HAL_CM_EnableModuleClocks_pt)( UINT32, UINT32 );
#define RomEnableClocks(a, b) \
  (*(HAL_CM_EnableModuleClocks_pt) ((PUBLIC_API_BASE+PUBLIC_API_CM_ENABLEMODULECLOCKS)&0xFFFFFFFE))(a, b);

//PUBLIC_API_CTRL_CONFIGUREPADS
typedef UINT32 (** const HAL_CTRL_ConfigurePads_pt)( UINT32, UINT32 );
#define RomCtrlConfigurePads(a, b) \
  (*(HAL_CTRL_ConfigurePads_pt) ((PUBLIC_API_BASE+PUBLIC_API_CTRL_CONFIGUREPADS)&0xFFFFFFFE))(a, b);

VOID
PadConfiguration (
  VOID
  )
{
  // TODO: pad configuration

  // Configure UART3 pads
  RomCtrlConfigurePads (2, 2);

}

VOID
ClockInit (
  VOID
  )
{
  // TODO: clocks configuration code clean up

  // CORE, PER DPLLs are configured part of Configuration header which OMAP4 ROM parses.

  // Turn on functional & interface clocks to MMC1 and I2C1 modules.
  MmioOr32(0x4a009328, 0x03070002);

  //Enable DMTIMER3 with SYS_CLK source
  MmioOr32(0x4A009440, 0x2);

  //Enable DMTIMER4 with SYS_CLK source
  MmioOr32(0x4A009448, 0x2);

  // Enable UART3 clocks
  RomEnableClocks (2, 2);

  // Enable watchdog interface clocks
  RomEnableClocks (6, 1);

}

void loaddisplay();

RETURN_STATUS
EFIAPI
TimerConstructor (
  VOID
  )
{
 
  UINT32 TimerBaseAddress = GPTIMER4_BASE;
	
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
    loaddisplay();

  // Declare UEFI region
  MemoryBase     = FixedPcdGet32(PcdSystemMemoryBase);
  MemorySize     = FixedPcdGet32(PcdSystemMemorySize);
  UefiMemoryBase = MemoryBase + FixedPcdGet32(PcdPreAllocatedMemorySize);
  UefiMemorySize = FixedPcdGet32(PcdUefiMemPoolSize);
  StackBase      = (VOID *)(UefiMemoryBase + UefiMemorySize - StackSize);
  DEBUG((
        EFI_D_INFO | EFI_D_LOAD,
        "UEFI Memory Base = 0x%p, UEFI Memory Size = 0x%p\n",
        UefiMemoryBase,
        UefiMemorySize
    ));

  DEBUG((
        EFI_D_INFO | EFI_D_LOAD,
        "Stack Base = 0x%p, Stacks Size = 0x%p\n",
        StackBase,
        StackSize
    ));
  PadConfiguration();
 
ClockInit();
  HobList = HobConstructor(
      (VOID *)UefiMemoryBase, UefiMemorySize, (VOID *)UefiMemoryBase,
      StackBase);

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

//  GpmcConfiguration();

  // Install SoC driver HOBs
 // InstallPlatformHob();

  // Now, the HOB List has been initialized, we can register performance
  // information PERF_START (NULL, "PEI", NULL, StartTimeStamp);
  TimerConstructor();
  // SEC phase needs to run library constructors by hand.
  ProcessLibraryConstructorList();

  // Assume the FV that contains the PI (our code) also contains a compressed
  // FV.
  Status = DecompressFirstFv();
  ASSERT_EFI_ERROR(Status);

  // Load the DXE Core and transfer control to it
  Status = LoadDxeCoreFromFv(NULL, 0);

  // We should never reach here
  CpuDeadLoop();
}

VOID CEntryPoint(IN VOID *StackBase, IN UINTN StackSize)
{
  Main(StackBase, StackSize);
}