
/*! \mainpage Use WS2811/WS2812 LEDs with an AVR
 * \image html tree.png
 * \section intro_sec Introduction
 * This project is about using an WS2811 or WS2812 lightstribe with an AVR controller.
 * It is possible to handle up to 250 LEDs at the same time, so I chose an Atmega328p with enough RAM amount. If you
 * want to handle less LEDs you can use most parts of this project with every AVR. The AVR is programmed
 * to receive the light data over UART so you can control the LEDs by using a serial interface.
 * The interface uses a specified simple protocol which is described in \ref protocol_sec section. Everything
 * has been developed in a university course to control the lights of a Christmas tree. In the original
 * implementation there were some further components included. This is a simplified version of the implementation
 * so that everyone can use it. As an example for controlling the LEDs using a smart phone the \ref esp_sec
 * section shows how this could be done by using a webserver on the ESP8266. You can use everything else that
 * provide a serial interface (maybe connect with a bluetooth serial module). The structure of this documentation
 * is split in a hardware part for the AVR that describes the basic hardware that should be used. The next part
 * is about how the software is working on the AVR that handles the LEDs and different effects. You may include
 * some more stuff in your own. After that you can see a small protocol overview, where you find which command
 * can be sent to the AVR to control the LEDs. Be aware that at the initialization state all LEDs are off.
 * At the last point you can find an example how to use the implementation with an ESP8266 with a webserver.
 * You will find the source code for the ESP8266 and the basic hardware setup.
 
 \section hardware_sec Hardware
 The basic hardware you need is a AVR controller an some WS2811 or WS2812 LEDs you want to control. The AVR
 controller should have an hardware UART, otherwise you need to write some code for a software serial. In the project
 we chose an Atmega328p that has enough RAM to control 250 LEDs. The internal software structure buffers the color
 data for the LEDs to achieve an accurate timing, see section \ref software_sec. The AVR can be used with the internal
 clock at 8 MHz, remember to clear the clock divider fuse. Otherwise an external 8 MHz or 16 MHz clock source can be used,
 the definition \ref F_CPU must be set to the frequency you chose (remember to set the fuses for an external clock source).
 As an example the figure \ref one shows using an external 16 MHz crystal.
 \anchor one
 \image html  Ws2811_Atmega328_schematic.png "schematic of the AVR to controll WS2812/WS2811"
 \image latex  Ws2811_Atmega328_schematic.png "schematic of the AVR to controll WS2812/WS2811"
 \image rtf  Ws2811_Atmega328_schematic.png "schematic of the AVR to controll WS2812/WS2811"
 
 As you can see in the picture the AVR is programmed by using the ISP interface. The WS2812/WS2811 get the same voltage as
 the AVR, the light data is available at PinB0, you may change this if you like. Referring to the LEDs be aware of the current
 amount they may draw if every LED has its full brightness. One WS2812 can draw up to 60 mA, so one meter with 30 LEDs already
 need 1,8 A. If you want to control more LEDs you may have a problem with the voltage drop along the stribe. For example
 if you control 180 LEDs at six meters you not only need 10,8 A, furthermore you will probably have a voltage drop up to 2 V.
 To reduce the voltage drop you must increase the wire size with parallel wires to you stribe. You can see the voltage drop if
 you set all LEDs to white. If you have only a small voltage drop every LED will have the same color. If the voltage drop
 is too much you can see that the last LEDs will have less blue color, so they will light in a warm white color even up to red.
 If you want to try out the LEDs with the AVR you can build up everything on a breadboard. Pinheaders can be soldered easy at the
 light stribes as you can see in the figure \ref two.
 \anchor two
 \image html   WS2812.png "WS2812 stribe with pin header"
 \image latex   WS2812.png "WS2812 stribe with pin header"
 \image rtf   WS2812.png "WS2812 stribe with pin header"
 The connect GND to the common ground with the AVR, 5 V should be connected to a power supply that can handle the current you need.
 DI is the data in line, this should be connected to PinB0 at the AVR. The stribe is like a big shifting register, all the data
 you sent is shifted bit by bit through the stribe. So DO is the data out pin, you see some data at this pin if all LEDs before
 had already received their color data. The one wire protocol of the LEDs is described in the \ref software_sec section.
 
 \section software_sec software implementation
 

  \section install_sec Installation
  
  \subsection step1 Step 1: Opening the box
  
   etc...
  
  \section protocol_sec protocol overview
  
   
  
  \section esp_sec control via ESP8266
  
  author: Florian Wank, 2016
 */
