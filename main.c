//***************************************************************************************
//  MSP430 OLED display time and battery voltage measurement
//
//
//  Z. Zilahi
//
//  May 2016
//  Built with Code Composer Studio v6
//***************************************************************************************

#include <msp430.h>
#include "OLED\Oled_SSD1306.h"
#include <inttypes.h>
#include <stdio.h>
#include "stdbool.h"

// defines
#define RED	BIT0
#define GREEN BIT6

// global variables
static volatile unsigned char buttonOn;
static volatile unsigned char buttonInvert;
static volatile unsigned char buttonFlip;
static volatile unsigned char buttonContrast = 0x01;

static volatile uint8_t seconds = 0, minutes = 0, hours = 0;
static volatile uint8_t ticks = 0;
static volatile uint8_t updateDisplay = 0;
static volatile uint32_t voltage=0;

bool ADCMeasProgress = false;

typedef enum  {
			  TIME_DISPLAYING, TIME_HOUR_EDITING, TIME_MINUTE_EDITING
} TState;
TState state = TIME_DISPLAYING;

// prototypes
void init_Clock(void);
void init_UCB0(void);
void init_Timer0_A0(void);
void init_GPIO(void);
uint16_t Msp430_GetSupplyVoltage(void);

// main function configures hardware and runs demo program
void main(void)
{
	char time[8+1];
	char volt[16+1];
	uint16_t voltage = 0;
	uint16_t ADCValue = 0;	// Measured ADC Value

	WDTCTL = WDTPW + WDTHOLD;	// hold watchdog

	SSD1306PinSetup();
	init_GPIO();
	init_Clock();
	init_UCB0();
	init_Timer0_A0();

	// clear any oscillator faults
    do {
    	IFG1 &= ~OFIFG;
    	__delay_cycles(100);
	} while (IFG1 & OFIFG);
	IFG1 &= ~(OFIFG + WDTIFG);
	IE1 |= WDTIE;

	__enable_interrupt();

	//set watchdog as debouncing interval
    WDTCTL = WDTPW + WDTTMSEL + WDTSSEL + WDTIS1;

    SSD1306Init();
    clearScreen();

    // loop in LPM3
    while (1)
    {
    	switch(state){
    	case TIME_DISPLAYING:
			if(updateDisplay){
				if(minutes<10){
					sprintf(time,"%d:0%d",hours,minutes);
				}
				else sprintf(time,"%d:%d",hours,minutes);
				if(seconds<10){
					sprintf(time,"%s:0%d",time, seconds);
				}
				else sprintf(time,"%s:%d",time, seconds);

				stringDraw(2, 50, "Hour");//4,40 is almost the centre of the screen
				stringDraw(4, 40, time);//4,40 is almost the centre of the screen
				stringDraw(6,20, volt);
				updateDisplay = 0;
			}

			if(state == TIME_DISPLAYING) {
				if(!ADCMeasProgress){
					ADCValue = Msp430_GetSupplyVoltage();
					voltage = ADCValue;
					sprintf(volt,"Battery: %d mV", voltage);
					if ((99 < voltage) && (voltage < 1000)){
						volt[12]=0x20;
						volt[13]=0x0;
					}
					else if ((9 < voltage) && (voltage < 100)) {
						volt[11]=0x20;
						volt[12]=0x20;
						volt[13]=0x0;
					}
					else if(voltage < 10){
						volt[10]=0x20;
						volt[11]=0x20;
						volt[12]=0x20;
						volt[13]=0x0;
					}
				}
			}

			break;
    	case TIME_HOUR_EDITING:
    		if(updateDisplay){
				sprintf(time,"%d:     ",hours);
				stringDraw(2, 40, "Edit hour");//4,40 is almost the centre of the screen
				stringDraw(4, 40, time);//4,40 is almost the centre of the screen
				updateDisplay = 0;
			}
			break;
    	case TIME_MINUTE_EDITING:
    		if(updateDisplay){
    			if(minutes<10){
    				sprintf(time,"%d:0%d  ",hours,minutes);
				}
				else sprintf(time,"%d:%d  ",hours,minutes);
				stringDraw(2, 37, "Edit minutes");
				stringDraw(4, 40, time);
				updateDisplay = 0;
			}
			break;

    	}//switch

    	LPM3;
    }
}

