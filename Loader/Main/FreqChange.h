#ifndef FREQCHANGE_H
#define FREQCHANGE_H

#include "Typedef.h"

/***************freq_change*/

#define PMUA_DM_CC_CP    0xd4282808
#define PMUA_CP_IMR      0xd4282870
#define PMUA_CC_CP       0xd4282800
#define PMUA_CC_AP       0xd4282804
#define PMUA_CP_ISR      0xd4282878
#define PMU_FCCR_T       0xd4050008

#define PMUA_AP_ISR      0xd42828a0

#define PMU_PLL2CR       0xd4050034


#define PP_MAX_NUM 5

typedef enum
{
	core_PLL1_416MHz = 0x0,
	core_PLL1_624MHz = 0x1,
	core_PLL2 = 0x2,
	core_PLL1_312MHz = 0x3
} CORE_PLL_FREQ_T;


typedef enum
{
	ACLK_PLL1_312MHz = 0x0,
	ACLK_PLL1_624MHz = 0x1,
	ACLK_PLL2 = 0x2,
	ACLK_PLL1_416MHz = 0x3
} ACLK_PLL_FREQ_T;



typedef enum
{
	DCLK_PLL1_416MHz = 0x0,
	DCLK_PLL1_624MHz = 0x1,
	DCLK_PLL2 = 0x2,
	DCLK_PLL1_312MHz = 0x3
} DCLK_PLL_FREQ_T;


typedef enum
{
	VOLTAGE_0_90 = 0x18,
	VOLTAGE_1_00 = 0x20,
	VOLTAGE_1_10 = 0x28,
	VOLTAGE_1_15 = 0x2C,
	VOLTAGE_1_20 = 0x30,
	VOLTAGE_1_30 = 0x38
} PP_VOLTAGE_T;

typedef struct
{
	CORE_PLL_FREQ_T core_PLL_use;
	ACLK_PLL_FREQ_T aclk_PLL_use;
	DCLK_PLL_FREQ_T dclk_PLL_use;
	UINT16 PLL2_value;
	UINT8  PCLK_div;
	UINT8  XPCLK_div;
	UINT8  PDCLK_div;
	UINT8  ACLK_div;
	UINT8  DCLK_div;
	UINT8  DCLK_SYNC;
	PP_VOLTAGE_T PP_Voltage;
} ProductPoint_T;

typedef enum
{
	FC_OK,
	FC_PP_NOT_EXIST,
	FC_ERROR,
	FC_NOT_SUPPORT
} FC_RTN_CODE_T;

FC_RTN_CODE_T PP_Switch(UINT8 ProductPoint);
void Sea_freq_change(UINT8 ProductPoint);
#endif