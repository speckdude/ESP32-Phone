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

/*This header contains modem functions for esp32 cellphone project
*
*
*
*
*
*/

#ifndef MODEM_H
#define MODEM_H

//includes
#include <string.h>
#include <FreeRTOS.h> //mutex

#include "modemCommunications.h"
#include "modemManager.h"
#include "phone_debug.h"	//for debug suppport
#include "constants.h"

////~~~~~~~~~~~~~~~~~~~~~~~~enums~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
enum MessageType
{
	COMMAND_RESPONSE,
	UNSOLICITED_DATA,
	EMPTY_MESSAGE,
	UNKNOWN_MESSAGE
};

enum CommandResultCode
{
	AT_BUFFER_OVERFLOW = -2,
	AT_NOT_RESULT = -1,
	AT_OK,
	AT_WAITING_FOR_INPUT,
	AT_ERROR
};

enum UnsolicitedDataType
{
	RING,
	CDS_NOTIFICATION,
	CDSI_NOTIFICATION,
	CMT_NOTIFICATION,
	CMTI_NOTIFICATION,
	CME_ERROR,
	CMS_ERROR,
	UNKNOWN_UNSOLICITED_DATA
};

//~~~~~~~~~~~~~~~~~~~~~~~~~type definitions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct Modem {					//modem object
	pModemCommunicationsObj mdmComObj; //pointer to modem communications object
};
typedef Modem *pModem;

typedef struct ModemMessage {
	MessageType type;
	union Result
	{
		CommandResultCode commandResult;
		UnsolicitedDataType unsolicited;
	}result;
	char* recievedMessage;
};


//~~~~~~~~~~~~~~~~~~~~~~~function prototypes~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//create/destroy modem
pModem createModem(int RXPin, int TXPin);
void destroyModem();

//~~~~~~~~~~modem functions~~~~~~~~~~~~~
//modem write
int sendModemData(char* data, bool isCommand);

//modem read
ModemMessage readModemMessage(char* messageDataStorage, int storageSize);
CommandResultCode continueReadModemMessage(char* messageDataStorage, int storageSize);

//misc
void flushModemOutput();
int checkModem();
#endif