// set clock to max
void init_Clock(void)
{
	BCSCTL2 = SELM_0 + DIVM_0 + DIVS_0;

	if (CALBC1_1MHZ != 0xFF)
	{
		__delay_cycles(10000);
		DCOCTL = 0x00;
		BCSCTL1 = CALBC1_1MHZ;
		DCOCTL = CALDCO_1MHZ;
	}

	BCSCTL1 |= XT2OFF + DIVA_0;
	BCSCTL3 = XT2S_0 + LFXT1S_0 + XCAP_3;//was XCAP_1 before
}

// gpio init buttons and leds
void init_GPIO(void)
{
	P1SEL |= BIT1;					// ADC input pin P1.1

	P1OUT |= RED + GREEN;
	P1DIR |= RED + GREEN;

	// LP S2 button
	P1OUT |= BIT2 + BIT3 + BIT4;
	P1REN = BIT2 + BIT3 + BIT4;
	P1IES = BIT2 + BIT3 + BIT4;
	P1IFG = 0;
	P1IE = BIT2 + BIT3 + BIT4;

	// OLED booster buttons
	P2OUT |= BIT3 + BIT4;
	P2REN = BIT3 + BIT4;
	P2IES = BIT3 + BIT4;
	P2IFG = 0;
	P2IE = BIT3 + BIT4;
}

void init_UCB0(void)
{
	UCB0CTL1 |= UCSWRST;
	UCB0CTL0 = UCCKPH + UCMSB + UCMST + UCMODE_1 + UCSYNC;
	UCB0CTL1 = UCSSEL_2 + UCSWRST;
	UCB0BR0 = 32;
	UCB0CTL1 &= ~UCSWRST;

}

// timer0_a0 on 1s wakeup as activity indicator
void init_Timer0_A0(void)
{
    TA0CCTL0 = CM_0 + CCIS_0 + OUTMOD_4 + CCIE;
    TA0CCR0 = 32768;
    TA0CTL = TASSEL_1 + ID_0 + MC_1;
}

uint16_t Msp430_GetSupplyVoltage(void)
{
	uint16_t raw_value;
	// first attempt - measure Vcc/2 with 1.5V reference (Vcc < 3V )
	ADC10CTL0 &= ~ENC;
	//ADC10CTL0 |= SREF_1 + REFON + ADC10SHT_2 + ADC10SR + ADC10ON + ADC10IE;
	//ADC10CTL1 |= INCH_1 + SHS_0 + ADC10DIV_3 + ADC10SSEL_0 + CONSEQ1;
	//ADC10AE0 |= BIT1;


	ADC10CTL0 = SREF_1 + REFON + REF2_5V + ADC10ON + ADC10SHT_3 + ADC10IE;  // use internal ref, turn on 2.5V ref, set samp time = 64 cycles
	ADC10CTL1 = INCH_11;

	ADCMeasProgress = true;
	__delay_cycles(100);//wait it for REF settle
	// start conversion and wait for it
	ADC10CTL0 |= ENC + ADC10SC;
	LPM3;//enter low power mode

	raw_value = ADC10MEM;
	// check for overflow
	/*if (raw_value == 0x3ff) {
		// switch range - use 2.5V reference (Vcc >= 3V)
		ADC10CTL0 &= ~ENC;
		ADC10CTL0 |= REF2_5V + REFON + ADC10ON;
		ADC10AE0 |= BIT1;
		__delay_cycles(100);//wait it for REF settle
		// start conversion and wait for it
		ADC10CTL0 |= ENC + ADC10SC;
		LPM3;//enter low power mode

		raw_value = ADC10MEM;
		ADCMeasProgress = false;

		// convert value to mV
		return ((uint32_t)raw_value * 2500) / 1024;
	} else {*/
		ADCMeasProgress = false;
		// convert value to mV
		return ((uint32_t)raw_value * 5000) / 1024;
	//}
}

// toggle LEDs at 1s interval
#pragma vector=TIMER0_A0_VECTOR
__interrupt void timerA0_isr(void)
{
	P1OUT ^= RED + GREEN;
	++ticks;
	++seconds;
	if ( seconds == 60 ) {
		seconds = 0;
		++minutes;
		if ( minutes == 60 ) {
			minutes = 0;
			++hours;
			if ( hours == 24 ) {
				hours = 0;
				//seconds = 2; // correction for too slow quartz
				//++days;
			}
		}
	}
	updateDisplay = 1;
	LPM3_EXIT;
}

