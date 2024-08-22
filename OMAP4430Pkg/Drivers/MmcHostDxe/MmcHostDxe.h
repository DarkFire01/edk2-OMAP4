/** @file
*
*  Copyright (c) 2011, ARM Limited. All rights reserved.
*  
*  This program and the accompanying materials                          
*  are licensed and made available under the terms and conditions of the BSD License         
*  which accompanies this distribution.  The full text of the license may be found at        
*  http://opensource.org/licenses/bsd-license.php                                            
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
*
**/

#ifndef _MMC_HOST_DXE_H_
#define _MMC_HOST_DXE_H_

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
 
#include <Library/OmapDmaLib.h>
 

#include <Protocol/EmbeddedExternalDevice.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/MmcHost.h>

#include <OMAP4430/OMAP4430.h>
#include <TWL6040.h>

//MMC/SD/SDIO1 register definitions.
#define MMCHS1BASE        0x4809C000
#define MMC_REFERENCE_CLK (96000000)

#define MMCHS_SYSCONFIG   (MMCHS1BASE + 0x110)
#define SOFTRESET         BIT1
#define ENAWAKEUP         BIT2

#define MMCHS_SYSSTATUS   (MMCHS1BASE + 0x114)
#define RESETDONE_MASK    BIT0
#define RESETDONE         BIT0

#define MMCHS_CSRE        (MMCHS1BASE + 0x124)
#define MMCHS_SYSTEST     (MMCHS1BASE + 0x128)

#define MMCHS_CON         (MMCHS1BASE + 0x12C)
#define OD                BIT0
#define NOINIT            (0x0UL << 1)
#define INIT              BIT1
#define HR                BIT2
#define STR               BIT3
#define MODE              BIT4
#define DW8_1_4_BIT       (0x0UL << 5)
#define DW8_8_BIT         BIT5
#define MIT               BIT6
#define CDP               BIT7
#define WPP               BIT8
#define CTPL              BIT11
#define CEATA_OFF         (0x0UL << 12)
#define CEATA_ON          BIT12

#define MMCHS_PWCNT       (MMCHS1BASE + 0x130)

#define MMCHS_BLK         (MMCHS1BASE + 0x204)
#define BLEN_512BYTES     (0x200UL << 0)

#define MMCHS_ARG         (MMCHS1BASE + 0x208)

#define MMCHS_CMD         (MMCHS1BASE + 0x20C)
#define DE_ENABLE         BIT0
#define BCE_ENABLE        BIT1
#define ACEN_ENABLE       BIT2
#define DDIR_READ         BIT4
#define DDIR_WRITE        (0x0UL << 4)
#define MSBS_SGLEBLK      (0x0UL << 5)
#define MSBS_MULTBLK      BIT5
#define RSP_TYPE_MASK     (0x3UL << 16)
#define RSP_TYPE_136BITS  BIT16
#define RSP_TYPE_48BITS   (0x2UL << 16)
#define CCCE_ENABLE       BIT19
#define CICE_ENABLE       BIT20
#define DP_ENABLE         BIT21 
#define INDX(CMD_INDX)    ((CMD_INDX & 0x3F) << 24)

#define MMCHS_RSP10       (MMCHS1BASE + 0x210)
#define MMCHS_RSP32       (MMCHS1BASE + 0x214)
#define MMCHS_RSP54       (MMCHS1BASE + 0x218)
#define MMCHS_RSP76       (MMCHS1BASE + 0x21C)
#define MMCHS_DATA        (MMCHS1BASE + 0x220)

#define MMCHS_PSTATE      (MMCHS1BASE + 0x224)
#define CMDI_MASK         BIT0
#define CMDI_ALLOWED      (0x0UL << 0)
#define CMDI_NOT_ALLOWED  BIT0
#define DATI_MASK         BIT1
#define DATI_ALLOWED      (0x0UL << 1)
#define DATI_NOT_ALLOWED  BIT1

#define MMCHS_HCTL        (MMCHS1BASE + 0x228)
#define DTW_1_BIT         (0x0UL << 1)
#define DTW_4_BIT         BIT1
#define SDBP_MASK         BIT8
#define SDBP_OFF          (0x0UL << 8)
#define SDBP_ON           BIT8
#define SDVS_1_8_V        (0x5UL << 9)
#define SDVS_3_0_V        (0x6UL << 9)
#define IWE               BIT24

