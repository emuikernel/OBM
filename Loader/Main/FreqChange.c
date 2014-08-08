/******************************************************************************
**	(C) Copyright 2007 Marvell International Ltd. ?
**	All Rights Reserved
******************************************************************************/
#include "FreqChange.h"


ProductPoint_T Sea_PP[PP_MAX_NUM]=
{

	{core_PLL1_416MHz, ACLK_PLL1_312MHz, DCLK_PLL1_416MHz, 0x0, 2, 2, 4, 3, 2, 0, VOLTAGE_1_15},	/*PP0: Reserved*/

	{core_PLL1_624MHz, ACLK_PLL1_624MHz, DCLK_PLL1_624MHz, 0x0, 2, 4, 4, 4, 2, 0, VOLTAGE_1_15},	/*PP1: Reserved*/
	
	{core_PLL2, ACLK_PLL1_624MHz, DCLK_PLL2, 400, 1, 2, 2, 4, 1, 0, VOLTAGE_1_15},	/*PP2: Reserved*/
	
	{core_PLL2, ACLK_PLL1_624MHz, DCLK_PLL1_416MHz, 1000, 2, 4, 4, 4, 1, 1, VOLTAGE_1_15},	/*PP3: Reserved*/

	{core_PLL2, ACLK_PLL1_624MHz, DCLK_PLL2, 1200, 2, 4, 6, 8, 3, 0, VOLTAGE_1_30}	/*PP4: */
};


void setup_pll2(UINT16 PLL2_freq)
{
	UINT32 REF_DIV, FB_DIV, KVCO, VCO_VRNG, KVCO_SEL_SE, tmp;
	
	if(PLL2_freq == 400)
	{
		FB_DIV = 0xb9;
		REF_DIV = 0x3;
		KVCO = 0x3;  /*0011: 1.53G~1.70G*/
		VCO_VRNG = 0x2;  /*KVCO-1*/
		KVCO_SEL_SE = 0x5;  /*VCODIV = 4*/
	}
	else if(PLL2_freq == 1000)
	{
		FB_DIV = 0xe7;
		REF_DIV = 0x3;
		KVCO = 0x5;  /*0011: 1.90G~2.10G*/
		VCO_VRNG = 0x4;  /*KVCO-1*/
		KVCO_SEL_SE = 0x2;  /*VCODIV = 2*/		
	}		
	else if(PLL2_freq == 1200)
	{
		FB_DIV = 0x115;
		REF_DIV = 0x3;
		KVCO = 0x7;  /*0011: 2.30G~2.40G*/
		VCO_VRNG = 0x6;  /*KVCO-1*/
		KVCO_SEL_SE = 0x2;  /*VCODIV = 2*/		
	}

	tmp  = BU_REG_READ(0xd4090104);
	tmp &= ~((0xF<<17) | (0x7<<14) | (0xF<<4));
	tmp |= (KVCO<<17) | (VCO_VRNG<<14) | (KVCO_SEL_SE<<4);
	BU_REG_WRITE(0xd4090104, tmp);
	
	BU_REG_WRITE(PMU_PLL2CR, BU_REG_READ(PMU_PLL2CR) & (~ 0xffff00) | (FB_DIV<<10) | (REF_DIV<<19));
	BU_REG_WRITE(PMU_PLL2CR, BU_REG_READ(PMU_PLL2CR) | 0x200);
	BU_REG_WRITE(PMU_PLL2CR, BU_REG_READ(PMU_PLL2CR) | 0x100);
}


UINT8 current_PP = 0x20;  //boot_PP

#if 0
void ModifyBuck1Voltage(PP_VOLTAGE_T pp_voltage)
{
	unsigned char regval;

	I2CEnableclockandPin();

	I2CConfigureDi();

	writei2c_addr_val(0x32,0x24,pp_voltage);

	regval = readi2c_addr_val(0x32,0x2);
	writei2c_addr_val(0x32,0x2,(regval | 0x4));
}
#endif

