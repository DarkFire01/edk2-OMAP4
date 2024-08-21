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

#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/EmbeddedGpio.h>

#include <OMAP4430/OMAP4430.h>

UINT32 
GpioBase (
  IN  UINTN Port
  )
{
  switch (Port) {
  case 1:  return GPIO1_BASE;
  case 2:  return GPIO2_BASE;
  case 3:  return GPIO3_BASE;
  case 4:  return GPIO4_BASE;
  case 5:  return GPIO5_BASE;
  case 6:  return GPIO6_BASE;
  default: return 0;
  }
}

EFI_STATUS
Get (
  IN  EMBEDDED_GPIO     *This,
  IN  EMBEDDED_GPIO_PIN Gpio,
  OUT UINTN               *Value
  )
{
  UINTN  Port;
  UINTN  Pin;
  UINT32 DataInRegister;

  if (Value == NULL)
  {
    return EFI_UNSUPPORTED;
  }

  Port    = GPIO_PORT(Gpio);
  Pin     = GPIO_PIN(Gpio);

  DataInRegister = GpioBase(Port) + GPIO_DATAIN;

  if (MmioRead32 (DataInRegister) & GPIO_DATAIN_MASK(Pin)) {
    *Value = 1;
  } else {
    *Value = 0;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
Set (
  IN  EMBEDDED_GPIO       *This,
  IN  EMBEDDED_GPIO_PIN   Gpio,
  IN  EMBEDDED_GPIO_MODE  Mode
  )
{
  UINTN  Port;
  UINTN  Pin;
  UINT32 OutputEnableRegister;
  UINT32 SetDataOutRegister;
  UINT32 ClearDataOutRegister;

  Port    = GPIO_PORT(Gpio);
  Pin     = GPIO_PIN(Gpio);

  OutputEnableRegister = GpioBase(Port) + GPIO_OE;
  SetDataOutRegister   = GpioBase(Port) + GPIO_SETDATAOUT;
  ClearDataOutRegister = GpioBase(Port) + GPIO_CLEARDATAOUT;

  switch (Mode)
  {
    case GPIO_MODE_INPUT:
      MmioAndThenOr32(OutputEnableRegister, ~GPIO_OE_MASK(Pin), GPIO_OE_INPUT(Pin));
      break;

    case GPIO_MODE_OUTPUT_0:
      MmioWrite32 (ClearDataOutRegister, GPIO_CLEARDATAOUT_BIT(Pin));
      MmioAndThenOr32(OutputEnableRegister, ~GPIO_OE_MASK(Pin), GPIO_OE_OUTPUT(Pin));
      break;

    case GPIO_MODE_OUTPUT_1:
      MmioWrite32 (SetDataOutRegister, GPIO_SETDATAOUT_BIT(Pin));
      MmioAndThenOr32(OutputEnableRegister, ~GPIO_OE_MASK(Pin), GPIO_OE_OUTPUT(Pin));
      break;

    default:
      return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
GetMode (
  IN  EMBEDDED_GPIO       *This,
  IN  EMBEDDED_GPIO_PIN   Gpio,
  OUT EMBEDDED_GPIO_MODE  *Mode
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
SetPull (
  IN  EMBEDDED_GPIO       *This,
  IN  EMBEDDED_GPIO_PIN   Gpio,
  IN  EMBEDDED_GPIO_PULL  Direction
  )
{
  return EFI_UNSUPPORTED;
}

EMBEDDED_GPIO Gpio = {
  Get,
  Set,
  GetMode,
  SetPull
};

EFI_STATUS
GpioInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS  Status;
  
  Status = gBS->InstallMultipleProtocolInterfaces(&ImageHandle, &gEmbeddedGpioProtocolGuid, &Gpio, NULL);
  return Status;
}