/**************************************************************************//**
 * @file     ws2811lichterkette.c
 * @brief    main file for interfacing WS2811/WS2812 LEDs
 * @details  This file contains the main environment for interfacing WS2811/WS2812 LEDs with an AVR.
			 The implementation has been done for an atmega328p. You may use another controller but
			 be aware of the memory you need for the color array (dynamically allocated). 
			 The AVR interfaces the one wire of the LEDs. All operations (effects, colorchange etc.)
			 are done on an lightdata array, that needs to be transmitted to the LEDs after your
			 operations. The reason for this is the critical timing for interfacing the LEDs. So also
			 be aware if you change the clock speed. If you do so you have to change the number of 
			 NOPs in the macros of Lightstribe.h. Because of the critical timing compile all files
			 at optimization O1! Furthermore be aware of the @link BAUD BAUDRATE  @endlink changes, the BAUD error may be
			 to worse if you change the CPU frequency. <br>
			 The one wire output is on the PIN B0! You can change in the main and @link Lightstribe.h @endlink. <br>
			 By default this file just initializes the AVR system, no updates to the LEDs are done by
			 default. To change the LED configuration you need to access the AVR UART Interface with
			 another controller (FTDI is also possible). Over the UART you send a message containing all
			 relevant information for the system. Therefore a simple protocol is used:
			 1 Byte preamble (254)
			 1 Byte total packet length (including the preamble)
			 1 Byte effect
			 1 Byte effect delay (effect speed)
			 1 Byte number of LEDs to control
			 n Bytes containing 8-Bit color values (RGB 3-3-2), depended on the effect, max. 50 values
			 The preamble 254 must never be used at another position!!!<br>
			 Protocol examples:<br>
			 @link SETFULLCOLOR @endlink:		254 6  0  1 20 22				 <br>
			 @link FILLUP		@endlink:		254 7  1 22 20 22 201			 <br>
			 @link BLINK		@endlink:		254 6  2 55 20 56				 <br>
			 @link RUNLED		@endlink:		254 7  3 55 20 56 151			 <br>
			 @link INITRAINBOW	@endlink:		254 5  9  0 20					 <br>
			 @link ROTATE_R		@endlink:		254 5 11 23 20					 <br>
			 @link CUSTOM		@endlink:		254 8 12  1 20 22 201 60		 <br>
			 @link EASTEREGG	@endlink:		254 5 13  2 20					 <br>
			 
			 The UART communication is done by using an RX interrupt an storing the data into a temp array.
			 In the main loop a flag shows if a data packet is complete. So you will get no update on the LEDs
			 if the UART package was wrong (too short).
			 In the project this programm has been written the UART was controlled by an ESP8266 or BLE113.
			 Have Fun!
 * @version  V1.00
 * @date     05.01.2016
 * @authors  Wank Florian
 *
 ******************************************************************************/
//define global variables
#define EXTERN
#include "globals.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>      
#include <stdlib.h>    
#include <string.h>

#include "Lightstribe.h"
#include "LedEffects.h"

//UART basic definitions
/** \brief Baudrate definition, choose 76800 or 38400, faster value prefered, the maximum speed of ESP8266 software-UART is 38400*/
#define BAUD 38400	
/** \brief calculate baudrate register value*/
#define MYUBRR F_CPU/16/BAUD -1

#define BAUD_REAL (F_CPU/(16*(MYUBRR+1)))		/*real baudrate in this configuration*/
#define BAUD_ERROR ((BAUD_REAL*1000)/BAUD)		/*calculate baudrate error*/	
#if ((BAUD_ERROR<990) || (BAUD_ERROR>1010))
	#error baudrate error greater 1% !			/*show an error message if the baudrate error is greater than 1%*/
