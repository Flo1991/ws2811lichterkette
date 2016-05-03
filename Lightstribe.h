/**************************************************************************//**
 * @file     Lightstribe.h
 * @brief    basic functions for controlling WS2811/WS2812 LEDs
 * @version  V1.00
 * @date     05.01.2016
 * @authors  Wank Florian
 *
 ******************************************************************************/
#include <stdint.h>
#include <avr/io.h>

#ifndef LIGHTSTRIBE_H_
#define LIGHTSTRIBE_H_

/** \brief 24 Bit color structure RGB 8-8-8*/
struct color24bit{
	uint8_t red;									/**<8 Bit red*/
	uint8_t green;									/**<8 Bit green*/
	uint8_t blue;									/**<8 Bit blue*/
};

#if F_CPU == 16000000
#pragma message("Use 16 MHz Macros: ")
/** \brief macro to transmit a one wire High Bit at PB0; all Pins of PORTB are set/reset*/
#define SETHIGH PORTB=0x01;\
			   asm ("nop");\
			   asm ("nop");\
			   asm ("nop");\
			   asm ("nop");\
			   asm ("nop");\
			   asm ("nop");\
			   asm ("nop");\
			   asm ("nop");\
			   asm ("nop");\
			   asm ("nop");\
			   asm ("nop");\
			   PORTB=0x00;\
			   asm ("nop");\
			   asm ("nop");\
			   asm ("nop");
			   
#elif F_CPU == 8000000
#pragma message("Use 8 MHz Macros: ")
#define SETHIGH PORTB=0x01;\
			   asm ("nop");\
			   asm ("nop");\
			   asm ("nop");\
			   asm ("nop");\
			   asm ("nop");\
			   PORTB=0x00;\
			   asm ("nop");\
			   asm ("nop");
#endif



#if F_CPU == 16000000
/** \brief macro to transmit a one wire Low Bit at PB0; all Pins of PORTB are set/reset*/
#define SETLOW PORTB=0x01;\
			  asm ("nop");\
			  asm ("nop");\
			  asm ("nop");\
			  asm ("nop");\
			  asm ("nop");\
			  PORTB=0x00;\
			  asm ("nop");\
			  asm ("nop");\
			  asm ("nop");\
			  asm ("nop");\
			  asm ("nop");\
			  asm ("nop");\
			  asm ("nop");\
			  asm ("nop");\
			  asm ("nop");
#elif F_CPU == 8000000
#define SETLOW PORTB=0x01;\
			   asm ("nop");\
			   asm ("nop");\
			   PORTB=0x00;\
			   asm ("nop");\
			   asm ("nop");\
			   asm ("nop");
#endif


//function to change one LED at a specific position; all other LEDs are not changed; run transmit2leds afterwards
void changeled(struct color24bit color, uint8_t *lightdata, uint8_t lednr);
//function to set one LED at a specific position; all other LEDs are turned off; run transmit2leds afterwards
void setled(struct color24bit color, uint8_t *lightdata, uint8_t lednr);
//transmit the color array to the stribe --> one wire data transmission
void transmit2leds(uint8_t lightdata[]);

#endif /* LIGHTSTRIBE_H_ */