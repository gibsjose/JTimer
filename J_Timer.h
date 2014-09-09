/*
 * Copyright (c) 2013 All Right Reserved
 *
 * J_Timer.h
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


#ifndef J_TIMER_H_
#define J_TIMER_H_

#ifndef F_CPU
#define F_CPU 16000000L
#endif

/*** DEFINITIONS ***/
//Define Timers
#define TIMER_0 0x01
#define TIMER_1 0x02
#define TIMER_2 0x03

//Define Modes
#define DELAY_MODE 0x01
#define CTC_MODE 0x02
#define PHASE_CORRECT_PWM_MODE 0x03
#define PHASE_CORRECT_PWM_DC_MODE 0x04

//Define Max Delay Times
#define TIMER_0_MAX_DELAY_TIME_PS_1		(1000.0 / F_CPU) * 0xFF
#define TIMER_0_MAX_DELAY_TIME_PS_8		(1000.0 / F_CPU) * 0xFF * 8
#define TIMER_0_MAX_DELAY_TIME_PS_64	(1000.0 / F_CPU) * 0xFF * 64
#define TIMER_0_MAX_DELAY_TIME_PS_256	(1000.0 / F_CPU) * 0xFF * 256
#define TIMER_0_MAX_DELAY_TIME_PS_1024	(1000.0 / F_CPU) * 0xFF * 1024

#define TIMER_1_MAX_DELAY_TIME_PS_1		(1000.0 / F_CPU) * 0xFFFF
#define TIMER_1_MAX_DELAY_TIME_PS_8		(1000.0 / F_CPU) * 0xFFFF * 8
#define TIMER_1_MAX_DELAY_TIME_PS_64	(1000.0 / F_CPU) * 0xFFFF * 64
#define TIMER_1_MAX_DELAY_TIME_PS_256	(1000.0 / F_CPU) * 0xFFFF * 256
#define TIMER_1_MAX_DELAY_TIME_PS_1024	(1000.0 / F_CPU) * 0xFFFF * 1024

#define TIMER_2_MAX_DELAY_TIME_PS_1		(1000.0 / F_CPU) * 0xFF
#define TIMER_2_MAX_DELAY_TIME_PS_8		(1000.0 / F_CPU) * 0xFF * 8
#define TIMER_2_MAX_DELAY_TIME_PS_64	(1000.0 / F_CPU) * 0xFF * 64
#define TIMER_2_MAX_DELAY_TIME_PS_256	(1000.0 / F_CPU) * 0xFF * 256
#define TIMER_2_MAX_DELAY_TIME_PS_1024	(1000.0 / F_CPU) * 0xFF * 1024

//Define Errors
#define ERR_INVALID_MODE		-100
#define ERR_INVALID_TIMER_RANGE	-101
#define ERR_INVALID_DELAY_TIME	-102
#define ERR_INVALID_TIMER		-103

/*** GLOBAL FUNCTION PROTOTYPES ***/
int ConfigureTimer(int numArgs, ...);
int ResetTimer(int timer);
char * ErrorMessage(int error);

#endif /* J_TIMER_H_ */