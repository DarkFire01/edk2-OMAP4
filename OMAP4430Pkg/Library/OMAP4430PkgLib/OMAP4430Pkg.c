/** @file
*
*  Copyright (c) 2018, Linaro Limited. All rights reserved.
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

#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>

#include <Ppi/ArmMpCoreInfo.h>
#include <Omap4430/Omap4430.h>

VOID
VolConSetup (
  IN  UINTN                     PmcSlaveAddr,
  IN  UINTN                     SmpsAddr,
  IN  UINTN                     Data
  )
{
  // Setup VolCon command
  MmioWrite32 (PRM_VC_VAL_BYPASS,
               ((Data<<PRM_VC_VAL_BYPASS_DATA_POS)|
               (SmpsAddr<<PRM_VC_VAL_BYPASS_REGADDR_POS)|
               (PmcSlaveAddr)) );

  // Validate command
  MmioOr32 (PRM_VC_VAL_BYPASS, BIT24);
  
  // Wait for command completion
  while( MmioRead32(PRM_VC_VAL_BYPASS) & BIT24 );
}

VOID
VolConInit (
  VOID
  )
{
  // Sram Ldo Voltage Control Override (MPU, CORE, IVA)
  MmioWrite32 (CONTROL_CORE_LDOSRAM_IVA_VOLTAGE_CTRL,
               CONTROL_CORE_LDOSRAM_VOLTAGE_CTRL_VAL);
  MmioWrite32 (CONTROL_CORE_LDOSRAM_MPU_VOLTAGE_CTRL,
               CONTROL_CORE_LDOSRAM_VOLTAGE_CTRL_VAL);
  MmioWrite32 (CONTROL_CORE_LDOSRAM_CORE_VOLTAGE_CTRL,
               CONTROL_CORE_LDOSRAM_VOLTAGE_CTRL_VAL);

  // Voltage controller setup
  MmioWrite32 (PRM_VC_CFG_I2C_MODE, VC_CFG_I2C_MODE_VAL);
  MmioWrite32 (PRM_VC_CFG_I2C_CLK, VC_CFG_I2C_CLK_VAL);
}

VOID
ClockModuleEnable (
  IN  UINTN                     Addr,
  IN  UINTN                     Mode
  )
{
  MmioOr32 (Addr, Mode);
  while( (MmioRead32(Addr) & (BIT16|BIT17)) != 0 );
}

VOID
ClockInit (
  UINT32 BoardRevision
  )
{
  // Init Voltage Controller
  VolConInit ();

  // Setup CORE, MPU, IVA voltages for OPP100 operation

  ClockModuleEnable (0x4A307838, 0x1); //cm_wkup_gpio1_clkctrl: AUTO

  if( BoardRevision == 1 ) {
    //   TPS62361       => VDD_MPU
    //   TWL6030 VCORE2 => VDD_IVA
    //   TWL6030 VCORE1 => VDD_CORE

	// Setup CORE voltage
    VolConSetup(PMIC_SMPS_ID0_SLAVE_ADDR,
	            PMIC_VCORE1_CFG_FORCE_REGADDR,
                PMIC_VCORE1_CFG_FORCE_VSEL_VDD_CORE_4460);

    // Clear TPS VSEL0
    MmioAnd32 (GPIO1_BASE + GPIO_OE, ~BIT7);
    MmioOr32 (GPIO1_BASE + GPIO_CLEARDATAOUT, BIT7);

	// Setup MPU voltage
    VolConSetup(TPS62361_SLAVE_ADDR,
	            TPS62361_SET1_REG_ADDR,
                TPS62361_SET1_REG_VAL);

    // Set TPS VSEL0
    MmioOr32 (GPIO1_BASE + GPIO_SETDATAOUT, BIT7);
  }
  else {
    //   TWL6030 VCORE1 => VDD_MPU
    //   TWL6030 VCORE2 => VDD_IVA
    //   TWL6030 VCORE3 => VDD_CORE

	// Setup CORE voltage
    VolConSetup(PMIC_SMPS_ID0_SLAVE_ADDR,
	            PMIC_VCORE3_CFG_FORCE_REGADDR,
                PMIC_VCORE3_CFG_FORCE_VSEL);

	// Setup MPU voltage
    VolConSetup(PMIC_SMPS_ID0_SLAVE_ADDR,
	            PMIC_VCORE1_CFG_FORCE_REGADDR,
                PMIC_VCORE1_CFG_FORCE_VSEL_VDD_MPU_4430);
  }

  // Setup IVA voltage
  VolConSetup(PMIC_SMPS_ID0_SLAVE_ADDR,
              PMIC_VCORE2_CFG_FORCE_REGADDR,
              PMIC_VCORE2_CFG_FORCE_VSEL);
  
  // Accelerate MPU frequency
  MmioWrite32 (CKGEN_CM1_CM_DIV_M2_DPLL_MPU, CM_DIV_M2_DPLL_MPU_OPP100_VAL);

  // Accelerate CORE clocks
  MmioWrite32 (CKGEN_CM1_CM_DIV_M3_DPLL_CORE, CM_DIV_M3_DPLL_CORE_OPP100_VAL);
  MmioWrite32 (CKGEN_CM1_CM_DIV_M4_DPLL_CORE, CM_DIV_M4_DPLL_CORE_OPP100_VAL);
  MmioWrite32 (CKGEN_CM1_CM_DIV_M5_DPLL_CORE, CM_DIV_M5_DPLL_CORE_OPP100_VAL);
  MmioWrite32 (CKGEN_CM1_CM_DIV_M6_DPLL_CORE, CM_DIV_M6_DPLL_CORE_OPP100_VAL);
  MmioWrite32 (CKGEN_CM1_CM_DIV_M7_DPLL_CORE, CM_DIV_M7_DPLL_CORE_OPP100_VAL);

  // Start core frequency update
  MmioWrite32 ( CKGEN_CM1_CM_SHADOW_FREQ_CONFIG1,
               (CM_SHADOW_FREQ_CONFIG1_DPLL_CORE_M2_DIV_OPP100|
                CM_SHADOW_FREQ_CONFIG1_DPLL_CORE_DPLL_EN_LOCK|
                CM_SHADOW_FREQ_CONFIG1_DLL_RESET_RST|
                CM_SHADOW_FREQ_CONFIG1_FREQ_UPDATE_START));

  // Wait for core frequency update completion
  while( (MmioRead32(CKGEN_CM1_CM_SHADOW_FREQ_CONFIG1) & (BIT0)) != 0 );

  // Setup and lock IVA DPLL
  MmioWrite32 (CKGEN_CM1_CM_CLKSEL_DPLL_IVA,
               CKGEN_CM1_CM_CLKSEL_DPLL_IVA_CLKSEL_VAL);
  MmioWrite32 (CKGEN_CM1_CM_DIV_M4_DPLL_IVA,
               CKGEN_CM1_CM_CLKSEL_DPLL_IVA_M4_VAL);
  MmioWrite32 (CKGEN_CM1_CM_DIV_M5_DPLL_IVA,
               CKGEN_CM1_CM_CLKSEL_DPLL_IVA_M5_VAL);
  MmioWrite32 (CKGEN_CM1_CM_BYPCLK_DPLL_IVA,
               CKGEN_CM1_CM_CLKSEL_DPLL_IVA_BYCLK_VAL);
  MmioWrite32 (CKGEN_CM1_CM_CLKMODE_DPLL_IVA,
               CKGEN_CM1_CM_CLKSEL_DPLL_IVA_CLKMODE_VAL);

  // Setup and lock ABE DPLL
  MmioWrite32 (CKGEN_CM1_CM_CLKSEL_DPLL_ABE,
               CKGEN_CM1_CM_CLKSEL_DPLL_ABE_VAL);
  MmioWrite32 (CKGEN_CM1_CM_DIV_M2_DPLL_ABE,
               CKGEN_CM1_CM_DIV_M2_DPLL_ABE_VAL);
  MmioWrite32 (CKGEN_CM1_CM_DIV_M3_DPLL_ABE,
               CKGEN_CM1_CM_DIV_M3_DPLL_ABE_VAL);
  MmioWrite32 (CKGEN_CM1_CM_CLKMODE_DPLL_ABE,
               CKGEN_CM1_CM_CLKMODE_DPLL_ABE_VAL);

  // Setup and lock USB DPLL
  MmioWrite32 (CKGEN_CM2_CM_CLKSEL_DPLL_USB,
               CKGEN_CM2_CM_CLKSEL_DPLL_USB_VAL);
  MmioWrite32 (CKGEN_CM2_CM_DIV_M2_DPLL_USB,
               CKGEN_CM2_CM_DIV_M2_DPLL_USB_VAL);
  MmioWrite32 (CKGEN_CM2_CM_CLKMODE_DPLL_USB,
               CKGEN_CM2_CM_CLKMODE_DPLL_USB_VAL);

  MmioOr32 (0x4A009020, 0x100); //ISS_CLKCTRL_OPTFCLKEN
  MmioOr32 (0x4A009120, 0xF00); //DSS_CLKCTRL_OPTFCLKEN

  MmioWrite32 (0x4A008900, 0x2); //cm_mpu_m3_clkstctrl: SW_WKUP
  MmioWrite32 (0x4A008F00, 0x2); //cm_ivahd_clkstctrl: SW_WKUP
  MmioWrite32 (0x4A004400, 0x2); //cm_dsp_clkstctrl: SW_WKUP
  MmioWrite32 (0x4A009100, 0x2); //cm_dss_clkstctrl: SW_WKUP
  MmioWrite32 (0x4A009200, 0x2); //cm_sgx_clkstctrl: SW_WKUP
  MmioWrite32 (0x4A004500, 0x2); //cm1_abe_clkstctrl: SW_WKUP
  MmioWrite32 (0x4A008C00, 0x2); //cm_c2c_clkstctrl: SW_WKUP
  MmioWrite32 (0x4A009000, 0x2); //cm_cam_clkstctrl: SW_WKUP
  MmioWrite32 (0x4A008A00, 0x2); //cm_sdma_clkstctrl: SW_WKUP

  MmioWrite32 (0x4A008E20, 0x1); //cm_l3instr_l3_3_clkctrl: AUTO
  MmioWrite32 (0x4A008E28, 0x1); //cm_l3instr_l3_instr_clkctrl: AUTO
  MmioWrite32 (0x4A008E40, 0x1); //cm_l3instr_intrconn_wp1_clkctrl: AUTO
  MmioWrite32 (0x4A009338, 0x1); //cm_l3init_hsi_clkctrl: AUTO
  
  ClockModuleEnable (0x4A004528, 0x2); //cm1_abe_aess_clkctrl: ENABLE
  
  // TODO: pdm needs clock enabled externally to make it functional
  MmioWrite32 (0x4A004530, 0x2); //cm1_abe_pdm_clkctrl: ENABLE

  ClockModuleEnable (0x4A004538, 0x2); //cm1_abe_dmic_clkctrl: ENABLE
  ClockModuleEnable (0x4A004540, 0x2); //cm1_abe_mcasp_clkctrl: ENABLE
  ClockModuleEnable (0x4A004548, 0x2); //cm1_abe_mcbsp1_clkctrl: ENABLE
  ClockModuleEnable (0x4A004550, 0x2); //cm1_abe_mcbsp2_clkctrl: ENABLE
  ClockModuleEnable (0x4A004558, 0x2); //cm1_abe_mcbsp3_clkctrl: ENABLE
  ClockModuleEnable (0x4A004560, 0x2); //cm1_abe_slimbus_clkctrl: ENABLE
  ClockModuleEnable (0x4A004568, 0x2); //cm1_abe_timer5_clkctrl: ENABLE
  ClockModuleEnable (0x4A004570, 0x2); //cm1_abe_timer6_clkctrl: ENABLE
  ClockModuleEnable (0x4A004578, 0x2); //cm1_abe_timer7_clkctrl: ENABLE
  ClockModuleEnable (0x4A004580, 0x2); //cm1_abe_timer8_clkctrl: ENABLE
  ClockModuleEnable (0x4A004588, 0x2); //cm1_abe_wdt3_clkctrl: ENABLE
  ClockModuleEnable (0x4A009450, 0x2); //cm_l4per_gptimer9_clkctrl: ENABLE
  ClockModuleEnable (0x4A009428, 0x2); //cm_l4per_gptimer10_clkctrl: ENABLE
  ClockModuleEnable (0x4A009430, 0x2); //cm_l4per_gptimer11_clkctrl: ENABLE
  ClockModuleEnable (0x4A009440, 0x2); //cm_l4per_gptimer3_clkctrl: ENABLE
  ClockModuleEnable (0x4A009448, 0x2); //cm_l4per_gptimer4_clkctrl: ENABLE
  ClockModuleEnable (0x4A009488, 0x2); //cm_l4per_hdq1w_clkctrl: ENABLE
  ClockModuleEnable (0x4A0094E0, 0x2); //cm_l4per_mcbsp4_clkctrl: ENABLE
  ClockModuleEnable (0x4A0094F8, 0x2); //cm_l4per_mcspi2_clkctrl: ENABLE
  ClockModuleEnable (0x4A009500, 0x2); //cm_l4per_mcspi3_clkctrl: ENABLE
  ClockModuleEnable (0x4A009508, 0x2); //cm_l4per_mcspi4_clkctrl: ENABLE
  ClockModuleEnable (0x4A009520, 0x2); //cm_l4per_mmcsd3_clkctrl: ENABLE
  ClockModuleEnable (0x4A009528, 0x2); //cm_l4per_mmcsd4_clkctrl: ENABLE
  ClockModuleEnable (0x4A009560, 0x2); //cm_l4per_mmcsd5_clkctrl: ENABLE
  ClockModuleEnable (0x4A009540, 0x2); //cm_l4per_uart1_clkctrl: ENABLE
  ClockModuleEnable (0x4A009548, 0x2); //cm_l4per_uart2_clkctrl: ENABLE
  ClockModuleEnable (0x4A009558, 0x2); //cm_l4per_uart4_clkctrl: ENABLE
  ClockModuleEnable (0x4A009438, 0x2); //gptimer2: ENABLE
  ClockModuleEnable (0x4A009460, 0x1); //gpio2: AUTO
  ClockModuleEnable (0x4A009468, 0x1); //gpio3: AUTO
  ClockModuleEnable (0x4A009470, 0x101); //gpio4: AUTO+FCLK
  ClockModuleEnable (0x4A009478, 0x1); //gpio5: AUTO
  ClockModuleEnable (0x4A009480, 0x1); //gpio6: AUTO
  ClockModuleEnable (0x4A0094A8, 0x2); //i2c2: ENABLE
  ClockModuleEnable (0x4A0094B0, 0x2); //i2c3: ENABLE
  ClockModuleEnable (0x4A0094B8, 0x2); //i2c4: ENABLE
  ClockModuleEnable (0x4A0094F0, 0x2); //mcspi1: ENABLE
  ClockModuleEnable (0x4A307878, 0x2); //cm_wkup_keyboard_clkctrl: ENABLE
  ClockModuleEnable (0x4A009020, 0x2); //cm_cam_iss_clkctrl: ENABLE
  ClockModuleEnable (0x4A009028, 0x2); //cm_cam_fdif_clkctrl: ENABLE
  ClockModuleEnable (0x4A009120, 0x2); //cm_dss_dss_clkctrl: ENABLE
  ClockModuleEnable (0x4A009220, 0x2); //cm_sgx_sgx_clkctrl: ENABLE
  ClockModuleEnable (0x4A009328, 0x1040002); //mmchs1: ENABLE
  ClockModuleEnable (0x4A009330, 0x1040002); //mmchs2: ENABLE
  ClockModuleEnable (0x4A009358, 0xFF02); //cm_l3init_hsusbhost_clkctrl: ENABLE
  ClockModuleEnable (0x4A009368, 0x1); //usbtll: AUTO

  MmioWrite32 (0x4A008900, 0x3); //cm_mpu_m3_clkstctrl: HW_AUTO
  MmioWrite32 (0x4A008F00, 0x3); //cm_ivahd_clkstctrl: HW_AUTO
  MmioWrite32 (0x4A004400, 0x3); //cm_dsp_clkstctrl: HW_AUTO
  MmioWrite32 (0x4A009100, 0x3); //cm_dss_clkstctrl: HW_AUTO
  MmioWrite32 (0x4A009200, 0x3); //cm_sgx_clkstctrl: HW_AUTO
  MmioWrite32 (0x4A004500, 0x3); //cm1_abe_clkstctrl: HW_AUTO
  MmioWrite32 (0x4A008C00, 0x3); //cm_c2c_clkstctrl: HW_AUTO
  MmioWrite32 (0x4A008A00, 0x3); //cm_sdma_clkstctrl: HW_AUTO
  MmioWrite32 (0x4A009400, 0x3); //l4per: HW_AUTO
  MmioWrite32 (0x4A009300, 0x3); //l3init: HW_AUTO
  MmioWrite32 (0x4A008B00, 0x3); //memif: HW_AUTO
  MmioWrite32 (0x4A008D00, 0x3); //l4cfg: HW_AUTO
  MmioWrite32 (0x4A009000, 0x0); //cm_cam_clkstctrl: NO_SLEEP
}

PAD_CONFIGURATION PadConfigurationTableSharedCore[] = {
    {GPMC_AD0, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat0 */
    {GPMC_AD1, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat1 */
    {GPMC_AD2, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat2 */
    {GPMC_AD3, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat3 */
    {GPMC_AD4, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat4 */
    {GPMC_AD5, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat5 */
    {GPMC_AD6, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat6 */
    {GPMC_AD7, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat7 */
    {GPMC_NOE, (PTU | IEN | OFF_EN | OFF_OUT_PTD | M1)},	 /* sdmmc2_clk */
    {GPMC_NWE, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_cmd */
    {SDMMC1_CLK, (PTU | OFF_EN | OFF_OUT_PTD | M0)},	 /* sdmmc1_clk */
    {SDMMC1_CMD, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_cmd */
    {SDMMC1_DAT0, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat0 */
    {SDMMC1_DAT1, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat1 */
    {SDMMC1_DAT2, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat2 */
    {SDMMC1_DAT3, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat3 */
    {SDMMC1_DAT4, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat4 */
    {SDMMC1_DAT5, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat5 */
    {SDMMC1_DAT6, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat6 */
    {SDMMC1_DAT7, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat7 */
    {I2C1_SCL, (PTU | IEN | M0)},				/* i2c1_scl */
    {I2C1_SDA, (PTU | IEN | M0)},				/* i2c1_sda */
    {I2C2_SCL, (PTU | IEN | M0)},				/* i2c2_scl */
    {I2C2_SDA, (PTU | IEN | M0)},				/* i2c2_sda */
    {I2C3_SCL, (PTU | IEN | M0)},				/* i2c3_scl */
    {I2C3_SDA, (PTU | IEN | M0)},				/* i2c3_sda */
    {I2C4_SCL, (PTU | IEN | M0)},				/* i2c4_scl */
    {I2C4_SDA, (PTU | IEN | M0)},				/* i2c4_sda */
    {UART3_CTS_RCTX, (PTU | IEN | M0)},			/* uart3_tx */
    {UART3_RTS_SD, (M0)},					/* uart3_rts_sd */
    {UART3_RX_IRRX, (IEN | M0)},				/* uart3_rx */
    {UART3_TX_IRTX, (M0)},					/* uart3_tx */
    {USBB1_ULPITLL_CLK, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M4)},/* usbb1_ulpiphy_clk */
    {USBB1_ULPITLL_STP, (OFF_EN | OFF_OUT_PTD | M4)},		/* usbb1_ulpiphy_stp */
    {USBB1_ULPITLL_DIR, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dir */
    {USBB1_ULPITLL_NXT, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_nxt */
    {USBB1_ULPITLL_DAT0, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat0 */
    {USBB1_ULPITLL_DAT1, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat1 */
    {USBB1_ULPITLL_DAT2, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat2 */
    {USBB1_ULPITLL_DAT3, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat3 */
    {USBB1_ULPITLL_DAT4, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat4 */
    {USBB1_ULPITLL_DAT5, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat5 */
    {USBB1_ULPITLL_DAT6, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat6 */
    {USBB1_ULPITLL_DAT7, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat7 */
    {USBB1_HSIC_DATA, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* usbb1_hsic_data */
    {USBB1_HSIC_STROBE, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* usbb1_hsic_strobe */
    {USBC1_ICUSB_DP, (IEN | M0)},					/* usbc1_icusb_dp */
    {USBC1_ICUSB_DM, (IEN | M0)},					/* usbc1_icusb_dm */
    {UNIPRO_TY2, (PTU | IEN | M3)},					/* gpio_1 */
    {GPMC_WAIT1,  (PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M3)},	/* gpio_62 */
    {FREF_CLK2_OUT, (PTU | IEN | M3)},				/* gpio_182: BOARD_ID0 */
	{GPMC_AD8, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M3)},	/* gpio_32 */
	{GPMC_AD9, (PTU | IEN | M3)},					/* gpio_33 */
	{GPMC_AD10, (PTU | IEN | M3)},					/* gpio_34 */
	{GPMC_AD11, (PTU | IEN | M3)},					/* gpio_35 */
	{GPMC_AD12, (PTU | IEN | M3)},					/* gpio_36 */
	{GPMC_AD13, (PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M3)},	/* gpio_37 */
	{GPMC_AD14, (PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M3)},	/* gpio_38 */
	{GPMC_AD15, (PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M3)},	/* gpio_39 */
	{GPMC_A16, (M3)},						/* gpio_40 */
	{GPMC_A17, (PTD | M3)},						/* gpio_41 */
	{GPMC_A18, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* kpd_row6 */
	{GPMC_A19, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* kpd_row7 */
	{GPMC_A20, (IEN | M3)},						/* gpio_44 */
	{GPMC_A21, (M3)},						/* gpio_45 */
	{GPMC_A22, (M3)},						/* gpio_46 */
	{GPMC_A23, (OFF_EN | OFF_PD | OFF_IN | M1)},			/* kpd_col7 */
	{GPMC_A24, (PTD | M3)},						/* gpio_48: BOARD_ID1 (Panda ES only) */
	{GPMC_A25, (PTD | M3)},						/* gpio_49 */
	{GPMC_NCS0, (M3)},						/* gpio_50 */
	{GPMC_NCS1, (IEN | M3)},					/* gpio_51 */
	{GPMC_NCS2, (IEN | M3)},					/* gpio_52 */
	{GPMC_NCS3, (IEN | M3)},					/* gpio_53 */
	{GPMC_NWP, (M3)},						/* gpio_54 */
	{GPMC_CLK, (PTD | M3)},						/* gpio_55 */
	{GPMC_NADV_ALE, (M3)},						/* gpio_56 */
	{GPMC_NBE0_CLE, (M3)},						/* gpio_59 */
	{GPMC_NBE1, (PTD | M3)},					/* gpio_60 */
	{GPMC_WAIT0, (PTU | IEN | M3)},					/* gpio_61 */
	{C2C_DATA11, (PTD | M3)},					/* gpio_100 */
	{C2C_DATA12, (PTU | IEN | M3)},					/* gpio_101: BOARD_ID1 (Panda only) */
	{C2C_DATA13, (PTD | M3)},					/* gpio_102 */
	{C2C_DATA14, (M1)},						/* dsi2_te0 */
	{C2C_DATA15, (PTD | M3)},					/* gpio_104 */
	{HDMI_HPD, (M0)},						/* hdmi_hpd */
	{HDMI_CEC, (M0)},						/* hdmi_cec */
	{HDMI_DDC_SCL, (PTU | M0)},					/* hdmi_ddc_scl */
	{HDMI_DDC_SDA, (PTU | IEN | M0)},				/* hdmi_ddc_sda */
	{CSI21_DX0, (IEN | M0)},					/* csi21_dx0 */
	{CSI21_DY0, (IEN | M0)},					/* csi21_dy0 */
	{CSI21_DX1, (IEN | M0)},					/* csi21_dx1 */
	{CSI21_DY1, (IEN | M0)},					/* csi21_dy1 */
	{CSI21_DX2, (IEN | M0)},					/* csi21_dx2 */
	{CSI21_DY2, (IEN | M0)},					/* csi21_dy2 */
	{CSI21_DX3, (PTD | M7)},					/* csi21_dx3 */
	{CSI21_DY3, (PTD | M7)},					/* csi21_dy3 */
	{CSI21_DX4, (PTD | OFF_EN | OFF_PD | OFF_IN | M7)},		/* csi21_dx4 */
	{CSI21_DY4, (PTD | OFF_EN | OFF_PD | OFF_IN | M7)},		/* csi21_dy4 */
	{CSI22_DX0, (IEN | M0)},					/* csi22_dx0 */
	{CSI22_DY0, (IEN | M0)},					/* csi22_dy0 */
	{CSI22_DX1, (IEN | M0)},					/* csi22_dx1 */
	{CSI22_DY1, (IEN | M0)},					/* csi22_dy1 */
	{CAM_SHUTTER, (OFF_EN | OFF_PD | OFF_OUT_PTD | M0)},		/* cam_shutter */
	{CAM_STROBE, (OFF_EN | OFF_PD | OFF_OUT_PTD | M0)},		/* cam_strobe */
	{CAM_GLOBALRESET, (PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M3)},	/* gpio_83 */
	{ABE_MCBSP2_DR, (IEN | OFF_EN | OFF_OUT_PTD | M0)},		/* abe_mcbsp2_dr */
	{ABE_MCBSP2_DX, (OFF_EN | OFF_OUT_PTD | M0)},			/* abe_mcbsp2_dx */
	{ABE_MCBSP2_FSX, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_mcbsp2_fsx */
	{ABE_MCBSP1_CLKX, (IEN | M0)},					/* abe_mcbsp1_clkx */
	{ABE_MCBSP1_DR, (IEN | M0)},					/* abe_mcbsp1_dr */
	{ABE_MCBSP1_DX, (OFF_EN | OFF_OUT_PTD | M0)},			/* abe_mcbsp1_dx */
	{ABE_MCBSP1_FSX, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_mcbsp1_fsx */
	{ABE_PDM_UL_DATA, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_pdm_ul_data */
	{ABE_PDM_DL_DATA, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_pdm_dl_data */
	{ABE_PDM_FRAME, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_pdm_frame */
	{ABE_PDM_LB_CLK, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_pdm_lb_clk */
	{ABE_CLKS, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_clks */
	{ABE_DMIC_CLK1, (M0)},						/* abe_dmic_clk1 */
	{ABE_DMIC_DIN1, (IEN | M0)},					/* abe_dmic_din1 */
	{ABE_DMIC_DIN2, (PTU | IEN | M3)},				/* gpio_121 */
	{ABE_DMIC_DIN3, (IEN | M0)},					/* abe_dmic_din3 */
	{UART2_CTS, (PTU | IEN | M7)},					/* uart2_cts */
	{UART2_RTS, (M7)},						/* uart2_rts */
	{UART2_RX, (PTU | IEN | M7)},					/* uart2_rx */
	{UART2_TX, (M7)},						/* uart2_tx */
	{HDQ_SIO, (M3)},						/* gpio_127 */
	{MCSPI1_CLK, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* mcspi1_clk */
	{MCSPI1_SOMI, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* mcspi1_somi */
	{MCSPI1_SIMO, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* mcspi1_simo */
	{MCSPI1_CS0, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* mcspi1_cs0 */
	{MCSPI1_CS1, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M3)},	/* mcspi1_cs1 */
	{MCSPI1_CS2, (PTU | OFF_EN | OFF_OUT_PTU | M3)},		/* gpio_139 */
	{MCSPI1_CS3, (PTU | IEN | M3)},					/* gpio_140 */
	{SDMMC5_CLK, (PTU | IEN | OFF_EN | OFF_OUT_PTD | M0)},		/* sdmmc5_clk */
	{SDMMC5_CMD, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc5_cmd */
	{SDMMC5_DAT0, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc5_dat0 */
	{SDMMC5_DAT1, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc5_dat1 */
	{SDMMC5_DAT2, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc5_dat2 */
	{SDMMC5_DAT3, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc5_dat3 */
	{MCSPI4_CLK, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* mcspi4_clk */
	{MCSPI4_SIMO, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* mcspi4_simo */
	{MCSPI4_SOMI, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* mcspi4_somi */
	{MCSPI4_CS0, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* mcspi4_cs0 */
	{UART4_RX, (IEN | M0)},						/* uart4_rx */
	{UART4_TX, (M0)},						/* uart4_tx */
	{USBB2_ULPITLL_CLK, (IEN | M3)},				/* gpio_157 */
	{USBB2_ULPITLL_STP, (IEN | M5)},				/* dispc2_data23 */
	{USBB2_ULPITLL_DIR, (IEN | M5)},				/* dispc2_data22 */
	{USBB2_ULPITLL_NXT, (IEN | M5)},				/* dispc2_data21 */
	{USBB2_ULPITLL_DAT0, (IEN | M5)},				/* dispc2_data20 */
	{USBB2_ULPITLL_DAT1, (IEN | M5)},				/* dispc2_data19 */
	{USBB2_ULPITLL_DAT2, (IEN | M5)},				/* dispc2_data18 */
	{USBB2_ULPITLL_DAT3, (IEN | M5)},				/* dispc2_data15 */
	{USBB2_ULPITLL_DAT4, (IEN | M5)},				/* dispc2_data14 */
	{USBB2_ULPITLL_DAT5, (IEN | M5)},				/* dispc2_data13 */
	{USBB2_ULPITLL_DAT6, (IEN | M5)},				/* dispc2_data12 */
	{USBB2_ULPITLL_DAT7, (IEN | M5)},				/* dispc2_data11 */
	{USBB2_HSIC_DATA, (PTD | OFF_EN | OFF_OUT_PTU | M3)},		/* gpio_169 */
	{USBB2_HSIC_STROBE, (PTD | OFF_EN | OFF_OUT_PTU | M3)},		/* gpio_170 */
	{UNIPRO_TX0, (PTD | IEN | M3)},					/* gpio_171 */
	{UNIPRO_TY0, (OFF_EN | OFF_PD | OFF_IN | M1)},			/* kpd_col1 */
	{UNIPRO_TX1, (OFF_EN | OFF_PD | OFF_IN | M1)},			/* kpd_col2 */
	{UNIPRO_TY1, (OFF_EN | OFF_PD | OFF_IN | M1)},			/* kpd_col3: BOARD_ID2 (gpio_171) */
	{UNIPRO_TX2, (PTU | IEN | M3)},					/* gpio_0 */
	{UNIPRO_RX0, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* kpd_row0 */
	{UNIPRO_RY0, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M3 | DIS)}, /* kpd_row1: BOARD_ID4 (gpio_2) */
	{UNIPRO_RX1, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M3 | DIS)}, /* kpd_row2: BOARD_ID3 (gpio_3) */
	{UNIPRO_RY1, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* kpd_row3 */
	{UNIPRO_RX2, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* kpd_row4 */
	{UNIPRO_RY2, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* kpd_row5 */
	{USBA0_OTG_CE, (PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M0)},	/* usba0_otg_ce */
	{USBA0_OTG_DP, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* usba0_otg_dp */
	{USBA0_OTG_DM, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* usba0_otg_dm */
	{FREF_CLK1_OUT, (M0)},						/* fref_clk1_out */
	{SYS_NIRQ1, (PTU | IEN | M0)},					/* sys_nirq1 */
	{SYS_NIRQ2, (PTU | IEN | M0)},					/* sys_nirq2 */
	{SYS_BOOT0, (PTU | IEN | M3)},					/* gpio_184 */
	{SYS_BOOT1, (M3)},						/* gpio_185 */
	{SYS_BOOT2, (PTD | IEN | M3)},					/* gpio_186 */
	{SYS_BOOT3, (M3)},						/* gpio_187 */
	{SYS_BOOT4, (M3)},						/* gpio_188 */
	{SYS_BOOT5, (PTD | IEN | M3)},					/* gpio_189 */
	{DPM_EMU0, (IEN | M0)},						/* dpm_emu0 */
	{DPM_EMU1, (IEN | M0)},						/* dpm_emu1 */
	{DPM_EMU2, (IEN | M0)},						/* dpm_emu2 */
	{DPM_EMU3, (IEN | M5)},						/* dispc2_data10 */
	{DPM_EMU4, (IEN | M5)},						/* dispc2_data9 */
	{DPM_EMU5, (IEN | M5)},						/* dispc2_data16 */
	{DPM_EMU6, (IEN | M5)},						/* dispc2_data17 */
	{DPM_EMU7, (IEN | M5)},						/* dispc2_hsync */
	{DPM_EMU8, (IEN | M5)},						/* dispc2_pclk */
	{DPM_EMU9, (IEN | M5)},						/* dispc2_vsync */
	{DPM_EMU10, (IEN | M5)},					/* dispc2_de */
	{DPM_EMU11, (IEN | M5)},					/* dispc2_data8 */
	{DPM_EMU12, (IEN | M5)},					/* dispc2_data7 */
	{DPM_EMU13, (IEN | M5)},					/* dispc2_data6 */
	{DPM_EMU14, (IEN | M5)},					/* dispc2_data5 */
	{DPM_EMU15, (IEN | M5)},					/* dispc2_data4 */
	{DPM_EMU16, (M3)},						/* gpio_27 */
	{DPM_EMU17, (IEN | M5)},					/* dispc2_data2 */
	{DPM_EMU18, (IEN | M5)},					/* dispc2_data1 */
	{DPM_EMU19, (IEN | M5)},					/* dispc2_data0 */
};

PAD_CONFIGURATION PadConfigurationTableSharedWkup[] = {
    {PAD1_SR_SCL, (PTU | IEN | M0)}, /* sr_scl */
    {PAD0_SR_SDA, (PTU | IEN | M0)}, /* sr_sda */
    {PAD1_SYS_32K, (IEN | M0)},	 /* sys_32k */
    {PAD0_FREF_CLK3_OUT, (M0)}, /* fref_clk3_out */
	{PAD0_SIM_IO, (IEN | M0)},		/* sim_io */
	{PAD1_SIM_CLK, (M0)},			/* sim_clk */
	{PAD0_SIM_RESET, (M0)},			/* sim_reset */
	{PAD1_SIM_CD, (PTU | IEN | M0)},	/* sim_cd */
	{PAD0_SIM_PWRCTRL, (M0)},		/* sim_pwrctrl */
	{PAD1_FREF_XTAL_IN, (M0)},		/* # */
	{PAD0_FREF_SLICER_IN, (M0)},		/* fref_slicer_in */
	{PAD1_FREF_CLK_IOREQ, (M0)},		/* fref_clk_ioreq */
	{PAD0_FREF_CLK0_OUT, (M2)},		/* sys_drm_msecure */
	{PAD1_FREF_CLK3_REQ, M7},		/* safe mode */
	{PAD0_FREF_CLK4_OUT, (PTU | M3)},	/* led status_2 */
	{PAD0_SYS_NRESPWRON, (M0)},		/* sys_nrespwron */
	{PAD1_SYS_NRESWARM, (M0)},		/* sys_nreswarm */
	{PAD0_SYS_PWR_REQ, (PTU | M0)},		/* sys_pwr_req */
	{PAD1_SYS_PWRON_RESET, (M3)},		/* gpio_wk29 */
	{PAD0_SYS_BOOT6, (IEN | M3)},		/* gpio_wk9 */
	{PAD1_SYS_BOOT7, (IEN | M3)},		/* gpio_wk10 */
};

PAD_CONFIGURATION PadConfigurationTable4430Core[] = {
	{ABE_MCBSP2_CLKX, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_mcbsp2_clkx */
};

PAD_CONFIGURATION PadConfigurationTable4430Wkup[] = {
	{PAD1_FREF_CLK4_REQ, (PTU | M3)},	/* led status_1 */
};

PAD_CONFIGURATION PadConfigurationTable4460Core[] = {
	{ABE_MCBSP2_CLKX, (PTU | OFF_EN | OFF_OUT_PTU | M3)},		/* led status_1 */
};

PAD_CONFIGURATION PadConfigurationTable4460Wkup[] = {
    {PAD1_FREF_CLK4_REQ, (M3)}, /* gpio_wk7 for TPS: Mode 3 */
};

VOID
PadConfiguration (
  UINT32 BoardRevision
  )
{
  UINTN             Index;
  UINTN             NumPinsToConfigure;

  // Calculate number of pins for core domain
  NumPinsToConfigure = sizeof(PadConfigurationTableSharedCore) / sizeof(PAD_CONFIGURATION);
  
  for (Index = 0; Index < NumPinsToConfigure; Index++) {
    // Configure the pin with specific Pad configuration.
    MmioWrite16((OMAP4430_CONTROL_MODULE_CORE_BASE+PadConfigurationTableSharedCore[Index].Off),
	            PadConfigurationTableSharedCore[Index].Val);
  }

  // Calculate number of pins for wkup domain
  NumPinsToConfigure = sizeof(PadConfigurationTableSharedWkup) / sizeof(PAD_CONFIGURATION);
  
  for (Index = 0; Index < NumPinsToConfigure; Index++) {
    // Configure the pin with specific Pad configuration.
    MmioWrite16((OMAP4430_CONTROL_MODULE_WKUP_BASE+PadConfigurationTableSharedWkup[Index].Off),
	            PadConfigurationTableSharedWkup[Index].Val);
  }
  
  // If PandaBoard-ES
  if( BoardRevision == 1 ) {
      // Calculate number of pins for core domain
      NumPinsToConfigure = sizeof(PadConfigurationTable4460Core) / sizeof(PAD_CONFIGURATION);
  
      for (Index = 0; Index < NumPinsToConfigure; Index++) {
        // Configure the pin with specific Pad configuration.
        MmioWrite16((OMAP4430_CONTROL_MODULE_CORE_BASE+PadConfigurationTable4460Core[Index].Off),
	              PadConfigurationTable4460Core[Index].Val);
      }
	  
      // Calculate number of pins for wkup domain
      NumPinsToConfigure = sizeof(PadConfigurationTable4460Wkup) / sizeof(PAD_CONFIGURATION);
  
      for (Index = 0; Index < NumPinsToConfigure; Index++) {
        // Configure the pin with specific Pad configuration.
        MmioWrite16((OMAP4430_CONTROL_MODULE_WKUP_BASE+PadConfigurationTable4460Wkup[Index].Off),
	              PadConfigurationTable4460Wkup[Index].Val);
      }
  }
  else {
      // Calculate number of pins for core domain
      NumPinsToConfigure = sizeof(PadConfigurationTable4430Core) / sizeof(PAD_CONFIGURATION);
  
      for (Index = 0; Index < NumPinsToConfigure; Index++) {
        // Configure the pin with specific Pad configuration.
        MmioWrite16((OMAP4430_CONTROL_MODULE_CORE_BASE+PadConfigurationTable4430Core[Index].Off),
	              PadConfigurationTable4430Core[Index].Val);
      }
	  
      // Calculate number of pins for wkup domain
      NumPinsToConfigure = sizeof(PadConfigurationTable4430Wkup) / sizeof(PAD_CONFIGURATION);
  
      for (Index = 0; Index < NumPinsToConfigure; Index++) {
        // Configure the pin with specific Pad configuration.
        MmioWrite16((OMAP4430_CONTROL_MODULE_WKUP_BASE+PadConfigurationTable4430Wkup[Index].Off),
	              PadConfigurationTable4430Wkup[Index].Val);
      }
  }

  // EMIF pads
  MmioWrite32 (0x4A100638, 0x7c7c7c7c);
  MmioWrite32 (0x4A10063C, 0x7c7c7c7c);
  MmioWrite32 (0x4A100640, 0x7C787C00);
  MmioWrite32 (0x4A100644, 0xA0888C0F);

  MmioWrite32 (0x4A100648, 0x7C7C7C7C);
  MmioWrite32 (0x4A10064C, 0x7C7C7C7C);
  MmioWrite32 (0x4A100650, 0x7C787C00);
  MmioWrite32 (0x4A100654, 0xA0888C0F);

}


ARM_CORE_INFO mHiKey960InfoTable[] = {
  {
    // Cluster 0, Core 0
    0x0, 0x0,

    // MP Core MailBox Set/Get/Clear Addresses and Clear Value
    (UINT64)0xFFFFFFFF
  },
/*
  {
    // Cluster 0, Core 1
    0x0, 0x1,

    // MP Core MailBox Set/Get/Clear Addresses and Clear Value
    (UINT64)0xFFFFFFFF
  },
  {
    // Cluster 0, Core 2
    0x0, 0x2,

    // MP Core MailBox Set/Get/Clear Addresses and Clear Value
    (UINT64)0xFFFFFFFF
  },
  {
    // Cluster 0, Core 3
    0x0, 0x3,

    // MP Core MailBox Set/Get/Clear Addresses and Clear Value
    (UINT64)0xFFFFFFFF
  },
  {
    // Cluster 1, Core 0
    0x1, 0x0,

    // MP Core MailBox Set/Get/Clear Addresses and Clear Value
    (UINT64)0xFFFFFFFF
  },
  {
    // Cluster 1, Core 1
    0x1, 0x1,

    // MP Core MailBox Set/Get/Clear Addresses and Clear Value
    (UINT64)0xFFFFFFFF
  },
  {
    // Cluster 1, Core 2
    0x1, 0x2,

    // MP Core MailBox Set/Get/Clear Addresses and Clear Value
    (UINT64)0xFFFFFFFF
  },
  {
    // Cluster 1, Core 3
    0x1, 0x3,

    // MP Core MailBox Set/Get/Clear Addresses and Clear Value
    (UINT64)0xFFFFFFFF
  }
*/
};

/**
  Return the current Boot Mode

  This function returns the boot reason on the platform

  @return   Return the current Boot Mode of the platform

**/
EFI_BOOT_MODE
ArmPlatformGetBootMode (
  VOID
  )
{
  return BOOT_WITH_FULL_CONFIGURATION;
}

/**
  Initialize controllers that must setup in the normal world

  This function is called by the ArmPlatformPkg/Pei or ArmPlatformPkg/Pei/PlatformPeim
  in the PEI phase.

**/
RETURN_STATUS
ArmPlatformInitialize (
  IN  UINTN                     MpId
  )
{

    // Set up Pin muxing.
  PadConfiguration (0);

  // Set up system clocking
  ClockInit (0);

  // Make sure GPMC region region 0 is disabled
  // Not doing so makes gpmc_init hang early in kernel init
  //MmioAnd32 (GPMC_CONFIG7_0, ~CSVALID);
  return RETURN_SUCCESS;
}

EFI_STATUS
PrePeiCoreGetMpCoreInfo (
  OUT UINTN                   *CoreCount,
  OUT ARM_CORE_INFO           **ArmCoreTable
  )
{
  // Only support one cluster
  *CoreCount    = sizeof(mHiKey960InfoTable) / sizeof(ARM_CORE_INFO);
  *ArmCoreTable = mHiKey960InfoTable;
  return EFI_SUCCESS;
}

// Needs to be declared in the file. Otherwise gArmMpCoreInfoPpiGuid is undefined in the contect of PrePeiCore
EFI_GUID mArmMpCoreInfoPpiGuid = ARM_MP_CORE_INFO_PPI_GUID;
ARM_MP_CORE_INFO_PPI mMpCoreInfoPpi = { PrePeiCoreGetMpCoreInfo };

EFI_PEI_PPI_DESCRIPTOR      gPlatformPpiTable[] = {
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI,
    &mArmMpCoreInfoPpiGuid,
    &mMpCoreInfoPpi
  }
};

VOID
ArmPlatformGetPlatformPpiList (
  OUT UINTN                   *PpiListSize,
  OUT EFI_PEI_PPI_DESCRIPTOR  **PpiList
  )
{
  *PpiListSize = sizeof(gPlatformPpiTable);
  *PpiList = gPlatformPpiTable;
}
