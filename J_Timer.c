/*
 * Copyright (c) 2013 All Right Reserved
 *
 * J_Timer.c
 *
 * Created: 10/11/2013
 *
 *  Author: Joe Gibson
 *	
 *	Description:
 *	This library implements simple functions for configuring
 *	timers on the ATmega platform, and the ATmega328P, specifically.
 *	
 *	This library allows users to easily configure Timer 0-2 for
 *	a number of common configurations, and performs many of the
 *	calculations and error checking that would normally need to be
 *	performed when configuring timers for PWM, CTC, and other modes.
 *
 */ 

#define F_CPU 16000000L

#include <avr/io.h> //AVR Register and Pin Definitions
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdarg.h>	//Variable Argument Lists
#include <string.h>
#include "J_Timer.h"
#include "usart.h"

/*
	Write Timer Configuration Library
	
	typedef struct TIMER_PARAM_T
	{
		Resolution_T Resolution; //8-Bit or 16-Bit (enum)
		TimerMode_T Mode; //Modes (enum)
		unsigned char Enable;
		
		
	}TIMER_PARAM_T;
	
	//Create Timed Routines List
	int createTimedRoutines(int timer, int routinesList_ms[], ...)
	
	Possibly pass in TimerStructure to all these. Have default for ATmega328P (Timer 0, 1, 2) with correct values
	but also allow user to create timer structure to be passed and checked against for all routines
	
	int ConfigureTimer(int numArgs, int timer, int mode, int param1, int param2, int param3, int param4...)
	
	Where timer is TIMER_0, TIMER_1, or TIMER_2
	Where mode is DELAY_MODE, CTC_MODE, PHASE_CORRECT_PWM_MODE, NORMAL_MODE, etc.
	
	//Remember to put in header file
	#ifndef F_CPU
	#define F_CPU 16000000L
	#endif
	
	DELAY_MODE:
		Use timer as a delay, using interrupts and CTC mode with OCRXA/B
		param1: delayTime - Time to delay in milliseconds
		param2: topSelect - Max counter select (OCRXA/OCRXB/0xFF/0xFFF/etc...)
		param3: interruptEn - Use/don't use interrupts
	
	PHASE_CORRECT_PWM_MODE:
		param1: period - Period in ms
		param2: onTime - On Time in ms
		param3: pin - Pin to use (OCXA/OCXB) - Check against Timer Number and availability
		
	PHASE_CORRECT_PWM_DC_MODE:
		param2: period - Period in ms
		param2: dutyCycle - Duty cycle 0 <= DC <= 1
		param3: pin - Pin to use (OCXA/OCXB) - Check against Timer Number and availability
	
	CTC_MODE:
		param1: top - Top count value - Check against timer res (8-bit/16-bit)
		
	Returns:
		0:	SUCCESS
		-1: ERR_TIMER_RANGE: Not enough range on timer
		-2: ERR_NAME Etc...
	*/

#define ERR_CHECK(err) {if(err){USART_Send_string(ErrorMessage(err));}}

int main(void)
{	
	//Error code
	int err = 0;
	
	//LED Pin
	DDRB |= _BV(5);
	
	//Initialize USART for debug
	USART_Init(MYUBRR);
	
	//Configure Timer 1 for a 1 second delay
	err = ConfigureTimer(3, TIMER_1, DELAY_MODE, 1000);
	
	//Check for error (see #define above main)
	ERR_CHECK(err)
	
	//Enable All Interrupts
	sei();
	
	while(1)
	{
		_delay_ms(20);
	}
	
	return 0;
}

ISR(TIMER1_COMPA_vect)
{
	//Toggle LED
	PORTB ^= _BV(5);
}