#define MMCHS_SYSCTL      (MMCHS1BASE + 0x22C)
#define ICE               BIT0
#define ICS_MASK          BIT1
#define ICS               BIT1
#define CEN               BIT2
#define CLKD_MASK         (0x3FFUL << 6)
#define CLKD_80KHZ        (0x258UL) //(96*1000/80)/2
#define CLKD_400KHZ       (0xF0UL)
#define DTO_MASK          (0xFUL << 16)
#define DTO_VAL           (0xEUL << 16)
#define SRA               BIT24
#define SRC_MASK          BIT25
#define SRC               BIT25
#define SRD               BIT26

#define MMCHS_STAT        (MMCHS1BASE + 0x230)
#define CC                BIT0
#define TC                BIT1
#define BWR               BIT4
#define BRR               BIT5
#define ERRI              BIT15
#define CTO               BIT16
#define DTO               BIT20
#define DCRC              BIT21
#define DEB               BIT22

#define MMCHS_IE          (MMCHS1BASE + 0x234)
#define CC_EN             BIT0
#define TC_EN             BIT1
#define BWR_EN            BIT4
#define BRR_EN            BIT5
#define CTO_EN            BIT16
#define CCRC_EN           BIT17
#define CEB_EN            BIT18
#define CIE_EN            BIT19
#define DTO_EN            BIT20
#define DCRC_EN           BIT21
#define DEB_EN            BIT22
#define CERR_EN           BIT28
#define BADA_EN           BIT29

#define MMCHS_ISE         (MMCHS1BASE + 0x238)
#define CC_SIGEN          BIT0
#define TC_SIGEN          BIT1
#define BWR_SIGEN         BIT4
#define BRR_SIGEN         BIT5
#define CTO_SIGEN         BIT16
#define CCRC_SIGEN        BIT17
#define CEB_SIGEN         BIT18
#define CIE_SIGEN         BIT19
#define DTO_SIGEN         BIT20
#define DCRC_SIGEN        BIT21
#define DEB_SIGEN         BIT22
#define CERR_SIGEN        BIT28
#define BADA_SIGEN        BIT29

#define MMCHS_AC12        (MMCHS1BASE + 0x23C)

#define MMCHS_CAPA        (MMCHS1BASE + 0x240)
#define VS30              BIT25
#define VS18              BIT26

#define MMCHS_CUR_CAPA    (MMCHS1BASE + 0x248)
#define MMCHS_REV         (MMCHS1BASE + 0x2FC)

#define CMD0              INDX(0)
#define CMD0_INT_EN       (CC_EN | CEB_EN)

#define CMD1              (INDX(1) | RSP_TYPE_48BITS)
#define CMD1_INT_EN       (CC_EN | CEB_EN | CTO_EN)

#define CMD2              (INDX(2) | CCCE_ENABLE | RSP_TYPE_136BITS)
#define CMD2_INT_EN       (CERR_EN | CIE_EN | CCRC_EN | CC_EN | CEB_EN | CTO_EN)

#define CMD3              (INDX(3) | CICE_ENABLE | CCCE_ENABLE | RSP_TYPE_48BITS)
#define CMD3_INT_EN       (CERR_EN | CIE_EN | CCRC_EN | CC_EN | CEB_EN | CTO_EN)

#define CMD5              (INDX(5) | RSP_TYPE_48BITS)
#define CMD5_INT_EN       (CC_EN | CEB_EN | CTO_EN)

#define CMD7              (INDX(7) | CICE_ENABLE | CCCE_ENABLE | RSP_TYPE_48BITS)
#define CMD7_INT_EN       (CERR_EN | CIE_EN | CCRC_EN | CC_EN | CEB_EN | CTO_EN)

#define CMD8              (INDX(8) | CICE_ENABLE | CCCE_ENABLE | RSP_TYPE_48BITS)
#define CMD8_INT_EN       (CERR_EN | CIE_EN | CCRC_EN | CC_EN | CEB_EN | CTO_EN)
//Reserved(0)[12:31], Supply voltage(1)[11:8], check pattern(0xCE)[7:0] = 0x1CE
#define CMD8_ARG          (0x0UL << 12 | BIT8 | 0xCEUL << 0)

