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

/*This body contains hardware level modem Communications handling for esp32 cellphone project
*
*
*
*
*
*/
#ifndef MODEMCOMMUNICATIONS_C
#define MODEMCOMMUNICATIONS_C


//includes
#include "modemCommunications.h"

//function definitions

//function: create Modem Communications Obj
//input:
//	pModemCommunicationsObj:	Pointer to a modem communications object
//	RXPin:						Integer to Recieve pin
//	TXPin:						Integer representing Transmit pin
//
//returns:
//Error(-1) or Success(0)

pModemCommunicationsObj createModemCommunicationsObj(int RXPin, int TXPin)
{
	pModemCommunicationsObj myMdmComObj = (pModemCommunicationsObj)malloc(sizeof(modemCommunicationsObj));
	if (myMdmComObj == NULL)
	{
		return NULL;
	}
	myMdmComObj->serial = &Serial1;
	myMdmComObj->serial->begin(MODEM_BAUD_RATE, SERIAL_8N1, RXPin, TXPin);

	return myMdmComObj;
}

//
//function: destroy Modem Communications Obj
//input:
//pModemCommunicationsObj:	Pointer to a modem communications object
//returns void

void destroyModemCommunicationsObj(pModemCommunicationsObj myMdmComObj)
{
	free(myMdmComObj);
}


//
//function: modem Write
//writes message to modem
//input:
//pModemCommunicationsObj:	Pointer to a modem communications object
//msg:						pointer to character array
//
//returns:
//number of characters written

int modemWrite(pModemCommunicationsObj myMdmComObj, char *msg)
{
	return myMdmComObj->serial->write(msg);
}

//
//function: modem Data Ready
//checks if data is waiting from modem.
//input:
//pModemCommunicationsObj:	Pointer to a modem communications object
//
//returns:
//0 if no data is ready, positive integer if data ready

int modemDataReady(pModemCommunicationsObj myMdmComObj)
{
	return myMdmComObj->serial->available();
}

//
//function: modem Read
//reads data from modem object if data is waiting
//input:
//pModemCommunicationsObj:	Pointer to a modem communications object
//
//returns:
//character pointer to recieved data
char* modemReadLine(pModemCommunicationsObj myMdmComObj)
{
	int numRead;
	numRead = myMdmComObj->serial->readBytesUntil('\n', myMdmComObj->inBuffer, (sizeof(myMdmComObj->inBuffer)-2));
	myMdmComObj->inBuffer[numRead] = '\n'; //replace terminator & ensure null character is at end of char array
	myMdmComObj->inBuffer[numRead+1] = '\0'; //ensure null character is at end of char array
	PRINT("line read: ", myMdmComObj->inBuffer, LOGGING);
	return myMdmComObj->inBuffer;
}

//function: modemReadAllData
//reads data from modem object if data is waiting
//input:
//pModemCommunicationsObj:	Pointer to a modem communications object
//
//returns:
//character pointer to recieved data
char* modemReadAllData(pModemCommunicationsObj myMdmComObj)
{
	int numRead;
	numRead = myMdmComObj->serial->available();
	myMdmComObj->serial->readBytes(myMdmComObj->inBuffer, numRead);
	myMdmComObj->inBuffer[numRead] = '\0'; //ensure null character is at end of char array
	PRINT("data read: ", myMdmComObj->inBuffer, LOGGING);
	return myMdmComObj->inBuffer;
}

#endif