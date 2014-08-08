#ifndef _BU_INTERRUPT_H_
#define _BU_INTERRUPT_H_

#define	CORE_AP		0x0
#define	CORE_CP		0x1

#define IPC_PRIORITY 	15
#define	DMA_PRIORITY	15
#define	BROM_PRIORITY	15

// definition for bit 4 of ICU_INT_0_63_CONF
#define	FIR_INT		0x0
#define	IRQ_INT		0x1

#define	INT_NUMS	64

#define	ICU_AP_GBL_IRQ_MSK_FIQ_MSK	0x1
#define	ICU_CP_GBL_IRQ_MSK_FIQ_MSK	0x1


/* interrupt mask */
#define		INT_IRQ				0		
#define		INT_FIQ				1		
#define		INT_SSP_HiFi		2			//ssp1_int_req	
#define		INT_SSP2			3			//ssp_int_req	
#define		INT_PMIC			4			//PAD -- m1_PMIC_INT
#define		INT_RTC				5			//rtc_hzclk_int_ndr	
#define		INT_RTC_ALARM		6			//	rtc_slp_alarm_ndr	
#define		INT_I2C_AP			7			//i2c0_int	
#define		INT_I2C_CP			8			//COMM -- i2c0_int	
#define		INT_KeyPad			9			//kp_int	
#define		INT_Rotarey			10		
#define		INT_Tackball		11		
#define		INT_KeyPress		12		
#define		INT_AP_TMR1			13			//timer_1_irq	
#define		INT_AP_TMR2			14			//timer_2_irq
#define		INT_AP_TMR3			15			//timer_3_irq
#define		INT_IPC_AP_DATAACK	16			//ipc_946_926_int0
#define		INT_IPC_AP_SET_CMD	17			//ipc_946_926_int1
#define		INT_IPC_AP_SET_MSG	18			//ipc_946_926_int2
#define		INT_IPC_AP_SET_GP	19			//ipc_946_926_int3
#define		INT_IPC_AP_WAKEUP	20			//ipc2pmu_946_wakeup
#define		INT_IPC_CP_DATAACK	21			//ipc_926_946_int0
#define		INT_IPC_CP_SET_CMD	22			//ipc_926_946_int1
#define		INT_IPC_CP_SET_MSG	23			//ipc_926_946_int2
#define		INT_IPC_CP_SET_GP	24			//ipc_926_946_int3
#define		INT_IPC_CP_WAKEUP	25			//ipc2pmu_926_wakeup
#define		INT_UART1_Slow		26			// COMM 
#define		INT_UART2_Fast		27			//ffuart1_int
#define		INT_UART3_Fast		28			//ffuart2_int
#define		INT_AP2_TMR1		29			//timer1_1_irq
#define		INT_AP2_TMR2		30			//timer1_2_irq
#define		INT_CP_TMR1			31			//timer_1_irq		
#define		INT_CP_TMR2			32			//timer_2_irq		
#define		INT_CP_TMR3			33			//timer_3_irq		
#define		INT_GSSP 			34			//COMM -- (PCM on MSA)				
#define		INT_WDT				35			//wdt_irq		
#define		INT_Main_PMU		36			//PMU
#define		INT_SG_FRQ_CHG		37			//PMU
#define		INT_MK_FRQ_CHG		38			//PMU
#define		INT_MMC				39			//sdh2icu_int	only 1 int from SD (2)	
#define		INT_AEU				40			//aeu_int		
#define		INT_LCD_INTF		41			//lcd_irq		
#define		INT_CI_INTF			42			//ipe_irq		
#define		INT_IRE				43			//Rotation Enging ire_irq		
#define		INT_USB				44			//usb_int		
#define		INT_NAND			45			//nd2icu_int		
#define		INT_HiFi_DMA		46			//squ_int 	net name dc_int
#define		INT_DMA_CP			47			//dma_int0	
#define		INT_DMA_AP			48			//dma_int1	
#define		INT_GPIO			49			//gpio_int	
#define		INT_AP2_TMR3		50
#define		INT_RESERVED		51			//reserved

#define		INT_IPC_SRV0_SG		52		
#define		INT_IPC_SRV1_SG		53		
#define		INT_IPC_SRV2_SG		54		
#define		INT_IPC_SRV3_SG		55		
#define		INT_IPC_SRV0_MK		56		
#define		INT_IPC_SRV1_MK		57		
#define		INT_IPC_SRV2_MK		58		
#define		INT_IPC_SRV3_MK		59		
#define		INT_AP_PMU			60			//pmu_int	
#define		INT_Fab0_TO			61			//fabric0_timeout	
#define		INT_Fab1_TO			62			//fabric1_timeout
#define		INT_SM				63			//From Pin Mux (ND_RDY line)

#define		INT_MIPI_HSI		41

// DCB Interrupt translation table for TTC
//----------------------------------------
#define USB0_OTG_INT (INT_USB)
#define FFUART_INT (INT_UART2_Fast)
#define USB_CLIENT_INT (INT_USB)
#define U2D_CLIENT_INT (INT_USB)
#define DMA_CNTL_INT (INT_DMA_AP)

//Prototypes
unsigned char GetPortInterruptFlag(void);
void ClearPortInterruptFlag(void);
unsigned int EnablePeripheralIRQInterrupt(unsigned int InterruptID);
unsigned int DisablePeripheralIRQInterrupt(unsigned int InterruptID);
void INT_init(void);
void IRQ_Glb_Ena(void);
unsigned int GetIrqStatus(void);
#endif /* _BU_INTERRUPT_H_ */
