/**************************************************************************//**
 * @file     LedEffects.c
 * @brief    effect functions for controlling WS2811/WS2812 LEDs
 * @details  This file contains different effect functions to control WS2811/WS2812 LEDs using 
			 an AVR. It also contains a conversion function to convert 8 Bit color values (RGB 3-3-2)
			 to 24 Bit color values (RGB/GRB 8-8-8). The effects control first the lightdata array
			 and then transmit the array data to the stribe. Using different operations result in different
			 effects. You can add different functions if you like to. But remember that all operations
			 need to be done on the lightdata array that needs to be transmitted at one block to the 
			 LEDs after your array has been changed.
 * @version  V1.00
 * @date     05.01.2016
 * @authors  Wank Florian
 *
 ******************************************************************************/
#include "globals.h"
#include "Lightstribe.h"
#include "LedEffects.h"
#include <util/delay.h>

/** \brief  Arduino map function; used for color conversion
    \param [in]  	uint8_t x: value to map
	\param [in]  	uint8_t in_min : minimum value input reference
	\param [in]  	uint8_t in_max : maximum value input reference
	\param [in]  	uint8_t out_min : minimum value output reference
	\param [in]  	uint8_t out_max : maximum value output reference
    \return 		uint8_t : mapped value referring to the input
    \note     		This function is used for color conversion from 8 Bit to 24 Bit colors;
					How it works:
					in_min < x < in_max convert to out_min < returnvalue < out_max
					by positioning the x proportionally in the new number range
 */
