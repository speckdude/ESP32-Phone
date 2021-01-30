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
#include <modemCommunications.h>
#include <string.h>
#include <phone_debug.h>	//for debug suppport
#include <constants.h>

//enums
enum resultCode
{
	AT_NOT_RESULT=-1,
	AT_OK,
	AT_CONNECT,
	AT_RING,
	AT_NO_CARRIER,
	AT_ERROR,
	AT_NO_DIALTONE,
	AT_BUSY,
	AT_NO_ANSWER
};


//~~~~~~~~~~~~~~~~~~~~~~~~~type definitions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct modem {					//modem object
	pModemCommunicationsObj mdmComObj; //pointer to modem communications object
} Modem;
typedef Modem *pModem;

typedef struct ATCommand {
	char *command;
	char *args;
}ATCommand;


//~~~~~~~~~~~~~~~~~~~static function prototypes~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static resultCode checkForResultCode(char *responseStr);

//~~~~~~~~~~~~~~~~~~~~~~~function prototypes~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//create/destroy modem
pModem createModem(int RXPin, int TXPin);
void destroyModem(pModem myModem);

//~~~~~~~~~~modem functions~~~~~~~~~~~~~
//modem write
int sendATCommand(pModem myModem, ATCommand command);
int sendRawData(pModem myModem, char* data);

//modem read
resultCode getCommandResponse(pModem myModem);
int checkExpectedResponse(pModem myModem, char *response);

//misc
int checkModem(pModem myModem);
#endif