#endif

//Protocol definition for UART communication
//The protocol is defined as:
//1 Byte preamble (254)
//1 Byte total packet length (including the preamble)
//1 Byte effect
//1 Byte effect delay (effect speed)
//1 Byte number of LEDs to control
//n Bytes containing 8-Bit color values (RGB 3-3-2), depended on the effect, max. 50 values

/** \brief definition of the preamble is 254, no other data field must contain this value*/
#define PREAMBLE 254		
/** \brief definition of the second field; contains the total packet length (including the preamble)*/
#define LENINDEX 1			
/** \brief definition of 1 Byte effect at third position, the MSBit is used to choose WS2811/WS2812 (color profile RGB or GRB)*/
#define EFFECTINDEX 2		
/** \brief definition of the delay field, contains the delay duplicator*/
#define DELAYINDEX 3		
/** \brief field position for the number of LEDs to control, should be max. 200 (dynamic memory allocation for the lightdata array)*/
#define NUMOFLEDINDEX 4		


//compiling info output
#pragma message("MYUBRR: " _STR(MYUBRR))
#pragma message("CPU Frequency: " _STR(F_CPU) "Hz")
#pragma message("Baudrate: " _STR(BAUD))
#pragma message("Configuration: MAXNUMCOLORS=" _STR(MAXNUMCOLORS)  " | UART_BUFFER_SIZE=" _STR(UART_BUFFER_SIZE) " | PREAMBLE=" _STR(PREAMBLE))

/** \brief  Init the hardware UART with Baud = 76800/38400, depending on @link BAUD @endlink definition, 8 Databits, 1 Stopbit, no Parity
    \return 		void
    \note     		This function depends on the oscillator clock frequency and the @link BAUD @endlink defintion.
					If your UART is not working first check all frequency issues (Fuse settings, clock speed, clock divider, Baudrate)			
 */
void init_uart(void)
{
	DDRD |= _BV(PD1);
	DDRD &= ~_BV(PD0);
	
	 //Set BAUD
	 UBRR0H = ((MYUBRR) >> 8);
	 UBRR0L = MYUBRR;
	 
	 UCSR0B |= (1 << RXEN0) ;//| (1 << TXEN0);     // Enable receiver and transmitter
	 UCSR0B |= (1 << RXCIE0);					// Enable the receiver interrupt
	 UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);   // 8 data Bit, one stop Bit
}