uint8_t map(uint8_t x, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/** \brief  color conversion function; converts a 8 Bit color (RGB 3-3-2) to a 24 Bit color (RGB 8-8-8)
    \param [in]  	uint8_t startcolor: 8 Bit color to convert
    \return 		struct color24bit : 24 Bit color result
    \note     		This function converts the 8 Bit color to a 24 Bit color depending on the ledtype.
					This is neccessary because of differnt color formats (WS2811->RGB ; WS2812->GRB).
					Original the whole environment was for WS2812 LEDs!				
 */
struct color24bit colorconv8to24(uint8_t startcolor)
{
	struct color24bit color;
	if (ledtype==11)
	{	//color conversion for WS2811 LEDs (RGB color)
		//the converted values are assigned to the colors of the struct, red an green are switched
		//because of the different color profiles
		color.blue =map((0b00000011 & startcolor),0,3,0,255);	//2 Bit blue converted to 8 bit
		color.red=map((0b00011100 & startcolor)>>2,0,7,0,255);	//3 Bit green converted to 8 bit, assigned to red (color profiles!)
		color.green=map((0b11100000 & startcolor)>>5,0,7,0,255);//3 Bit red converted to 8 bit, assigned to green (color profiles!)	
	}
	else
	{	//color conversion for WS2812 LEDs (GRB color)
		//the converted values are assigned to the colors of the struct
		//no color switching is done, the environment is for WS2812 LEDs (GRB)
		color.blue =map((0b00000011 & startcolor),0,3,0,255);	//2 Bit blue
		color.green=map((0b00011100 & startcolor)>>2,0,7,0,255);//3 Bit green
		color.red=map((0b11100000 & startcolor)>>5,0,7,0,255);	//3 Bit red
	}
	return color;
}

/** \brief  simple delay function; no concrete delay time
    \param [in]  	uint16_t delay : delay value
    \return 		void
    \note     		This function is just a variable delay, there is no coherence with a concrete time (i.e. s, ms)			
 */
void effectdelay(uint16_t delay)
{
	uint16_t j;
	if (delay==0)
		return;
	do
	{
		j=2000;
		if (PacketComplete==1)	//interrupt the function if new settings have been received
			break;
		do
		{
			asm ("nop");
		} while (--j);
	} while (--delay);
	
}

/** \brief  Set all LEDs to the chosen color; run transmit2leds afterwards to update the LEDs
    \param [in]  	struct color24bit color : color to set
	\param [in]  	uint8_t *lightdata : lightdata array that holds the color values for the stribe
    \return 		void
    \note     		This function sets the lightdata array. To update the stribe run transmit2leds afterwards! 		
 */
void setfullcolor(struct color24bit color, uint8_t *lightdata)
{
	uint8_t ledcolor;
	uint16_t i;
	for (i=0;i<NumOfLeds*3;i++)	//Loop over color array (lightdata)
	{
		ledcolor = i%3;
		//set the array elements
		if (ledcolor==0)
			*lightdata++=color.green;	
		else if(ledcolor==1)
			*lightdata++=color.red;		
		else
			*lightdata++=color.blue;
	}
}

/** \brief  Set all LEDs off; run transmit2leds afterwards to update the LEDs
	\param [in]  	uint8_t *lightdata : lightdata array that holds the color values for the stribe
    \return 		void
    \note     		This function sets the lightdata array to 0x00. To update the stribe run transmit2leds afterwards! 		
 */
void resetstribe(uint8_t *lightdata)
{
	struct color24bit color;
	color.blue = 0x00;
	color.green= 0x00;
	color.red = 0x00;
	setfullcolor(color, lightdata);
}

/** \brief  Rotate the lightdata for 1 LED Position; run transmit2leds afterwards to update the LEDs
	\param [in]  	uint8_t *lightdata : lightdata array that holds the color values for the stribe
	\param [in]  	uint8_t direction : direction to rotate
    \return 		void
    \note     		This function rotates lightdata array. To update the stribe run transmit2leds afterwards! 
					The rotation "moves every LED" by one step, the overflowing LED is appended at the other ending.
					Example:
					RED BLUE YELLOW GREEN ... rotate... BLUE YELLOW GREEN RED
					other direction:
					RED BLUE YELLOW GREEN ... rotate... GREEN RED BLUE YELLOW
 */
void rotate(uint8_t *lightdata, uint8_t direction)
{
	uint8_t temp1, temp2, temp3;
	uint8_t *tempp;
	uint16_t i;
	
	if (direction==0)
	{
		//Store overflowing LED
		temp1 = *lightdata;
		temp2= *(lightdata+1);
		temp3 =*(lightdata+2);
		//Rotate the array (minus 1 LED-->overflow; 1 LED correlate three 8 Bit color values)
		for (i=0;i<NumOfLeds*3-3;i++)
		{	//increase the array pointer step by step
			*lightdata = *(lightdata+3);
			lightdata++;
		}
		//assign overflowed LED
		*lightdata++ = temp1;
		*lightdata++ = temp2;
		*lightdata++ = temp3;
		
	}
	else
	{
		//Set a pointer to the end of the lightdata
		tempp = lightdata + NumOfLeds*3 -1;
		//Store overflowing LED
		temp1 = *tempp;
		temp2 = *(tempp-1);
		temp3 = *(tempp-2);
		
		//Rotate the array (minus 1 LED-->overflow; 1 LED correlate three 8 Bit color values)
		for (i=0;i<(NumOfLeds*3-3);i++)
		{	//decrease the array pointer step by step
			*tempp = *(tempp-3);	
			tempp--;	
		}
		//assign overflowed LED
		*tempp--=temp1;
		*tempp--=temp2;
		*tempp = temp3;		
	}
}

/** \brief  Rotate the lightdata for n LED Positions; run transmit2leds afterwards to update the LEDs
	\param [in]  	uint8_t *lightdata : lightdata array that holds the color values for the stribe
	\param [in]  	uint8_t direction : direction to rotate
	\param [in]  	uint8_t width : width to rotate
    \return 		void
    \note     		This function rotates lightdata array. To update the stribe run transmit2leds afterwards! 
					The rotation "moves every LED" by n steps, the overflowing LEDs are appended at the other ending.
					Example:
					RED BLUE YELLOW GREEN PINK ... rotate 2 ... YELLOW GREEN PINK RED BLUE
					other direction:
					RED BLUE YELLOW GREEN PINK ... rotate 2 ... GREEN PINK RED BLUE YELLOW
 */
void rotateN(uint8_t *lightdata, uint8_t direction, uint8_t width)
{
	uint8_t i;
	for (i=0;i<width;i++)
	{
		rotate(lightdata,direction);
	}
}

/** \brief  init the runled effect; run runrunled afterwards to start the effect
	\details			This function initializes the running LED effect. The running LED effect has
					a background color that is used for all LEDs except one. One LED is in the
					foreground color an moves stepwise along the stribe. The initialization prepares
					the lightdata array by setting one LED at the start position and filling the
					others with the background color. 
	\param [in]  	struct color24bit color : 24 Bit color for the effect
	\param [in]  	uint8_t *lightdata : lightdata array that holds the color values for the stribe
	\param [in]  	struct color24bit background : 24 Bit color for the effect background
    \return 		void
    \note     		Run runrunled afterwards to start the effect! 
 */
void initrunled(struct color24bit color, uint8_t *lightdata, struct color24bit background)
{
	setfullcolor(background,lightdata);
	changeled(color, lightdata,0);
}

/** \brief  Do the runled effect; before this function is called the lightdata needs to be initiliazed using initrunled!
	\details			This function runs the running LED effect. The running LED effect has
					a background color that is used for all LEDs except one. The one LED
					moves stepwise to the next position depending on the chosen direction.
					Direction 0/1 are right/left, direction 2 runs from left to right an back again.
					For direction 0/1 the running LED overflows and begins on the other ending.
	\param [in]  	uint8_t *lightdata : lightdata array that holds the color values for the stribe
	\param [in]  	uint8_t direction : movement direction, 0/1 = right/left, 2 = left->right and back
    \return 		void
    \note     		No need to run transmit2leds afterwards! This is already done in the function.
					The function is interrupted if a new UART package is completely received so a new
					effect gets active.
 */
void runrunled(uint8_t *lightdata, uint8_t direction)
{
	uint8_t i;
	
	//Run from left to right and back, one loop in this function, main while repeats the effect
	if (direction==2)
	{
		for (i=0;i<NumOfLeds;i++)
		{
			transmit2leds(lightdata);
			rotate(lightdata,1);
			effectdelay(effectime);
			if (PacketComplete==1)
				break;
		}
		for (i=0;i<NumOfLeds;i++)
		{
			
			rotate(lightdata,0);
			transmit2leds(lightdata);
			effectdelay(effectime);
			if (PacketComplete==1)
				break;
		}	
	}
	else
	{	//Only one rotation is done, main while does the effect
		rotate(lightdata,direction);
		transmit2leds(lightdata);
		effectdelay(effectime);		
	}
}

/** \brief  blink the whole stribe; this function does not need another function call
	\details			This function creates a blinking effect. First all LEDs are set to the
					chosen color, after the defined delay the LEDs are turned off. This is
					repeated in the main while loop.
	\param [in]  	struct color24bit color : color for the blink effect	
	\param [in]  	uint8_t *lightdata : lightdata array that holds the color values for the stribe
    \return 		void
    \note     		No need to run transmit2leds afterwards! This is already done in the function.
 */
void blinkled(struct color24bit color, uint8_t *lightdata)
{
	//Set the chosen color
	setfullcolor(color, lightdata);
	transmit2leds(lightdata);
	effectdelay(effectime);
	//Turn the stribe off
	resetstribe(lightdata);
	transmit2leds(lightdata);
	effectdelay(effectime);
}

/** \brief  initialize the alternating function; call run_alternating afterwards
	\details			This function initializes the alternating effect. The effect assigns
					every even LED number in one color and the odd numbers in the background color.
					If the effect is running, the odd and even LED switch positions.				
	\param [in]  	struct color24bit color : color for the alternate effect (Init even LEDs)	
	\param [in]  	struct color24bit backcolor : color for the alternate effect bakckground (Init odd LEDs)	
	\param [in]  	uint8_t *lightdata : lightdata array that holds the color values for the stribe
    \return 		void
    \note     		Run run_alternating afterwards to start the effect!
 */
void init_alternating(struct color24bit color, struct color24bit backcolor, uint8_t *lightdata)
{
	uint16_t i;
	setfullcolor(backcolor, lightdata);		//Set background color
	for (i=0;i<NumOfLeds;i++)
	{
		if(i%2==0)
		{
			changeled(color,lightdata,i);	//set the even LEDs
		}
	}
}

/** \brief  Run the alternating effect; call init_alternating before
	\details			This function runs the alternating effect. The effect assigns
					every even LED number in one color and the odd numbers in the background color.
					If the effect is running, the odd and even LED switch positions.
					This function rotates the LEDs by one position to achieve the effect.
					The rotation direction is not of importance.					
	\param [in]  	uint8_t *lightdata : lightdata array that holds the color values for the stribe
    \return 		void
    \note     		No need to run transmit2leds afterwards! The effect is generated by the main while loop.
 */
void run_alternating(uint8_t *lightdata )
{
	transmit2leds(lightdata);
	effectdelay(effectime);
	rotate(lightdata,1);
}

/** \brief  Recolor the LED stribe; no other function call is necessary
	\details			This function generates a recolor effect. The old configuration of the
					LEDs is overwritten with the new color step by step. When the whole stribe
					is filled with the new color the effect ends.
	\param [in]  	struct color24bit color : color that is used for recoloring
	\param [in]  	uint8_t *lightdata : lightdata array that holds the color values for the stribe
    \return 		void
    \note     		No need to run transmit2leds afterwards! The effect is standalone and ends if the stribe 
					is recolored.
 */
void recolor(struct color24bit color, uint8_t *lightdata)
{
	uint8_t i;
	for (i=0;i<NumOfLeds;i++)
	{
		changeled(color,lightdata,i);
		transmit2leds(lightdata);
		effectdelay(effectime);
		if (PacketComplete==1)
			break;
	}
}

/** \brief  Generate a fading color effect. No other function call is necessary.
	\details			This function generates a fading color effect. At the beginning the whole
					stribe is filled with the chosen color. The color intensity of each color channel
					(blue, red, green) is decreased until the stribe is off. After that the color
					values are increased until the chosen color values are reached. The effect looks
					different depending on the chosen color because the color value proportion is not kept
					over the whole effect. 
	\param [in]  	struct color24bit color : color that is used for the fading effect
	\param [in]  	uint8_t *lightdata : lightdata array that holds the color values for the stribe
    \return 		void
    \note     		No need to run transmit2leds afterwards! The effect is standalone and ends is looped
					in the main while loop. The color value proportion is not kept over the whole effect.
 */
void faden(struct color24bit color, uint8_t *lightdata)
{
	uint8_t i;
	uint8_t maxgreen, maxred, maxblue;
	maxgreen =color.green;
	maxblue = color.blue;
	maxred = color.red;
	for (i=0;i<255;i++)	//Fade down to LED off
	{
		setfullcolor(color,lightdata);
		transmit2leds(lightdata);
		effectdelay(effectime);
		//Decrease the color values that are greater than 0, stop if every value is 0
		if (color.green > 0)
		{
			--color.green;
		}
		if (color.blue > 0)
		{
			--color.blue;
		}
		if (color.red > 0)
		{
			--color.red;
		}
		if (color.red == 0 && color.blue == 0 && color.green == 0)
		{
			break;
		}
		if (PacketComplete==1)
		{
			break;
		}
	}
	
	for (i=0;i<255;i++)	//Fade up to chosen color
	{
		setfullcolor(color,lightdata);
		transmit2leds(lightdata);
		effectdelay(effectime);
		//Increase the color values is they are lower than the chosen color value, stop if all maximums are reached
		if (color.green < maxgreen)
		{
			++color.green;
		}
		if (color.blue < maxblue)
		{
			++color.blue;
		}
		if (color.red < maxred)
		{
			++color.red;
		}
		if (color.red == maxred && color.blue == maxblue && color.green == maxgreen)
		{
			break;
		}
		if (PacketComplete==1)
		{
			break;
		}
	}	
}

/** \brief  Initialize a rainbow on the color array; to show the rainbow run transmit2leds afterwards
	\details			This function fills the color array with rainbow colors. For this effect the color
					array is filled with different colors that are calculated by increasing and decreasing
					the color channels to loop over a RGB palette. 
	\param [in]  	uint8_t *lightdata : lightdata array that holds the color values for the stribe
    \return 		void
    \note     		Run transmit2leds afterwards! A nice effect is to rotate the array stepwise after the
					rainbow initialization (run transmit2leds after every rotation).
					The effect directly sets color values, so there may be a problem with the color profiles
					(RGB vs. GRB). The function was primary written for WS2812 LEDs (GRB)! 
					The effect needs a minimum number of 20 LEDs to look nice!				
 */
void initrainbow(uint8_t *lightdata)
{
	uint8_t steps = NumOfLeds / 5;
	struct color24bit color;
	uint8_t i,j;
	//Start rainbow with red color
	color.red = 0xFF;
	color.blue= 0x00;
	color.green=0x00;
	j=0;
	for(i=0;i<NumOfLeds;i++)
	{
		if (j<steps)
		{
			color.blue = 0x00+0xFF/steps*j;		//increase blue to get violett		
		}
		else if(j>steps && j<=2*steps)
		{
			color.red = 0xFF-0xFF/steps*(j/2);	//decrease red to get blue
		}
		else if(j>2*steps && j<=3*steps)
		{
			color.green = 0x00+0xFF/steps*(j/3);//increase green to get cyan
		}
		else if(j>3*steps && j<=4*steps)
		{
			color.blue = 0xFF-0xFF/steps*(j/4);	//decrease blue to get green
		}
		else if(j>4*steps && j<=5*steps)
		{
			color.red = 0x00+0xFF/steps*(j/5);	//increase red to get yellow
		}
		else if(j>6*steps)
		{
			color.green = 0xFF-0xFF/steps*(j/6);//decrease green to get red
		}
		j++;
		changeled(color,lightdata,i);
	}
}

/** \brief  Initialize the easteregg; do not use directly; this function is used by the easteregg function
	\param [in]  	struct color24bit color : color for the easteregg
	\param [in]  	uint8_t *lightdata : lightdata array that holds the color values for the stribe
    \return 		void
    \note     		Do not use this function directly; this function is used by the easteregg function	
 */
void eastereggbase(struct color24bit color, uint8_t *lightdata)
{
	uint8_t i,j;
	uint8_t n;
	j=NumOfLeds;
	for (i=0;i<NumOfLeds;i++)
	{
		n=(j-i);
		changeled(color,lightdata,0);
		while(n-->0)
		{
			rotate(lightdata,1);
			transmit2leds(lightdata);
			effectdelay(effectime);
		}
		if (PacketComplete==1)
		break;
	}
}

/** \brief  Run the easteregg; No other function call is necessary.
	\param [in]  	uint8_t *lightdata : lightdata array that holds the color values for the stribe
    \return 		void
    \note     		Just try it :-) funny looking effect	
 */
void easteregg(uint8_t *lightdata)
{
	struct color24bit color, color2;
	uint8_t i;
	color=colorconv8to24(252);
	color2=colorconv8to24(201);
	eastereggbase(color2,lightdata);
	for (i=0;i<100;i++)
	{
		if (PacketComplete==1)
		break;
		_delay_ms(50);
	}
	eastereggbase(color,lightdata);
	for (i=0;i<100;i++)
	{
		if (PacketComplete==1)
		break;
		_delay_ms(50);
	}
}

/** \brief  This function fills up the stribe; No other function call is necessary.
	\details		This function fills up the whole stribe and beginns again if it is finished.
					First one LED moves in the chosen color stepwise through the whole stribe and recolors
					all LEDs in the background color which have already been passed.
					At the end of the stribe the LED stays an the next single LED is going to move to
					the last-1 position. The next LED to the last-2 position. This is going on until
					the whole stribe is colored. Then the effect restarts (main while loop).
	\param [in]  	struct color24bit color : foreground color for the moving LED
	\param [in]  	struct color24bit backcolor : background color
	\param [in]  	uint8_t *lightdata : lightdata array that holds the color values for the stribe
    \return 		void
    \note     		This is a standalone effect.
 */
void fillup(struct color24bit color,struct color24bit backcolor, uint8_t *lightdata)
{
	uint8_t i,j;
	for (i=0;i<NumOfLeds;i++)
	{
		for (j=0;j<NumOfLeds-i;j++)
		{
			changeled(color,lightdata,j);			//running LED, foreground
			if (j>0)
			{
				changeled(backcolor,lightdata,j-1);	//background LEDs
			}
			transmit2leds(lightdata);
			effectdelay(effectime);
		}
		if (PacketComplete==1)
			break;
		effectdelay(effectime);
	}
}