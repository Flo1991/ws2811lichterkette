/**************************************************************************//**
 * @file     Lightstribe.c
 * @brief    basic functions for controlling WS2811/WS2812 LEDs
 * @details  This file contains the basic functions to control WS2811/WS2812 LEDs using 
			 an AVR. It declares the function to transmit lightdata to a stribe using the
			 one wire protocol. For the right timing be aware of the crystal frequency!
			 This code is written for using an extern clock of 16 MHz, if you change it 
			 you need to modify the number of NOPs in the macros defined in the header file.
			 This file also contains the basic functions to set or to change one LED in the stribe.
			 The whole system is working with a color array that stores the 24 Bit colors for all
			 LEDs in an GRB format (WS2812). Every effect changes the array, after that the array
			 is sent out by the transmit2leds function. This guarantees a correct timing. The most
			 functions base on uint8_t variables so the maximum length of the stribe to control 
			 contains 255 LEDs. This should not be changed because you have hardware limitations as well
			 that will limit a basic setup to 200-250 LEDs.
 * @version  V1.00
 * @date     05.01.2016
 * @authors  Wank Florian
 *
 ******************************************************************************/

#include "globals.h"
#include "Lightstribe.h"
#include <util/delay.h>

/** \brief  change the color of one LED at a specific position; run transmit2leds afterwards to update the LEDs
    \param [in]  	struct color24bit color : 24 bit color in GRB format
	\param [in]  	uint8_t *lightdata : pointer to the complete lightdata that contains all color values
	\param [in]  	uint8_t lednr : position of the LED that should be changed
    \return 		void
    \note     		the right color format is created using the colorconv8to24-function with the ledtype predefined
 */
void changeled(struct color24bit color, uint8_t *lightdata, uint8_t lednr)
{
	if (lednr>NumOfLeds)
		return;
	lightdata=lightdata+lednr*3;
	*lightdata++=color.green;
	*lightdata++=color.red;
	*lightdata++=color.blue;
}

/** \brief  set the color of one LED at a specific position, all others are off; run transmit2leds afterwards to update the LEDs
    \param [in]  	struct color24bit color : 24 bit color in GRB format
	\param [in]  	uint8_t *lightdata : pointer to the complete lightdata that contains all color values
	\param [in]  	uint8_t lednr : position of the LED that should be set
    \return 		void
    \note     		the right color format is created using the colorconv8to24-function with the ledtype predefined;
					all other LEDs are cleared so they are off
 */
void setled(struct color24bit color, uint8_t *lightdata, uint8_t lednr)
{
	uint8_t ledcolor;
	uint16_t i;
	if (lednr>NumOfLeds)
		return;
	//Loop over the whole color array (-->NumOfLeds*3)
	for (i=0;i<NumOfLeds*3;i++)
	{
		if (i==(lednr*3) || i==(lednr*3+1) || i==(lednr*3+2))
		{	//position of the LED to set
			ledcolor = i%3;
			if (ledcolor==0)
				*lightdata++=color.green;
			else if(ledcolor==1)
				*lightdata++=color.red;
			else
				*lightdata++=color.blue;
		}
		else
		{	//all others off (0x00-->black)
			ledcolor = i%3;
			if (ledcolor==0)
				*lightdata++=0x00;
			else if(ledcolor==1)
				*lightdata++=0x00;
			else
				*lightdata++=0x00;
		}	
	}	
}

/** \brief  transmit the color array to the stribe
	\details		To control the LEDs of type WS2811/WS2812 a critical timing is necessary. To achieve the correct
					timing and to create effects the lightdata is stored in an array first. All operations effect the
					color array. If the color array is prepared it is transmitted to the stribes via a one-wire protocol
					using this function. This function generates the high and low times using assembler NOPs to achieve 
					the timing. The number of NOPs are stored in macros for transmitting a Low Bit (SETLOW) or a High Bit
					(SETHIGH). This function should not be changed or optimized because of the timing!
	\param [in]  	uint8_t lightdata[] : data with the colors for each LED to control
    \return 		void
    \note     		This function should not be changed or optimized because of the timing!
					Do not use higher optimization than O1!!!
					Do not remove the {} brackets because SETLOW/SETHIGH are definitions with several commands!
 */
void transmit2leds(uint8_t lightdata[])
{
	uint16_t i ;
	uint8_t byte2send ;
	for(i=0;i<NumOfLeds*3;i++)
	{
		byte2send = lightdata[i];	
		//Transmit each Bit of one Byte using the One Wire Protocoll
		if ((byte2send & 128)==0)
		{
			SETLOW
		}
		else
		{
			SETHIGH
		}
		if ((byte2send & 64)==0)
		{
			SETLOW
		}
		else
		{
			SETHIGH
		}
		if ((byte2send & 32)==0)
		{
			SETLOW
		}
		else
		{
			SETHIGH
		}
		if ((byte2send & 16)==0)
		{
			SETLOW
		}
		else
		{
			SETHIGH
		}
		if ((byte2send & 8)==0)
		{
			SETLOW
		}
		else
		{
			SETHIGH
		}
		if ((byte2send & 4)==0)
		{
			SETLOW
		}
		else
		{
			SETHIGH
		}
		if ((byte2send & 2)==0)
		{
			SETLOW
		}
		else
		{
			SETHIGH
		}
		if ((byte2send & 1)==0)
		{
			SETLOW
		}
		else
		{
			SETHIGH
		}	
	}
	_delay_us(55);		//defined delay after the transmission is complete (Datasheet says >=50us)
}