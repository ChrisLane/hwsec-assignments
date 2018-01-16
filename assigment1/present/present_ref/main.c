#include <msp430.h>
#include <string.h>
#include <driverlib.h>
#include <uart_fifo.h>
#include <timer_a.h>

// Include file for algorithm to test
#include "crypto.h"

#define interrupt(x) void __attribute__((interrupt (x)))

#define TRIGGER_ACTIVE() GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN3)
#define TRIGGER_RELEASE() GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN3)

/**
 9600 Baud 8/n/1
 Send: 'e' (0x65) + 10 byte key + 8 byte pt
 Receive: 8 byte encPresent(key, pt)
*/


/*
 * Timer code for measuring cycles
 */
static volatile uint64_t timer_ctr = 0;

volatile void __attribute__((__interrupt__(TIMER0_A1_VECTOR))) ctrinc(void)
{
	timer_ctr += (uint64_t)0x10000;
	//GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
	TA0IV = 0;
}

static void Init_TimerA()
{
	// // Clear timer
	// TA0CTL = TACLR;
	// 
	// // Select SMCLK, continuous mode, enable interrupt, pre-scale by 8
	// TA0CTL = TASSEL_2 | MC_2 | TAIE | ID_3;
	
	Timer_A_initContinuousModeParam param = {0};
	param.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    param.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_8;
    param.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    param. timerClear = TIMER_A_DO_CLEAR;
    param.startTimer = true;
	
	Timer_A_initContinuousMode(TIMER_A0_BASE, &param);
}

uint64_t cpucycles()
{
	return ((uint64_t)timer_ctr | (uint64_t)TA0R) << 3;
}
 
/*
 * GPIO Initialization
 */
static void Init_GPIO()
{
    // Set all GPIO pins to output low for low power
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_PJ, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7|GPIO_PIN8|GPIO_PIN9|GPIO_PIN10|GPIO_PIN11|GPIO_PIN12|GPIO_PIN13|GPIO_PIN14|GPIO_PIN15);
	
    // Set PJ.4 and PJ.5 as Primary Module Function Input, LFXT.
    GPIO_setAsPeripheralModuleFunctionInputPin
	(
	   GPIO_PORT_PJ,
	   GPIO_PIN4 + GPIO_PIN5,
	   GPIO_PRIMARY_MODULE_FUNCTION
    );
		   
	GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
	GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN3);
	GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN6);

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PMM_unlockLPM5();
}

/*
 * Clock System Initialization
 */
static void Init_Clock()
{
    // Set DCO frequency to 8 MHz
    CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_6);
	
    //Set external clock frequency to 32.768 KHz
    CS_setExternalClockSource(32768, 0);
	
    //Set ACLK=LFXT
    CS_initClockSignal(CS_ACLK, CS_LFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
	
    // Set SMCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
	
    // Set MCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
	
    //Start XT1 with no time out
    CS_turnOnLFXT(CS_LFXT_DRIVE_0);
}

int main(void)
{
	// Stop watchdog timer
	WDTCTL = WDTPW | WDTHOLD;               
	//WDT_A_hold(__MSP430_BASEADDRESS_WDT_A__); 
	
    Init_GPIO();
	
	GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
	
	TRIGGER_RELEASE();
	
	// Toggle LED1 and LED2 to indicate start
	int i;
	for (i = 0; i < 10; i++)
	{
		GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
		GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN6);
		__delay_cycles(200000);
	}
	
	Init_Clock();
	
	// 9600 Baud
	uart_init();
	
	Init_TimerA();
	
	// Enable global interrupts
    __enable_interrupt();
	
	// Flush
	while(rx_flag)
	{
		uart_getc();
	}
	
	uint8_t ct[CRYPTO_IN_SIZE];
	uint8_t key[CRYPTO_KEY_SIZE];
	uint8_t b = 0;
	uint64_t begin = 0, end = 0, duration;
	
	while (1)
    {	
		while(rx_flag != 0)
		{
			
			uint8_t x = uart_getc();
			
			if(x == 'e')
			{
				GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN6);
				
				// Get key
				for(b = 0; b < CRYPTO_KEY_SIZE; b++)
				{
					key[b] = uart_getc();
				}
				
				// Get input
				for(b = 0; b < CRYPTO_IN_SIZE; b++)
				{
					ct[b] = uart_getc();
				}
				
				// Execute crypto code
				TRIGGER_ACTIVE();
				begin = cpucycles();
				crypto_func(ct, key);
				end = cpucycles();
				TRIGGER_RELEASE();
				
				// Return output
				for(b = 0; b < CRYPTO_OUT_SIZE; b++)
				{
					uart_putc(ct[b]);
				}
				
				duration = end - begin; 
				
				for(b = 0; b < 8; b++)
				{
					uart_putc(duration & (uint64_t)0xff);
					duration >>= 8;
				}
				
				GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN6);
			}
			
		}
	}
} 