#define CMD9              (INDX(9) | CCCE_ENABLE | RSP_TYPE_136BITS)
#define CMD9_INT_EN       (CCRC_EN | CC_EN | CEB_EN | CTO_EN)

#define CMD16             (INDX(16) | CICE_ENABLE | CCCE_ENABLE | RSP_TYPE_48BITS)
#define CMD16_INT_EN      (CERR_EN | CIE_EN | CCRC_EN | CC_EN | CEB_EN | CTO_EN)

#define CMD17             (INDX(17) | DP_ENABLE | CICE_ENABLE | CCCE_ENABLE | RSP_TYPE_48BITS | DDIR_READ)
#define CMD17_INT_EN      (CERR_EN | CIE_EN | CCRC_EN | CC_EN | TC_EN | BRR_EN | CTO_EN | DTO_EN | DCRC_EN | DEB_EN | CEB_EN)

#define CMD18             (INDX(18) | DP_ENABLE | CICE_ENABLE | CCCE_ENABLE | RSP_TYPE_48BITS | MSBS_MULTBLK | DDIR_READ | BCE_ENABLE | DE_ENABLE)
#define CMD18_INT_EN      (CERR_EN | CIE_EN | CCRC_EN | CC_EN | TC_EN | BRR_EN | CTO_EN | DTO_EN | DCRC_EN | DEB_EN | CEB_EN)

#define CMD23             (INDX(23) | CICE_ENABLE | CCCE_ENABLE | RSP_TYPE_48BITS)
#define CMD23_INT_EN      (CERR_EN | CIE_EN | CCRC_EN | CC_EN | CEB_EN | CTO_EN)

#define CMD24             (INDX(24) | DP_ENABLE | CICE_ENABLE | CCCE_ENABLE | RSP_TYPE_48BITS | DDIR_WRITE)
#define CMD24_INT_EN      (CERR_EN | CIE_EN | CCRC_EN | CC_EN | TC_EN | BWR_EN | CTO_EN | DTO_EN | DCRC_EN | DEB_EN | CEB_EN)

#define CMD25             (INDX(25) | DP_ENABLE | CICE_ENABLE | CCCE_ENABLE | RSP_TYPE_48BITS | MSBS_MULTBLK | DDIR_READ | BCE_ENABLE | DE_ENABLE)
#define CMD25_INT_EN      (CERR_EN | CIE_EN | CCRC_EN | CC_EN | TC_EN | BRR_EN | CTO_EN | DTO_EN | DCRC_EN | DEB_EN | CEB_EN)

#define CMD55             (INDX(55) | CICE_ENABLE | CCCE_ENABLE | RSP_TYPE_48BITS)
#define CMD55_INT_EN      (CERR_EN | CIE_EN | CCRC_EN | CC_EN | CEB_EN | CTO_EN)

#define ACMD41            (INDX(41) | RSP_TYPE_48BITS)
#define ACMD41_INT_EN     (CERR_EN | CIE_EN | CCRC_EN | CC_EN | CEB_EN | CTO_EN)

#define ACMD6             (INDX(6) | RSP_TYPE_48BITS)
#define ACMD6_INT_EN      (CERR_EN | CIE_EN | CCRC_EN | CC_EN | CEB_EN | CTO_EN)
#define MAX_RETRY_COUNT  (100*5)

extern EFI_BLOCK_IO_PROTOCOL gBlockIo;

// CONTROL_CORE

#define CONTROL_CORE_U_BASE                          (0x4A002000)
#define CONTROL_CORE_ID_CODE                         (CONTROL_CORE_U_BASE+0x204)
#define CONTROL_CORE_LDOSRAM_IVA_VOLTAGE_CTRL        (CONTROL_CORE_U_BASE+0x320)
#define CONTROL_CORE_LDOSRAM_MPU_VOLTAGE_CTRL        (CONTROL_CORE_U_BASE+0x324)
#define CONTROL_CORE_LDOSRAM_CORE_VOLTAGE_CTRL       (CONTROL_CORE_U_BASE+0x328)

#define CONTROL_CORE_LDOSRAM_VOLTAGE_CTRL_VAL               (0x0401040F)

 
#define PBIASVMODE3V                                        (BIT21)
#define PBIASLITEPWRDNZ                                     (BIT22)
#define PBIASVMODEERR                                       (BIT23)
#define PBIASHIZ                                            (BIT25)
#define PBIASPWRDNZ                                         (BIT26)

