#pragma once

  #if 0
  // SoC registers. L3 interconnects
   #define SOC_REGISTERS_L3_PHYSICAL_BASE       0x68000000
    #define SOC_REGISTERS_L3_PHYSICAL_LENGTH     0x08000000
  #define SOC_REGISTERS_L3_ATTRIBUTES          ARM_MEMORY_REGION_ATTRIBUTE_DEVICE
  #endif
  
  // GPMC
  #define SOC_REGISTERS_L3_PHYSICAL_BASE       0x50000000
  #define SOC_REGISTERS_L3_PHYSICAL_LENGTH     0x08000000
   #define SOC_REGISTERS_L3_ATTRIBUTES          ARM_MEMORY_REGION_ATTRIBUTE_DEVICE
  
    // SoC registers. L4 interconnects
  #define SOC_REGISTERS_L4_PHYSICAL_BASE       0x48000000
    #define SOC_REGISTERS_L4_PHYSICAL_LENGTH     0x08000000
  #define SOC_REGISTERS_L4_ATTRIBUTES          ARM_MEMORY_REGION_ATTRIBUTE_DEVICE