void Sea_freq_change(UINT8 ProductPoint)
{

	UINT32 tmp;

     serial_outstr("Sea_freq_change pp\n");
serial_outnum(ProductPoint);
serial_outstr("\n");
   
	BU_REG_WRITE(0xd4282888, BU_REG_READ(0xd4282888) 
									| (0x1 <<1) //| (0x1 <<0)
									| (0x1 <<4)	
									| (0x1 <<9)	
									| (0x1 <<10)	
									| (0x1 <<12)	
									| (0x3 <<17) // | (0x1 <<18)
									| (0x1 <<22)
									| (0x1 <<27)); // | (0x3 <<21));
	
	if(Sea_PP[ProductPoint].PLL2_value != 0)
		setup_pll2(Sea_PP[ProductPoint].PLL2_value);

	
	BU_REG_WRITE(PMU_FCCR_T, BU_REG_READ(PMU_FCCR_T) & (~(0x3<<26)) | ((Sea_PP[ProductPoint].core_PLL_use)<<26));

	tmp = Sea_PP[ProductPoint].aclk_PLL_use;
	BU_REG_WRITE(PMU_FCCR_T, BU_REG_READ(PMU_FCCR_T) & (~((0x1<<25)|(0x1<<19))) | ((tmp & 0x1)<<19) | ((tmp & 0x2)<<24));

	BU_REG_WRITE(PMU_FCCR_T, BU_REG_READ(PMU_FCCR_T) & (~(0x3<<23)) | ((Sea_PP[ProductPoint].dclk_PLL_use)<<23));
	
	//uart_printf("PMU_FCCR = 0x%lx\r\n", BU_REG_READ(PMU_FCCR_T));

	for(tmp=0; tmp<0xffff; tmp++) ;  //delay

	//  FC done by CP Intr mask
	BU_REG_WRITE(PMUA_CP_IMR, BU_REG_READ(PMUA_CP_IMR) | (0x1 << 2));	
	
	while (((BU_REG_READ(PMUA_DM_CC_CP)>>25) & 0x1) != 0) ;// wait until Monh FC is done

     //  Mohawk allow freq. change voting 
	 BU_REG_WRITE(PMUA_CC_AP, BU_REG_READ(PMUA_CC_AP) | (0x1 << 27));
	 
	 tmp = (Sea_PP[ProductPoint].PCLK_div - 1)
		 | ((Sea_PP[ProductPoint].PDCLK_div - 1)<<3)
		 | ((Sea_PP[ProductPoint].XPCLK_div- 1)<<9)
		 | ((Sea_PP[ProductPoint].DCLK_div- 1)<<12)
		 | ((Sea_PP[ProductPoint].ACLK_div- 1)<<15)
		 | (((Sea_PP[ProductPoint].DCLK_SYNC)<<18))
		 | (0x1<<19)
		 | (0xF<<20)  /*sync bits*/
		 | (0x7<<24)  /*request bits*/
		 | (0x1<<27)  /*vote bit*/
		 ;
		 
	//ustica_vbuck1_0_set(Sea_PP[ProductPoint].PP_Voltage);

	BU_REG_WRITE(PMUA_CC_CP, tmp);
 
	while (((BU_REG_READ(PMUA_CP_ISR)>>2) & 0x1) != 0x1) ;

	BU_REG_WRITE(PMUA_CP_ISR, BU_REG_READ(PMUA_CP_ISR) & ~(0x1<<2));

	BU_REG_WRITE(PMUA_CC_CP, BU_REG_READ(PMUA_CC_CP) & (~(0x7<<24)) | (0x1 << 31));

}

FC_RTN_CODE_T PP_Switch(UINT8 ProductPoint)
{
	if(ProductPoint == current_PP)
		return FC_OK;
	
	if(ProductPoint >= PP_MAX_NUM)
		return FC_PP_NOT_EXIST;

/****PLL2 can not switch to PLL2 directly, should switch to PP1 temp******/

	if((current_PP != 0x20) && (Sea_PP[current_PP].PLL2_value != 0)	&& (Sea_PP[ProductPoint].PLL2_value != 0))
		Sea_freq_change(1);
	
/*****END**********/

	Sea_freq_change(ProductPoint);

	current_PP = ProductPoint;

#if I2C
	if (ProductPoint == 4)
	{
		Voltage_set_main();
	}
#endif

	return FC_OK;

}