/** \brief main function, should never end, effects are handled in main while*/
int main(void)
{
	uint16_t i,j;							//helper variables (counters)
	uint8_t TempBuffer[UART_BUFFER_SIZE];	//Temp. buffer for copy of the UART data to achieve data consistency	
	uint8_t *lightdata;						//lightdata pointer for lightdata array; the array size is dynamic to controll different numbers of LEDs
	
	NumOfLeds=50;							//default number of LEDs is 50 => one stribe
	//Flag initializations
	PacketComplete=0;							
	IsReading=0;
	PaketStart=0;	
	BufferCounter = 0;

	memset(RecBuffer,0,sizeof(RecBuffer[0])*UART_BUFFER_SIZE);	//clear the buffer	
	memset(TempBuffer,0,sizeof(RecBuffer[0])*UART_BUFFER_SIZE);	//clear the buffer
	ledtype = BASELEDTYPE;					//set default ledtype, 11 =>WS2811, 12 =>WS2812
	
	//Set the LED output Port (Pin B0 is used for LED data output)
	DDRB = 0x01;						
	PORTB = 0x00;

	//Basic initializations
	ReceivedChar = 1;
	effectime = 10;
	effect=255;
	BufferCounter=0;
	
	init_uart();							//Init the hardware UART
	sei();									//enable global interrupts

	//main system loop
	while(1){
		if (PacketComplete==1)	//new UART package containing color and effect data is available
		{
			//Prohibit the access to the UART RecBuffer while copying the data to a Temp Buffer
			IsReading=1;		
			PaketStart=0;
			memcpy(TempBuffer,RecBuffer,DataLen);	//Copy the UART data to a temp array	
			effect=TempBuffer[EFFECTINDEX] & 0x7F;	//get the effect from the temp array
			effectime=TempBuffer[DELAYINDEX];		//get the delay time for the effect form the temp array
			ledtype=BASELEDTYPE+((TempBuffer[EFFECTINDEX] & 0x80)>>7);//configure the ledtype depending on the MSBit of the effect
			NumOfLeds=TempBuffer[NUMOFLEDINDEX];	//get the number of leds to control
			IsReading=0;							//allow access to the UART RecBuffer
			memcpy(CompColorArray,&TempBuffer[5],DataLen-5);	//generate compressed color array
			if (lightdata!=NULL)
			{
				free(lightdata);
			}
			lightdata = (uint8_t *) malloc (NumOfLeds*3);		//allocate the lightdata array for uncompressed colors
			PacketComplete=0;						//reset PacketComplete flag
		}
		else
		{
			//main switch for effect handling
			switch(effect)
			{
				case SETFULLCOLOR:
					setfullcolor(colorconv8to24(CompColorArray[0]),lightdata);
					transmit2leds(lightdata);
					break;
				case FILLUP:
					fillup(colorconv8to24(CompColorArray[0]),colorconv8to24(CompColorArray[1]),lightdata);
					transmit2leds(lightdata);
					break;
				case BLINK:
					blinkled(colorconv8to24(CompColorArray[0]),lightdata);
					break;
				case RUNLED:
					initrunled(colorconv8to24(CompColorArray[0]),lightdata,colorconv8to24(CompColorArray[1]));
					effect++;
				case 4:
					runrunled(lightdata,1);
					break;
				case ALTERNATE:
					init_alternating(colorconv8to24(CompColorArray[0]),colorconv8to24(CompColorArray[1]),lightdata);
					effect++;
				case 6:
					run_alternating(lightdata);
					break;
				case RECOLOR:
					recolor(colorconv8to24(CompColorArray[0]),lightdata);
					effect=255;
					break;
				case FADEN:
					faden(colorconv8to24(CompColorArray[0]),lightdata);
					break;
				case INITRAINBOW:
					initrainbow(lightdata);
					transmit2leds(lightdata);
					break;
				case ROTATE_R:
					rotate(lightdata,0);
					effectdelay(effectime);
					transmit2leds(lightdata);
					break;
				case ROTATE_L:
					rotate(lightdata,1);
					effectdelay(effectime);
					transmit2leds(lightdata);
					break;
				case CUSTOM:
				//The custom effect assigns up to MAXNUMCOLORS=50 individual colors to the stribe
				//if the number of colors is smaller than the number of LEDs the colors are repeated using
				//modulo operation
					for (i=0;i<NumOfLeds;i++)
					{
						j = i % (DataLen-5);
						changeled(colorconv8to24(CompColorArray[j]),lightdata,i);
					}
					transmit2leds(lightdata);
					effect=255;
					break;
				case EASTEREGG:
					easteregg(lightdata);
					break;
				default:	//do nothing
					break;
			}
		}
		
		
	}
	  
}


/** \brief UART Interrupt handler, interrupts when new data is available in the RX buffer*/
ISR (USART_RX_vect)
{
	ReceivedChar = UDR0;							//Read data from the RX buffer     
	if (ReceivedChar==PREAMBLE && IsReading==0)		//Store data in the RecBuffer array only if it is not accessed by the main function
	{
		PacketComplete=0;
		PaketStart=1;								//Set packet start flag (-->254=PREAMBLE has been received)
		memset(RecBuffer,0,sizeof(RecBuffer[0])*UART_BUFFER_SIZE);//clear the buffer	
		BufferCounter=0;
		RecBuffer[0]=ReceivedChar;					//Store the preamble
	}
	else if (PaketStart==1)
	{
		//Store all Bytes after the preamble
		BufferCounter++;    
		RecBuffer[BufferCounter]=ReceivedChar;
		DataLen=RecBuffer[LENINDEX];				//Store data len of the data packet (preamble included)
		if (DataLen==BufferCounter+1)
		{
			PacketComplete=1;						//a whole packet has been received, update the effect in main
		}	     
	}	 
}