// S2 on Launchpad turns display on or off
#pragma vector=PORT1_VECTOR
__interrupt void port1_isr(void)
{
	if (P1IFG & BIT2)
	{
		P1IFG &= ~(BIT2 + BIT3 + BIT4);	    // clear P1.3 button flag
		P1IE &= ~(BIT2 + BIT3 + BIT4);			// clear P1.3 interrupt

		IFG1 &= ~WDTIFG;
		WDTCTL = (WDTCTL & 7) + WDTCNTCL + WDTPW + WDTTMSEL;
		IE1 |= WDTIE;

		if (state == TIME_DISPLAYING){
			state = TIME_HOUR_EDITING;
		}
		else if (state == TIME_HOUR_EDITING){
			state = TIME_MINUTE_EDITING;
		}
		else{
			stringDraw(2, 37, "            ");//4,40 is almost the centre of the screen
			state = TIME_DISPLAYING;
		}
	}

	if (P1IFG & BIT3)
    {
    	P1IFG &= ~(BIT2 + BIT3 + BIT4);	    // clear P1.3 button flag
    	P1IE &= ~(BIT2 + BIT3 + BIT4);			// clear P1.3 interrupt

    	IFG1 &= ~WDTIFG;
		WDTCTL = (WDTCTL & 7) + WDTCNTCL + WDTPW + WDTTMSEL;
		IE1 |= WDTIE;

		if (state == TIME_HOUR_EDITING)
		{
			hours = ++hours % 24;
		}
		else if (state == TIME_MINUTE_EDITING)
		{
			minutes = ++minutes % 60;
			seconds = 0;
		}
    }

	if (P1IFG & BIT4)
    {
    	P1IFG &= ~(BIT2 + BIT3 + BIT4);		// clear P1.4 button flag
    	P1IE &= ~(BIT2 + BIT3 + BIT4);			// clear P1.4 interrupt

    	IFG1 &= ~WDTIFG;
		WDTCTL = (WDTCTL & 7) + WDTCNTCL + WDTPW + WDTTMSEL;
		IE1 |= WDTIE;

    	// do something here
		if (buttonFlip == 1)
		{
			buttonFlip = 0;
			Set_FlipScreenVertically(0xF0);
		}
		else
		{
			buttonFlip = 1;
			Set_FlipScreenVertically(0xF8);
		}
    }
}

// left button inverts display; right button adjusts contrast
#pragma vector=PORT2_VECTOR
__interrupt void port2_isr(void)
{
	if (P2IFG & BIT3)
	{
		P2IFG &= ~(BIT3 + BIT4);			// clear P2 button flag
		P2IE &= ~(BIT3 + BIT4);			// clear P2 interrupt

		IFG1 &= ~WDTIFG;
		WDTCTL = (WDTCTL & 7) + WDTCNTCL + WDTPW + WDTTMSEL;
		IE1 |= WDTIE;

		// do something here
		if (buttonContrast < SSD1306_MAXCONTRAST)
		{
			buttonContrast += 25;
			Set_Contrast_Control(buttonContrast);
		}
		else
		{
			buttonContrast = 0x01;
			Set_Contrast_Control(buttonContrast);
		}
	}

	if (P2IFG & BIT4)
	{
		P2IFG &= ~(BIT3 + BIT4);			// clear P2 button flag
		P2IE &= ~(BIT3 + BIT4);			// clear P2 interrupt

		IFG1 &= ~WDTIFG;
		WDTCTL = (WDTCTL & 7) + WDTCNTCL + WDTPW + WDTTMSEL;
		IE1 |= WDTIE;

		// do something here
		if (buttonInvert == 1)
        {
        	buttonInvert = 0;
        	Set_Inverse_Display(0);
    	}
    	else
    	{
    		buttonInvert = 1;
    		Set_Inverse_Display(1);
    	}
	}
}

// wdt timer is used for debouncing P1.3 (S2 button) and the P2.3, P2.4 buttons
#pragma vector=WDT_VECTOR
__interrupt void watchdog_isr(void)
{
    IE1 &= ~WDTIE;

    P1IFG &= ~(BIT2 + BIT3 + BIT4);			// clear P1.3 button flag
    P1IE |=  BIT2 + BIT3 + BIT4;			// re-enable P1.3 interrupt

    P2IFG &= ~(BIT3 + BIT4);
    P2IE |= BIT3 + BIT4;
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
	// stop conversion, clear the interrupt flag and turn off ADC
	ADC10CTL0 &= ~(ENC | ADC10IFG | ADC10ON | REFON);
	//ADC10AE0 &= ~BIT1;

	LPM3_EXIT; // Return to active mode
}