int ConfigureTimer(int numArgs, ...)
{
	//Define definite variables
	int timer = 0;
	int mode = 0;
	
	//Define plausible variables
	//int period = 0;
	//int dutyCycle = 0;
	//int onTime = 0;
	int delayTime = 0;
	
	//Define variable argument list pointer
	va_list ap;

	//Initialize the argument list
	va_start(ap, numArgs);
	
	USART_Send_string("va_start called\n");
	
	//Determine Timer
	timer = va_arg(ap, int);
	
	USART_Send_int(timer);
	USART_Send_string(" timer number obtained\n");
	
	//Determine Timer Mode
	mode = va_arg(ap, int);
	
	USART_Send_int(mode);
	USART_Send_string(" mode obtained\n");
	
	switch(mode)
	{
		case DELAY_MODE :
			
			USART_Send_string("DELAY_MODE\n");
			
			//Obtain delay time
			delayTime = va_arg(ap, int);
			
			USART_Send_int(delayTime);
			USART_Send_string(" delay time obtained\n");
			
			//Ignore negative or zero delay times
			if(delayTime <= 0)
			{
				USART_Send_string("INVALID DELAY TIME\n");
				
				return ERR_INVALID_DELAY_TIME;
			}
			
			//Timer 0
			if(timer == TIMER_0)
			{
				//Reset Timer
				ResetTimer(TIMER_0);
				
				//Configure definite flags
				TCCR0A = _BV(WGM01); //CTC Mode
				TIMSK0 = _BV(OCIE0A); //Compare Match Interrupt Enable
				
				//Determine the smallest prescaler to achieve delay time
				if(delayTime <= TIMER_0_MAX_DELAY_TIME_PS_1)
				{	
					//Cannot achieve 1ms
					ResetTimer(TIMER_0);
					
					return ERR_INVALID_DELAY_TIME;
				}
				else if(delayTime <= TIMER_0_MAX_DELAY_TIME_PS_8)
				{
					//Cannot achieve 1ms
					ResetTimer(TIMER_0);
					
					return ERR_INVALID_DELAY_TIME;
				}
				
				//Lowest Resolution to achieve 1ms for Timer 0/2
				else if(delayTime <= TIMER_0_MAX_DELAY_TIME_PS_64)
				{
					TCCR0B |= _BV(CS00) | _BV(CS01);
					
					OCR0A = (int)(((double)delayTime / ((1000.0 / F_CPU) * 64 * 0xFF)) * 0xFF);
				}
				else if(delayTime <= TIMER_0_MAX_DELAY_TIME_PS_256)
				{
					TCCR0B |= _BV(CS02);
					
					OCR0A = (int)(((double)delayTime / ((1000.0 / F_CPU) * 256 * 0xFF)) * 0xFF);
				}
				else if(delayTime <= TIMER_0_MAX_DELAY_TIME_PS_1024)
				{
					TCCR0B |= _BV(CS00) | _BV(CS02);
					
					OCR0A = (int)(((double)delayTime / ((1000.0 / F_CPU) * 1024 * 0xFF)) * 0xFF);
				}
				
				//Delay time is above the max range for Timer 0
				else
				{
					ResetTimer(TIMER_0);
					
					return ERR_INVALID_DELAY_TIME;
				}
			}
			
			//Timer 1
			else if(timer == TIMER_1)
			{
				USART_Send_string("Timer 1 selected\n");
				
				//Reset Timer
				ResetTimer(TIMER_1);
				
				USART_Send_string("Timer 1 reset\n");
				
				//Configure definite flags
				TCCR1A = 0;
				TCCR1B = _BV(WGM12); //CTC Mode
				TIMSK1 = _BV(OCIE1A); //Compare Match Interrupt Enable
				
				//Determine the smallest prescaler to achieve delay time
				if(delayTime <= TIMER_1_MAX_DELAY_TIME_PS_1)
				{
					TCCR1B |= _BV(CS10);
					USART_Send_string("1 Prescaler\n");
					OCR1A = (int)(((double)delayTime / ((1000.0 / F_CPU) * 1 * 0xFFFF)) * 0xFFFF);
				}
				else if(delayTime <= TIMER_1_MAX_DELAY_TIME_PS_8)
				{
					TCCR1B |= _BV(CS01);
					USART_Send_string("8 Prescaler\n");
					OCR1A = (int)(((double)delayTime / ((1000.0 / F_CPU) * 8 * 0xFFFF)) * 0xFFFF);
				}
				else if(delayTime <= TIMER_1_MAX_DELAY_TIME_PS_64)
				{
					TCCR1B |= _BV(CS10) | _BV(CS11);
					USART_Send_string("64 Prescaler\n");
					OCR1A = (int)(((double)delayTime / ((1000.0 / F_CPU) * 64 * 0xFFFF)) * 0xFFFF);
				}
				else if(delayTime <= TIMER_1_MAX_DELAY_TIME_PS_256)
				{
					TCCR1B |= _BV(CS12);
					USART_Send_string("256 Prescaler\n");
					OCR1A = (int)(((double)delayTime / ((1000.0 / F_CPU) * 256 * 0xFFFF)) * 0xFFFF);
				}
				else if(delayTime <= TIMER_1_MAX_DELAY_TIME_PS_1024)
				{
					TCCR1B |= _BV(CS10) | _BV(CS12);
					USART_Send_string("1024 Prescaler\n");
					OCR1A = (int)(((double)delayTime / ((1000.0 / F_CPU) * 1024 * 0xFFFF)) * 0xFFFF);
				}
				
				//Delay time is above the max range for Timer 1
				else
				{
					ResetTimer(TIMER_1);
					USART_Send_string("INVALID DELAY TIME\n");
					return ERR_INVALID_DELAY_TIME;
				}
				
				USART_Send_int((((TCCR1B & _BV(CS12)) == _BV(CS12)) ? 1 : 0));
				USART_Send_string(" CS12\n");
				USART_Send_int((((TCCR1B & _BV(CS11)) == _BV(CS11)) ? 1 : 0));
				USART_Send_string(" CS11\n");
				USART_Send_int((((TCCR1B & _BV(CS10)) == _BV(CS10)) ? 1 : 0));
				USART_Send_string(" CS10\n");
				
				USART_Send_int(OCR1A);
				USART_Send_string(" OCR1A\n");
			}
			
			//Timer 2
			else if(timer == TIMER_2)
			{
				//Reset Timer
				ResetTimer(TIMER_2);
				
				//Configure definite flags
				TCCR2A = _BV(WGM21); //CTC Mode
				TIMSK2 = _BV(OCIE2A); //Compare Match Interrupt Enable
				
				//Determine the smallest prescaler to achieve delay time
				if(delayTime <= TIMER_2_MAX_DELAY_TIME_PS_1)
				{
					//Cannot achieve 1ms
					ResetTimer(TIMER_2);
					
					return ERR_INVALID_DELAY_TIME;
				}
				else if(delayTime <= TIMER_2_MAX_DELAY_TIME_PS_8)
				{
					//Cannot achieve 1ms
					ResetTimer(TIMER_2);
					
					return ERR_INVALID_DELAY_TIME;
				}
				
				//Lowest Resolution to achieve 1ms for Timer 0/2
				else if(delayTime <= TIMER_2_MAX_DELAY_TIME_PS_64)
				{
					TCCR2B |= _BV(CS20) | _BV(CS21);
					
					OCR2A = (int)(((double)delayTime / ((1000.0 / F_CPU) * 64 * 0xFF)) * 0xFF);
				}
				else if(delayTime <= TIMER_2_MAX_DELAY_TIME_PS_256)
				{
					TCCR2B |= _BV(CS22);
					
					OCR2A = (int)(((double)delayTime / ((1000.0 / F_CPU) * 256 * 0xFF)) * 0xFF);
				}
				else if(delayTime <= TIMER_2_MAX_DELAY_TIME_PS_1024)
				{
					TCCR2B |= _BV(CS20) | _BV(CS22);
					
					OCR2A = (int)(((double)delayTime / ((1000.0 / F_CPU) * 1024 * 0xFF)) * 0xFF);
				}
				
				//Delay time is above the max range for Timer 0
				else
				{
					ResetTimer(TIMER_2);
					
					return ERR_INVALID_DELAY_TIME;
				}
			}
			
			//Invalid Timer
			else
			{
				return ERR_INVALID_TIMER;	
			}
			
			break;
		
		case PHASE_CORRECT_PWM_MODE :
			break;
		
		default :
			return ERR_INVALID_MODE;
			break;
	}

	va_end(ap);
	
	//Success
	return 0;
}

int ResetTimer(int timer)
{
	switch(timer)
	{
		case TIMER_0 :
			TCCR0A = 0;
			TCCR0B = 0;
			TIMSK0 = 0;
			break;
		
		case TIMER_1 :
			TCCR1A = 0;
			TCCR1B = 0;
			TIMSK1 = 0;
			break;
			
		case TIMER_2 :
			TCCR2A = 0;
			TCCR2B = 0;
			TIMSK2 = 0;
			break;
		
		default :
			return ERR_INVALID_TIMER;
			break;
	}
	
	return 0;
}

char * ErrorMessage(int err)
{
	static char str[100];
	
	if(err == ERR_INVALID_MODE)
	{
		sprintf(str, "(%d) Invalid mode selected\n", err);
	}
	else if(err == ERR_INVALID_TIMER_RANGE)
	{
		sprintf(str, "(%d) Timer range invalid\n", err);
	}
	else if(err == ERR_INVALID_DELAY_TIME)
	{
		sprintf(str, "(%d) Delay time negative or too long for specified timer\n", err);
	}
	else
	{
		sprintf(str, "(%d) Unknown Error", err);
	}
	
	return str;
}