// PRM

#define PRM_U_BASE                                          (0x4A307B00)
 
#define PRM_RSTST_GLOBAL_COLD_RST_MASK                      (0x1)

#define PRM_RSTCTRL_RST_GLOBAL_COLD_SW_VAL                  (0x2)

#define PRM_VC_VAL_BYPASS                                   (PRM_U_BASE+0xA0)
#define PRM_VC_VAL_BYPASS_REGADDR_POS                       (8)
#define PRM_VC_VAL_BYPASS_DATA_POS                          (16)

#define PRM_VC_CFG_I2C_MODE                                 (PRM_U_BASE+0xA8)
#define PRM_VC_CFG_I2C_CLK                                  (PRM_U_BASE+0xAC)

#define VC_CFG_I2C_MODE_VAL                                 (0x0)
#define VC_CFG_I2C_CLK_VAL                                  (0x167FFB)

#define PMIC_SMPS_ID0_SLAVE_ADDR                            (0x12)
#define PMIC_VCORE3_CFG_FORCE_REGADDR                       (0x61)
#define PMIC_VCORE1_CFG_FORCE_REGADDR                       (0x55)
#define PMIC_VCORE2_CFG_FORCE_REGADDR                       (0x5B)
#define PMIC_VCORE3_CFG_FORCE_VSEL                          (0x28)
#define PMIC_VCORE1_CFG_FORCE_VSEL_VDD_MPU_4430             (0x32)
#define PMIC_VCORE1_CFG_FORCE_VSEL_VDD_CORE_4460            (0x28)
#define PMIC_VCORE2_CFG_FORCE_VSEL                          (0x28)

#define TPS62361_SLAVE_ADDR                                 (0x60)
#define TPS62361_SET1_REG_ADDR                              (0x01)
#define TPS62361_SET1_REG_VAL                               (0x46)

// CKGEN_CM1

#define CKGEN_CM1_U_BASE                                    (0x4A004100)

#define CKGEN_CM1_CM_DIV_M3_DPLL_CORE                       (CKGEN_CM1_U_BASE+0x34)
#define CKGEN_CM1_CM_DIV_M4_DPLL_CORE                       (CKGEN_CM1_U_BASE+0x38)
#define CKGEN_CM1_CM_DIV_M5_DPLL_CORE                       (CKGEN_CM1_U_BASE+0x3C)
#define CKGEN_CM1_CM_DIV_M6_DPLL_CORE                       (CKGEN_CM1_U_BASE+0x40)
#define CKGEN_CM1_CM_DIV_M7_DPLL_CORE                       (CKGEN_CM1_U_BASE+0x44)

#define CKGEN_CM1_CM_DIV_M2_DPLL_MPU                        (CKGEN_CM1_U_BASE+0x70)

#define CKGEN_CM1_CM_CLKMODE_DPLL_IVA                       (CKGEN_CM1_U_BASE+0xA0)
#define CKGEN_CM1_CM_CLKSEL_DPLL_IVA                        (CKGEN_CM1_U_BASE+0xAC)
#define CKGEN_CM1_CM_DIV_M4_DPLL_IVA                        (CKGEN_CM1_U_BASE+0xB8)
#define CKGEN_CM1_CM_DIV_M5_DPLL_IVA                        (CKGEN_CM1_U_BASE+0xBC)
#define CKGEN_CM1_CM_BYPCLK_DPLL_IVA                        (CKGEN_CM1_U_BASE+0xDC)

#define CKGEN_CM1_CM_CLKSEL_DPLL_IVA_CLKSEL_VAL             (0x19c10)
#define CKGEN_CM1_CM_CLKSEL_DPLL_IVA_M4_VAL                 (4)
#define CKGEN_CM1_CM_CLKSEL_DPLL_IVA_M5_VAL                 (7)
#define CKGEN_CM1_CM_CLKSEL_DPLL_IVA_BYCLK_VAL              (1)
#define CKGEN_CM1_CM_CLKSEL_DPLL_IVA_CLKMODE_VAL            (7)

