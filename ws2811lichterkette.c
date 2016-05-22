
/*! \mainpage Use WS2811/WS2812 LEDs with an AVR

\tableofcontents

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
 
  \section usage_sec Basic usage
  For using this implementation follow this steps:
  <ul>
  <li>set up the hardware as descriped in section \ref hardware_sec
  <li>set the \ref F_CPU clock to the value for your hardware
  <li>set the \ref BAUD to the value you like, 76800 or 38400 are suggested
  <li>compile your implementation (only O1 optimization is supported)
  <li>program your AVR with your binaries
  <li>set the clock divider fuse and the clock source fuse referring to your implementation
  <li>send protocol data (see section \ref protocol_sec) to the RX pin of the AVR over a serial device, e.g. an FTDI, ESP8266 or Arduino
  (UART is 8N1 on your chosen \ref BAUD)(example data 254 6 0 1 20 22 = 0xFE 0x06 0x00 0x01 0x14 0x16)
  </ul>
 
 \section hardware_sec Hardware
 The basic hardware you need is a AVR controller an some WS2811 or WS2812 LEDs you want to control. The AVR
 controller should have an hardware UART, otherwise you need to write some code for a software serial. In the project
 we chose an Atmega328p that has enough RAM to control 250 LEDs. The internal software structure buffers the color
 data for the LEDs to achieve an accurate timing, see section \ref software_sec. The AVR can be used with the internal
 clock at 8 MHz, remember to clear the clock divider fuse. Otherwise an external 8 MHz or 16 MHz clock source can be used,
 the definition \ref F_CPU must be set to the frequency you chose (remember to set the fuses for an external clock source).
 As an example \ref one "figure 1" shows using an external 16 MHz crystal.
 \anchor one
 \image html  Ws2811_Atmega328_schematic.png "Figure 1: schematic of the AVR to controll WS2812/WS2811"
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
 light stribes as you can see in \ref two "figure 2".
 \anchor two
 \image html   WS2812.png "Figure 2: WS2812 stribe with pin header"
 \image latex   WS2812.png "WS2812 stribe with pin header"
 \image rtf   WS2812.png "WS2812 stribe with pin header"
 The connect GND to the common ground with the AVR, 5 V should be connected to a power supply that can handle the current you need.
 DI is the data in line, this should be connected to PinB0 at the AVR. The stribe is like a big shifting register, all the data
 you sent is shifted bit by bit through the stribe. So DO is the data out pin, you see some data at this pin if all LEDs before
 had already received their color data. The one wire protocol of the LEDs is described in the next section \ref software_sec.
 
 Datasheets:<br>
 <a href="WS2812.pdf" target="_blank"><b>Datasheet WS2812</b></a>			<br>
 <a href="WS2811.pdf" target="_blank"><b>Datasheet WS2811</b></a>			<br>
 <a href="Atmega328.pdf" target="_blank"><b>Datasheet Atmega328</b></a>		<br>
 
 \section software_sec Software implementation
 If your hardware is ready you must flash your AVR device with the provided software. Therefore the ISP-6 connector should be
 used. To get the right timing remember to set the \ref F_CPU definition to the frequency you are working at. Furthermore set
 the fuses of the AVR referring to your implementation. This means you have to clear the clock divider fuse and may have to
 change the clock source. I suggest to use the AtmelStudio to program your AVR and its fuses. <br>
 The WS2812/WS2811 are controlled by one data line that works with a one wire protocol. Because of the missing clock line
 the timing is really important, this can either be achieved by doing some trick with the hardware interfaces (e.g. using the
 spi interface) or by bit banging. In this implementation bit banging is used. To get a good timing all color data must be
 transmitted in one block that is not interrupted by some other code. The timing specifications of the WS2812/WS2811 LEDs
 can be found in table \ref timingtable "1" which refers to the datasheet ( <a href="WS2812.pdf" target="_blank"><b>WS2812</b></a>).<br>
 
 \anchor timingtable
 <table>
 <caption id="timingtable">Timing table for WS2812/WS2811 one wire protocol</caption>
 <tr><th>Information                    <th>Timing							<th> Tolerance +/-
 <tr><td>Transfer 1 Bit                 <td>HighTime+LowTime=1,25 탎		<td>600 ns
 <tr><td>send 0, high time              <td>0,35 탎							<td>150 ns
 <tr><td>send 0, low time               <td>0,8 탎							<td>150 ns
 <tr><td>send 1, high time              <td>0,7 탎							<td>150 ns
 <tr><td>send 1, low time               <td>0,6 탎							<td>150 ns
 <tr><td>data transmission complete, low time <td> >50 탎				<td>  -
 </table>
 
 The timing is done by setting the output and wait the required time by doing nothing (call assembly NOPs). So it is important
 to compile the provided software at O1, other optimization levels may influence the timing. To send one bit (either high or low)
 two different macros are defined in Lightstribe.h (SETHIGH and SETLOW), one LED needs 24 color bits. The macros depend on the value of #F_CPU
 you entered in globals.h. Furthermore the header file Lightstrib.h declares a color struct to handle 24 bit colors (\ref color24bit) and three
 basic functions to control the LEDs. The corresponding c file Lightstribe.c implements these functions. The most important function
 is the \ref transmit2leds function. This function and only this function transmits data to the stribe. All other functions either call
 this function or manipulate the color array. To achieve the right timing all effects and operations are done on a color array that
 stores the color information for the LEDs. The information is sent to the LEDs by calling transmit2leds with the lightdata pointer that
 points to an dynamically allocated array that stores the color information depending on the number of LEDs you want to control. Therefore
 your color array must at least be able to contain 24 bits x your number of LEDs. It can be bigger, what will allow you to create even more
 effects (e.g. if you rotate a rainbow array). So the effects that are implemented in LedEffects.c change the color array and afterwards
 the \ref transmit2leds is called. The c file LedEffects.c not only contains effects but also different necessary functions for the 
 effects and the serial color handling. The \ref colorconv8to24 function converts the received 8 bit colors from the serial port to 24 bit
 colors for the lightstribes. So you only sent 8 bit colors over the serial port to the AVR to reduce data size. Further information can be
 found in the \ref protocol_sec section. The colors are decompressed with a simple \ref map function you may know from Arduino. The main.c file
 initializes the hardware and handles the LEDs. A serial interrupt stores the data temporary. If the data transmission is complete the main
 function will extract the information and set the new configuration for the lightstribe.<br>
 The last points to be mentioned in this section are some things you need to be careful. The first thing is that the 8 bit colors are in an
 RGB 3-3-2 format. The 24 bit color format depend on the LEDs. WS2812 LEDs use a GRB color scheme while WS2811 use a RGB color scheme. This
 is important, to achieve the right color the protocol includes a bit that decides the color scheme. The right color is resolved by the
 decompressing function \ref colorconv8to24. Another thing is that the colors are not linearized, what means that you cannot say that a
 color you got from a color table will be look like this. As an example you picked an orange from a 3-3-2 rgb color table. This orange
 will not be the same orange on the LED stribe. This depends on many parameters so linearizing is too much effort and almost impossible (to
 achieve linearization you would have to measure each color, compare it and evaluate correction parameters).
   
\section protocol_sec Protocol overview
This section gives an overview of the implemented serial protocol. The goal of the protocol was to be as simple as possible, to be easily 
implemented on the AVR and to use as less resources as possible. \ref three "Figure 3" shows the base structure of the protocol.
 \anchor three
 \image html   Protoll_V1_2_engl.png "Figure 3: serial protocol structure"
 \image latex   Protoll_V1_2_engl.png "serial protocol structure"
 \image rtf   Protoll_V1_2_engl.png "serial protocol structure"
As you can see a data transmission always starts with the preamble 254(0xFE). For a fast and easy implementation this preamble value must
only be used as preamble and must never be another field value (e.g. you must not send the color value 254). The next byte that is sent
contains the total length of the packet including the preamble and the length byte. If you sent a wrong length you may get an unexpected 
behavior until a new correct data packet is sent. The third field contains the effect. The different effects are listed in table  XXX.
In Bit 7 (MSb) you can choose the LED type you want to control, set the bit to 0 for WS2811 and to 1 for WS2812 LEDs. The next byte is
a value to control the effect speed. You can set a delay between 0 (no delay) and 250 (longest delay possible). The value unit and is not
a repeatable setting for different effects. This means that the delay is no correct wait function (e.g. wait for n milliseconds). Furthermore
the effects work on the color array what may be faster for some effects and slower for others. The best thing is to try the effects with different
values. The next field contains the number of the LEDs that should be controlled. Be aware that the maximum supported number of LEDs is 250, but this
depends on your hardware. The chosen Atmega328p can handle this amount, if you choose an AVR with less RAM this will not work. What you can do is
to allocate more LEDs than you actually have. This gives you the possibility to create further effects. What happens is that only a part of the
array is sent to the LEDs but the other color values are stored internally in the AVR (in fact all color data is transmitted to the LEDs but the 
superfluous information is overwritten by new data). The last data field are the color values. One color is 8 bit RGB 3-3-2 and you should sent
the right amount of colors for your chosen effect. If you sent to less information the data block will not be evaluated because the total length
does not match. For sending some data do not forget to configure your UART (8N1 \ref BAUD) on both sides.

 \anchor effecttable
 <table>
 <caption id="effecttable">Table containing all effects available over the serial protocol</caption>
 <tr><th>Effect<br>number <th>Number of colors <br> (1 byte RGB 3-3-2)			<th> Description												<th> Example <br> (decimal) 
 <tr><td>0 = SETFULLCOLOR	<td>1							<td>All LEDs glow at the same color without changes.			<td>254 6 1 20 22
 <tr><td>1 = FILLUP			<td>2 (foreground, background)	<td>One LED steps through the stribe in the foreground color and colors all LEDs after it in the background color.<br>
																At the end of the stribe the LED stays at the foreground color and another LED starts to step through the stribe.<br>
																This continues until the whole stribe is filled in the foreground color. <br>
																Then the stribe is cleared to the background color and the effects begins again.			<td>254 7 1 22 20 22 201
 <tr><td>2 = BLINK			<td>1							<td>The stribe blinks in the chosen color and to off (=black) repeatedly.		<td>254 6 2 55 20 56
 <tr><td>3 = RUNLED			<td>2 (foreground, background)	<td>All LEDs but one are colored in the background color.<br>
																The one in the foreground color walks through the stribe with overflowing to the beginning.			<td>254 7 3 55 20 56 151
 <tr><td>5 = ALTERNATE		<td>2 (foreground, background)	<td>The LEDs are alternating in the foreground and the background color.<br>
																First the even LEDs are colored in foreground and the uneven in the background color, after that vice versa.			<td>254 7 5 55 20 56 151
 <tr><td>7 = RECOLOR		<td>1							<td>The stribe is filled in a new color step by step until the whole stribe stays in the new color.			<td>254 6 7 55 20 38
 <tr><td>8 = FADE			<td>1							<td>The destination color is set and the base colors red, green and blue are decreased step by step until the stribe is off.<br>
																After that the color values are increased until the destination color is reached.<br>
																This generates a color fading effect. The color fading is not linearized.			<td>254 6 8 55 20 201
 <tr><td>9 = INITRAINBOW	<td>no color					<td>Set the stribe in a static rainbow color.		<td>254 5 9 0 20
 <tr><td>10 = ROTATE_R		<td>no color					<td>Rotate all LEDs one step to the right side (depends on lightdata array).			<td>254 5 10 232 20
 <tr><td>11 = ROTATE_L		<td>no color					<td>Rotate all LEDs one step to the left side (depends on lightdata array).			<td>254 5 11 23 20
 <tr><td>12 = CUSTOM		<td>N colors					<td>All LEDs are set to the static color referring to the sent color values.<br>
																If there are more LEDs than color values the colors are repeated.			<td>254 8 12 1 20 22 201 60
 </table>
The missing numbers in table \ref effecttable "2" are internally used by the AVR and must not be sent over the serial port.
  
\section owneffects_sec Implement further effects
This section tells you how to implement further effects. You may use already existing functions to generate new effects or add something completely new.
All effects should be written in the LedEffects.c file and declared in its header file LedEffects.h. You must know that everything works on a lightdata
array that contains the colors stored in an array. The array is sent directly to the stribe if the  \ref transmit2leds function is called. So you first
need to manipulate the array and than send it to the stribe. The array is ordered in GRB color because the implementation has been done for WS2812 LEDs
(WS2811 LEDs can be used the colors are converted in the \ref colorconv8to24 function referring to the MSb of the effect you sent, for more information
see section \ref protocol_sec). So lightdata[0] contains one byte green data, lightdata[1] one byte red data, lightdata[2] blue data and so on. In general
you can say lightdata[N%3==0] contains green,  lightdata[N%3==1],  lightdata[N%3==2] data. So the color array has a size of \ref MAXNUMCOLORS * 3. So your
function must at least have a pointer to the lightdata array as a call value. For creating your effect some nice functions are already implemented you may
use. You can find a list of them in table \ref functiontable "3".

 \anchor functiontable
 <table>
 <caption id="functiontable">Provided help functions for your own effect</caption>
 <tr><th>Function Name					<th>call values						<th> operation
 <tr><td>\ref map						<td>x,in_min,in_max,out_min,out_max <td>calculate an x value to a new number range
 <tr><td>\ref effectdelay				<td>delay							<td>wait some time dependend on delay
 <tr><td>\ref resetstribe               <td>*lightdata						<td>clear the stribe (all LEDs off)
 <tr><td>\ref rotate		            <td>*lightdata, direction			<td>rotate stribe by one position (means 3 bytes) in direction (right/left)
 <tr><td>\ref rotateN					<td>*lightdata, direction,width 	<td>rotate LEDs by "width" positions  (means width * 3 bytes) in direction (right/left)
 <tr><td>\ref setled					<td>color, *lightdata, lednr		<td>set one LED a position lednr in the chosen color, others off (black)
 <tr><td>\ref changeled					<td>color, *lightdata, lednr		<td>change the color of one LED at position lednr, others are unchanged
 </table>
 
 Your written effect should get an own definition in LedEffects.h . The last thing is to add your definition in the main switch case structure. 
 Referring to the implemented protocol your effect is available with the number you defined in LedEffects.h. You must sent the neccessary information
 for your effect, for example the color values you need and so on. To get the color value you sent you need to call \ref colorconv8to24 to convert
 the 8 bit RGB color into a 24 bit color. All colors you sent are available in the \ref CompColorArray. The first color you sent is stored in index zero.
 Your implemented function must not care about the color order if you use the \ref colorconv8to24 function. This does the conversion depending on the
 MSb of the effect you sent over the serial port. The delay is handled by the global var \ref effectime and the number of LEDs to control is stored in
 \ref NumOfLeds. The effect is stored in the \ref effect variable. You should not do any changes on the serial part and the protocol reading, otherwise
 you will change to complete behavior of this implementation.

\section limitations_sec Requirements and Limitations
The implementation to control has the following requirements and limitations:
  <ul>
  <li>colors are 8 bit compressed so you cannot get every color value of the LEDs
  <li>the protocol implementation with the preamble 254 prohibits this value for other protocol fields (e.g. color)
  <li>approximate amount of RAM (in bytes) you need: \ref MAXNUMCOLORS(=number of LEDs to control)*3 + \ref UART_BUFFER_SIZE *2 + \ref MAXNUMCOLORS + 160
  <li>only O1 optimization is supported
  <li>8 MHz and 16 MHz clock support
  <li>fuses must be programmed manually (clock source and clock divider)
  <li>WS2801 stribes not supported (different hardware interface with two wires)
  <li>AVR should run on 5 V
  </ul>
  
\section esp_sec Example usage with an ESP8266
This section gives a short introduction about using the provided programm with an ESP8266. In this example the ESP8266 works as
a wifi hotspot you can connect with and browse a website which allows different settings for the light stribe. The website is
quite simple and only a few effects and colors are supported. If you enter the button "DO IT" your configuration is transmitted over the serial interface
to the AVR. This is done through a software serial implementation, you find all necessary files below. You should step through all instructions to get
the example work.

\subsection setup_esp ESP8266 setup
First you need to setup the ESP8266. Because of different versions of ESP8266 modules you may miss something, this is just a quick guide. For more
information you can browse the web. First you must connect your ESP8266 to a host computer over a serial interface for example using an FTDI. Remember
to cross RX and TX of the serial port. Furthermore be aware of the ESP8266 voltage, it is 3,3 V. The current a serial chip may provide (some FTDIs provide
some current) may not be enough for the ESP8266 and what can cause different problems. <br>
So first you need to flash your ESP8266 with the nodemcu firmware that provides a software serial. The binaries that have been used in this example
can be found here: <br>
<a href="0x00000.bin" target="_blank"><b>Binaries part 1</b></a>		<br>
<a href="0x10000.bin" target="_blank"><b>Binaries part 2</b></a>		<br>
For uploading this binaries to the ESP8266 you should use the <a href="https://github.com/nodemcu/nodemcu-flasher">nodeMCUFlasher</a>  that can be found on github.
You need to set the serial port to which your ESP8266 is connected with and configure the source files for flashing the firmware. You need to set the COM port to 
which you ESP8266 is connected to (see \ref four "figure 4"). Furthermore you must consider the following hardware configuration:
<ul>
<li>3,3 V logic level
<li>bootmode low (IO15)
<li>chip enable high (CH_PD)
<li>reset high (drive low to reset the module)
<li>IO0 low for firmware flashing (high for programming and normal operation)
</ul>
The firmware programmer waits for the MAC of the ESP8266 module which will be successfully read if everything is done fine. As you
can see in \ref four "figure 4" the firmware programmer is still waiting for an ESP8266.
\anchor four
\image html   NodeMCUFlasher_flash.PNG "Figure 4: node MCU flasher"
\image latex  NodeMCUFlasher_flash.PNG "serial protocol structure"
\image rtf   NodeMCUFlasher_flash.PNG "serial protocol structure"
If the ESP8266 is connected right you now set the configuration to the provided binaries as you can see in \ref five "figure 5". You must browse to the binary
files and set the destination address. Now you can hit the "Flash"-Button (see \ref four "figure 4").
\anchor five
\image html   NodeMCUFlasher_config.PNG "Figure 5: node MCU flasher configuration"
\image latex  NodeMCUFlasher_config.PNG "serial protocol structure"
\image rtf   NodeMCUFlasher_config.PNG "serial protocol structure"
After flashing the firmware you need to reboot the ESP8266 module. Before you do this you should change the IO0 to high level (3V3) because after the reboot
we want to program the module with our own program. The reset can be done by setting reset low. The program we will upload to the module is written in Lua.
Lua is a scripting language that is interpreted by the firmware running on the module. So the performance is not the best, but the programming is quite simple.
The little program that you can find below set up the module as an access point, runs a simple webserver that interacts with a software serial to control the AVR.
One thing you must know about the Lua programming is that the variable types are assigned implicit so you cannot control whether a number is stored as 16 bit or
32 bit signed or unsigned variable. Another thing you should know is that the program you write needs the full memory space of its file. That means shorter variable
names save memory and furthermore documentation should be as short as possible or left.<br>
For writing your program you can use any text editor, notepad++ is a good choice. If you are finished you must upload the file to the module. Therefore you
use the same setup as for firmware updating but you must set IO0 to high level. For uploading your program you can use the ESP8266 Lua Loader. It is easy to handle
and you can try out several things first, before you upload your code. You can find the main window of the ESP8266 Lua Loader in \ref six "figure 6".
\anchor six
\image html   LuaLoader.png "Figure 6: ESP8266 Lua Loader"
\image latex  LuaLoader.png "ESP8266 Lua Loader"
\image rtf   LuaLoader.png "ESP8266 Lua Loader"
On the right side you can set the baud rate for uploading you program to the module. In the GPIO section you can easily set and reset them to try your wiring. 
By using the restart button you will restart the module. This may be necessary if your heap (RAM) is to low. This is caused by inefficient programming or by a 
program that is to big for the module. Global variables need a lot of heap. For uploading your program hit the "Upload File..." button. In a file browser you
choose your program that should be transferred to the module. After completion you hit the "dofile" button to run the program. This short description should be 
enough.<br>
So now we upload the Lua program that starts the webserver and sends data over a software serial to the AVR. You can find this program here:<br>
<a href="complex_server.lua" target="_blank"><b>lua program for controlling the AVR</b></a>		<br>
The program does the following:
  <ul>
  <li>set up the ESP8266 module as an access point (SSID=Lichterkette, password=12345678, you may change this)
  <li>start a webserver that listens on port 80
  <li>load the index.html website and handle requests
  <li>sent UART commands matching for the AVR implementation to generate different effects depending on the request
  <li>software serial is set to GPIO5 (GPIO5 is available at software number 1)
  </ul>
Some further things you should know:
  <ul>
  <li>the maximum of parallel accessing devices is four
  <li>parallel devices can never access another device
  <li>the software uart only supports TX (8N1 up to 38400 baud)
  <li>if you change the website you must change the hard coded content length of the website
  <li>the address of the ESP8266 is always 192.168.4.1
  </ul>
  
\subsection avr_con_esp Connect ESP8266 with AVR
After uploading the main program file you need to upload the <a href="_index.html" target="_blank"><b>_index.html</b></a> file. Before uploading remove the
underscore so that the files name is index.html (the underscore has been inserted because of conflicts with this html documentation). For uploading other
file types (than lua programs) to your ESP8266 module you need to use the "Upload Bin" button of the Lua Loader. The file will be uploaded to the file system
on the ESP8266. <br>
Now your ESP8266 module is ready to try the first communication with the AVR. So now you need a hardware setup where the ESP8266 and AVR are connected.
Be aware of the different voltage levels (AVR uses 5V, ESP8266 3V3). You should connect everything like \ref seven "figure 7" shows.

\anchor seven
\image html   Ws2811_Atmega328.png "Figure 7: schematic using ESP8266 with AVR WS2811 software"
\image latex  Ws2811_Atmega328.png "schematic using ESP8266 with AVR WS2811 software"
\image rtf   Ws2811_Atmega328.png "schematic using ESP8266 with AVR WS2811 software"

Your ESP8266 should still be connected with the host computer. If your hardware setup is ready you now must hit the "dofile" button in the Lua Loader 
(complexe_server.lua must be the selected file that should be executed). After a short time you can use any device to search for the access point that is set up
by the ESP8266 module. It will have your SSID (default "Lichterkette") and your chosen password (default "12345678"). After you connected to your ESP8266 module
you should open a web browser and browse the IP address 192.168.4.1. The browser should load the website. Now choose your configuration for the LEDs and hit the
"DO IT" Button on the website. The website should be reloaded and the lightstribes get the configuration you have chosen. If everything is working fine the last
thing to do is to make the ESP8266 as a stand alone device without the need of an external host. For this you must remove the complex_webserver.lua file from
the module. Now you rename the file on your host computer to init.lua. Afterwards you upload this file to the ESP8266. This file is always loaded at first when
the ESP8266 is powered on. So now you have a stand alone webserver that communicates with your AVR for controlling WS2811/WS2812 LEDs.<br>

\subsection short_setup Short setup
  <ul>
  <li>flash the provided image (<a href="0x00000.bin" target="_blank"><b>Binaries part 1</b></a>,<a href="0x10000.bin" target="_blank"><b>Binaries part 2</b></a>
  <li>upload the <a href="_index.html" target="_blank"><b>_index.html</b></a> renamed to index.html
  <li>upload the lua program <a href="complex_server.lua" target="_blank"><b>complex_server.lua</b></a> renamed to init.lua
  <li>setup your hardware refering to \ref seven "figure 7"
  <li>connect to your ESP8266 with your SSID and your password (default: Lichterkette, 12345678)
  <li>browse 192.168.4.1 on your device
  <li>set up your configuration and hit "DO IT"
  </ul>
  <br>
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
/** \brief Baudrate definition, choose 76800 or 38400, faster value preferred, the maximum speed of ESP8266 software-UART is 38400*/
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
/** \brief field position for the number of LEDs to control*/
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
	 
	 UCSR0B |= (1 << RXEN0) ;//| (1 << TXEN0);		// Enable receiver (and transmitter; commited out)
	 UCSR0B |= (1 << RXCIE0);						// Enable the receiver interrupt
	 UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);		// 8 data Bit, one stop Bit
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
				case FADE:
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
				//The custom effect assigns up to MAXNUMCOLORS individual colors to the stribe
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