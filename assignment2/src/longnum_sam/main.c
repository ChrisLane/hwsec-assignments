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

#define INPUT_BLOCK_SIZE 16
#define INPUT_BLOCK_SIZE_LIMBS (INPUT_BLOCK_SIZE/BYTES_PER_LIMB)
#define INPUT_BLOCK_CNT (CRYPTO_IN_SIZE/INPUT_BLOCK_SIZE)

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
		__delay_cycles(80000);
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
	
	// Operand s
	uint8_t s[CRYPTO_IN_SIZE];
	ln_limb_t ln_s[CRYPTO_IN_SIZE_WORDS];
	
	// Operand n
	uint8_t n[CRYPTO_IN_SIZE];
	ln_limb_t ln_n[CRYPTO_IN_SIZE_WORDS];
	
	// Operand mu
	uint8_t mu[CRYPTO_IN_SIZE + INPUT_BLOCK_SIZE];
	ln_limb_t ln_mu[CRYPTO_IN_SIZE_WORDS + 1];
	
	// Operand e
	ln_limb_t ln_e;
	
	// Result r
	ln_limb_t ln_r[CRYPTO_IN_SIZE_WORDS];

	ln_clear(ln_r, CRYPTO_IN_SIZE_WORDS);
	
	uint8_t block_index_s = 0, block_index_n = 0, block_index_mu = 0;
	
	uint64_t begin = 0, end = 0, duration;
	uint8_t p = 0;
	
	while (1)
    {	
		while(rx_flag != 0)
		{
			uint8_t x = uart_getc();
			
			// Input number s (8 blocks)
			if(x == 's')
			{
				GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN6);
				
				// Get number block
				for(p = 0; p < INPUT_BLOCK_SIZE; p++)
				{
					s[p + INPUT_BLOCK_SIZE * block_index_s] = uart_getc();
				}
				
				// RX ok
				uart_putc(0xFF);
				uart_putc(block_index_s);
				
				// Next block, roll over at 16
				block_index_s = (block_index_s + 1) % INPUT_BLOCK_CNT;
			}
			// Input number n (8 blocks)
			else if(x == 'n')
			{
				GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN6);
				
				// Get number block
				for(p = 0; p < INPUT_BLOCK_SIZE; p++)
				{
					n[p + INPUT_BLOCK_SIZE * block_index_n] = uart_getc();
				}
				
				// RX ok
				uart_putc(0xFF);
				uart_putc(block_index_n);
				
				// Next block, roll over at 16
				block_index_n = (block_index_n + 1) % INPUT_BLOCK_CNT;
			}
			// Input number mu (9 blocks)
			else if(x == 'm')
			{
				GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN6);
				
				// Get number block
				for(p = 0; p < INPUT_BLOCK_SIZE; p++)
				{
					mu[p + INPUT_BLOCK_SIZE * block_index_mu] = uart_getc();
				}
				
				// RX ok
				uart_putc(0xFF);
				uart_putc(block_index_mu);
				
				// Next block, roll over at 17
				block_index_mu = (block_index_mu + 1) % (INPUT_BLOCK_CNT + 1);
			}
			// Do encryption
			else if(x == 'e')
			{
				GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN6);
				
				// Get exponent
				ln_e = uart_getc();
				ln_e |= (uart_getc() << 8);
				

				// Assign
				ln_from_bytes(s, CRYPTO_IN_SIZE, ln_s, CRYPTO_IN_SIZE_WORDS);
				ln_from_bytes(n, CRYPTO_IN_SIZE, ln_n, CRYPTO_IN_SIZE_WORDS);
				ln_from_bytes(mu, CRYPTO_IN_SIZE + INPUT_BLOCK_SIZE, ln_mu, CRYPTO_IN_SIZE_WORDS + 1);
				
				// Execute crypto code
				TRIGGER_ACTIVE();
				begin = cpucycles();
				crypto_func(ln_s, ln_n, ln_mu, ln_e, ln_r);
				end = cpucycles();
				TRIGGER_RELEASE();

				duration = end - begin; 
				
				// Reset block indices
				block_index_s = 0;
				block_index_n = 0;
				block_index_mu = 0;
				
				// Crypto ok
				uart_putc(0xFF);
				uart_putc(0xFF);
				
				for(p = 0; p < 8; p++)
				{
					uart_putc(duration & (uint64_t)0xff);
					duration >>= 8;
				}
				
				GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN6);
			}
			// Get output (stored in A)
			else if(x == 'o')
			{
				GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN6);
				
				for(p = 0; p < INPUT_BLOCK_SIZE_LIMBS; p++)
				{
					ln_limb_t t = ln_r[p + INPUT_BLOCK_SIZE_LIMBS * block_index_s];
					uart_putc(t & 0xff);
					uart_putc((t >> 8) & 0xff);
				}

				// Block ok
				uart_putc(0xFF);
				uart_putc(block_index_s);
				
				// Next block, roll over
				block_index_s = (block_index_s + 1) % (INPUT_BLOCK_CNT * 2);
			}
			
		}
	}
} 
