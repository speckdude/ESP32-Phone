/*Copyright(c) 2021 speckdude
///Permission is hereby granted, free of charge, to any person obtaining a copy
///of this softwareand associated documentation files(the "Software"), to deal
///in the Software without restriction, including without limitation the rights
///to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
///copies of the Software, and to permit persons to whom the Software is
///furnished to do so, subject to the following conditions :
///
///The above copyright noticeand this permission notice shall be included in all
///copies or substantial portions of the Software.
///
///THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
///AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///SOFTWARE.
*/

// handles all debug printouts

#ifndef DEBUG_H
#define DEBUG_H

//includes
#include <stream.h>
#include <HardwareSerial.h>
#include "constants.h"


//defines
//check constants file for debug value

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ENUMS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
enum debugLevel {
	LOGGING,
	STARTUP,
	ERROR,
	TESTING
};

//~~~~~~~~~~~~~~~~~~~~~~~~~type definitions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//~~~~~~~~~~~~~~~~~~~~~~~function prototypes~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setDebugLevel(debugLevel level);

void PRINTS(char *s, debugLevel level);
void PRINT(char *s, char *v, debugLevel level);
void PRINTX(char *s, char *v, debugLevel level);

#endif