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

#ifndef DEBUG_C
#define DEBUG_C

#include "phone_debug.h"

debugLevel currentLevel = STARTUP;


void setDebugLevel(debugLevel level)
{
	currentLevel = level;
}

void PRINTS(char *s, debugLevel level) 
{ 
#if DEBUG
	if (level >= currentLevel)
	{
		Serial.print(F(s));
	}
#endif
}

void PRINT(char *s, char *v, debugLevel level)
{ 
#if DEBUG
	if (level >= currentLevel)
	{
		Serial.print(F(s));
		Serial.print(v);
	}
#endif
}

void PRINTX(char* s, char* v, debugLevel level)
{
#if DEBUG
	if (level >= currentLevel)
	{
		Serial.print(F(s));
		Serial.print(F("0x"));
		Serial.print((int)v, HEX);
	}
#endif
}



#endif