#define CKGEN_CM1_CM_CLKMODE_DPLL_ABE                       (CKGEN_CM1_U_BASE+0xE0)
#define CKGEN_CM1_CM_CLKSEL_DPLL_ABE                        (CKGEN_CM1_U_BASE+0xEC)
#define CKGEN_CM1_CM_DIV_M2_DPLL_ABE                        (CKGEN_CM1_U_BASE+0xF0)
#define CKGEN_CM1_CM_DIV_M3_DPLL_ABE                        (CKGEN_CM1_U_BASE+0xF4)

#define CKGEN_CM1_CM_CLKSEL_DPLL_ABE_VAL                    (0x82ee00)
#define CKGEN_CM1_CM_DIV_M2_DPLL_ABE_VAL                    (1)
#define CKGEN_CM1_CM_DIV_M3_DPLL_ABE_VAL                    (1)
#define CKGEN_CM1_CM_CLKMODE_DPLL_ABE_VAL                   (0xf27)

#define CKGEN_CM1_CM_SHADOW_FREQ_CONFIG1                    (CKGEN_CM1_U_BASE+0x160)
#define CM_SHADOW_FREQ_CONFIG1_DPLL_CORE_M2_DIV_OPP100      (1<<11)
#define CM_SHADOW_FREQ_CONFIG1_DPLL_CORE_DPLL_EN_LOCK       (7<<8)
#define CM_SHADOW_FREQ_CONFIG1_DLL_RESET_RST                (1<<3)
#define CM_SHADOW_FREQ_CONFIG1_FREQ_UPDATE_START            (1<<0)

#define CM_DIV_M2_DPLL_MPU_OPP100_VAL                       (1)
#define CM_DIV_M3_DPLL_CORE_OPP100_VAL                      (5)
#define CM_DIV_M4_DPLL_CORE_OPP100_VAL                      (8)
#define CM_DIV_M5_DPLL_CORE_OPP100_VAL                      (4)
#define CM_DIV_M6_DPLL_CORE_OPP100_VAL                      (6)
#define CM_DIV_M7_DPLL_CORE_OPP100_VAL                      (5)

// CKGEN_CM2

#define CKGEN_CM2_U_BASE                                    (0x4A008100)

#define CKGEN_CM2_CM_CLKMODE_DPLL_USB                       (CKGEN_CM2_U_BASE+0x80)
#define CKGEN_CM2_CM_CLKSEL_DPLL_USB                        (CKGEN_CM2_U_BASE+0x8C)
#define CKGEN_CM2_CM_DIV_M2_DPLL_USB                        (CKGEN_CM2_U_BASE+0x90)

#define CKGEN_CM2_CM_CLKSEL_DPLL_USB_VAL                    (0x25817)
#define CKGEN_CM2_CM_DIV_M2_DPLL_USB_VAL                    (0x282)
#define CKGEN_CM2_CM_CLKMODE_DPLL_USB_VAL                   (0x7)

// RESTORE_CM1

#define RESTORE_CM1_U_BASE                                  (0x4A004E00)

#define RESTORE_CM1_CM_DIV_M2_DPLL_CORE_RESTORE             (RESTORE_CM1_U_BASE+0x4)
#define RESTORE_CM1_CM_DIV_M3_DPLL_CORE_RESTORE             (RESTORE_CM1_U_BASE+0x8)
#define RESTORE_CM1_CM_DIV_M5_DPLL_CORE_RESTORE             (RESTORE_CM1_U_BASE+0x10)
#define RESTORE_CM1_CM_DIV_M6_DPLL_CORE_RESTORE             (RESTORE_CM1_U_BASE+0x14)
#define RESTORE_CM1_CM_DIV_M7_DPLL_CORE_RESTORE             (RESTORE_CM1_U_BASE+0x18)
#define RESTORE_CM1_CM_CLKSEL_DPLL_CORE_RESTORE             (RESTORE_CM1_U_BASE+0x1C)
#define RESTORE_CM1_CM_SHADOW_FREQ_CONFIG1_RESTORE          (RESTORE_CM1_U_BASE+0x30)

// SCRM

#define SCRM_U_BASE                                         (0x4A30A000)

#define SCRM_AUXCLK3                                        (SCRM_U_BASE+0x31C)
#define SCRM_AUXCLK3_VAL                                    (0x00010100)

#endif
