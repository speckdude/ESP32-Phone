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

/*This header contains hardware level modem Communications handling for esp32 cellphone project
*This file will require changes depending on compiler
*
*
*
*
*/

#ifndef MODEMCOMMUNICATIONS_H
#define MODEMCOMMUNICATIONS_H

//includes
#include <hardwareSerial.h>	//for serial support
#include <stream.h>
#include <phone_debug.h> //for debug support
#include <constants.h>	//for global constants



//type definitions
typedef struct modemCommunicationsObj {
	HardwareSerial *serial;
	char inBuffer[MODEM_IN_BUFFER_SIZE];	//should I have this buffer for intermediate storage? currently to store data until whole response recieved.
}ModemCommunicationsObj;

typedef ModemCommunicationsObj *pModemCommunicationsObj;

//functions

//create references to Modem Communications
pModemCommunicationsObj createModemCommunicationsObj(int RXPin, int TXPin);
void destroyModemCommunicationsObj(pModemCommunicationsObj myMdmComObj);

//send Modem Communications
int modemWrite(pModemCommunicationsObj myMdmComObj, char *msg);
//check modem
int modemDataReady(pModemCommunicationsObj myMdmComObj);
//read data from Modem
char* modemReadLine(pModemCommunicationsObj myMdmComObj);
char* modemReadAllData(pModemCommunicationsObj myMdmComObj);

#endif