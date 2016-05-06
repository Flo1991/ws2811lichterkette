/**************************************************************************//**
 * @file     globals.h
 * @brief    file that contains basic and global definitions, changes should be done carefully
 * @version  V1.00
 * @date     05.01.2016
 * @authors  Wank Florian
 *
 ******************************************************************************/
#include <stdint.h>

#ifndef GLOBALS_H_
#define GLOBALS_H_

//macros to display infos for CPU Frequency or other defines
#define _STR_EXPAND(tok) #tok
#define _STR(tok) _STR_EXPAND(tok)
#define _CPU_INFO(x) CPU_FREQUENCY##x

/** \brief macro for global variable management*/
#ifndef EXTERN
#define EXTERN extern
#endif

/** \brief global variable for number of leds to control*/
EXTERN uint8_t NumOfLeds;
/** \brief global effectime for effect delays, a higher value means a higher delay*/
EXTERN uint16_t effectime;
/** \brief global ledtype, 11 = WS2811 (RGB Color), 12 = WS2812 (GRB Color)*/
EXTERN uint8_t ledtype;
/** \brief default LED type of the stribe (11 for WS2811, do not change here! change ledtype in main function!)*/
#define BASELEDTYPE 11

/** \brief definition for maximum number of different colors that can be handled at the same time 
			(the maximum value should be 250, a higher value may result in an memory overflow refering to 2kByte (atmega328p))*/
#define MAXNUMCOLORS 50
/** \brief definition for UART Buffer, must be at least MAXNUMCOLORS+5*/
#define UART_BUFFER_SIZE 80

/** \brief color array containing the received packed 8-Bit colors*/
EXTERN uint8_t CompColorArray[MAXNUMCOLORS];
/** \brief receive buffer for UART communication*/
EXTERN uint8_t RecBuffer[UART_BUFFER_SIZE];
/** \brief counter for accessing the CompColorArray indices for data income*/
EXTERN uint8_t BufferCounter;
/** \brief variable to store the current packet length of the UART packet*/
EXTERN uint8_t DataLen;
/** \brief global effect variable to switch between the effects*/
EXTERN uint8_t effect;

//EXTERN uint8_t speed;

/** \brief flag to store if a UART packet is complete; a packet is complete if the BufferCounter equals DataLen*/
EXTERN uint8_t PacketComplete;
/** \brief flag to store if the @link PREAMBLE @endlink has been received*/
EXTERN uint8_t PaketStart;
/** \brief flag to show if the RecBuffer is in copy process so that the array cannot be filled with new data from UART*/
EXTERN uint8_t IsReading;
/** \brief current data received from UART*/
EXTERN volatile char ReceivedChar;

/** \brief CPU Frequency definition for avr delay function*/
#ifndef F_CPU
#define F_CPU 8000000
#endif
#endif /* GLOBALS_H_ */