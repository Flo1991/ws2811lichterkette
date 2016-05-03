/**************************************************************************//**
 * @file     LedEffects.h
 * @brief    file that contains different effect definitions for the lightstribe
 * @version  V1.00
 * @date     05.01.2016
 * @authors  Wank Florian
 *
 ******************************************************************************/
#include <stdint.h>

#ifndef LEDEFFECTS_H_
#define LEDEFFECTS_H_

//EFFECTS
/** \brief define for the setfullcolor effect, used for main switch*/
#define SETFULLCOLOR 0	
/** \brief define for the the fillup effect, used for main switch*/	
#define FILLUP 1
/** \brief define for the blink effect, used for main switch*/			
#define BLINK 2	
/** \brief define for the runled effect, used for main switch, refers to the runled init*/			
#define RUNLED 3	
/** \brief define for the alternating effect, used for main switch, refers to the alternate init*/		
#define ALTERNATE 5	
/** \brief define for the recolor effect, used for main switch*/		
#define RECOLOR 7	
/** \brief define for the fade effect, used for main switch*/		
#define FADEN 8		
/** \brief define for the initrainbow function, used for main switch*/		
#define INITRAINBOW 9	
/** \brief define for the the rotate function right, used for main switch*/	
#define ROTATE_R 10		
/** \brief define for the the rotate function left, used for main switch*/	
#define ROTATE_L 11	
/** \brief define for the custom effect, used for main switch, every LED is filled in a userdefined color (up to MAXNUMCOLORS, then reloop the colors)*/	
#define CUSTOM 12	
/** \brief define for the easteregg effect, used for main switch*/		
#define EASTEREGG 13		

uint8_t map(uint8_t x, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max);		//Map function for color conversion; calcualates a value in a new number range
struct color24bit colorconv8to24(uint8_t startcolor);											//Convert a 8 Bit color (RGB 3-3-2) to 24 Bit color (RGB 8-8-8); color assignment depends on the ledtype
void effectdelay(uint16_t delay);																//a simple variable delay function
void setfullcolor(struct color24bit color, uint8_t *lightdata);									//set the whole stribe in one color, call transmit2leds afterwards
void resetstribe(uint8_t *lightdata);															//set the whole stribe off, call transmit2leds afterwards
void rotate(uint8_t *lightdata, uint8_t direction);												//rotate the color array by one position
void rotateN(uint8_t *lightdata, uint8_t direction, uint8_t width);								//rotate the color array by n positions
void initrunled(struct color24bit color, uint8_t *lightdata, struct color24bit background);		//initialize the runled effect, call runrunled afterwards
void runrunled(uint8_t *lightdata, uint8_t direction);											//runs the runled effect, call initrunled before
void blinkled(struct color24bit color, uint8_t *lightdata);										//generate a blinking effect
void init_alternating(struct color24bit color, struct color24bit backcolor, uint8_t *lightdata);//initialize the alternating effect, call run_alternating afterwards
void run_alternating(uint8_t *lightdata );														//run the alternating effect, call init_alternating before
void recolor(struct color24bit color, uint8_t *lightdata);										//recolor the stribe step by step, stand alone function, ends after execution
void faden(struct color24bit color, uint8_t *lightdata);										//color fading effect, stand alone effect
void initrainbow(uint8_t *lightdata);															//init the stribe with rainbow colors, call transmit2leds afterwards
void eastereggbase(struct color24bit color, uint8_t *lightdata);								//part of the easteregg effect, do not call directly
void easteregg(uint8_t *lightdata);																//easteregg effect, try out and have fun :-)
void fillup(struct color24bit color,struct color24bit backcolor, uint8_t *lightdata);			//fill the stribe step by step until the stribe has one color, the background color is filled behind

#endif /* LEDEFFECTS_H_ */