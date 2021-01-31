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

/*This body contains modem functions for esp32 cellphone project
*
*
*
*
*
*/

#ifndef MODEM_C
#define MODEM_C

//includes
#include "modem.h"

//static function defintions
static resultCode checkForResultCode(char *responseStr)
{
	if(strstr(responseStr, "OK\r\n") != NULL)
	{
		return AT_OK;
	}
	if (strstr(responseStr, "ERROR\r\n") != NULL)
	{
		return AT_ERROR;
	}

	return AT_NOT_RESULT;
}



//function definitions

//function createmodem
//	creates modem object
//input:
//	rxpin:		integer value for a recieve pin
//	txpin:		integer value for a transmit pin
//output:
//	pmodem object

pModem createModem(int RXPin, int TXPin)
{
	pModem myModem = (pModem)malloc(sizeof(Modem));
	if (myModem == NULL) {
		return NULL;
	}

	myModem->mdmComObj = createModemCommunicationsObj(RXPin, TXPin);
	PRINTS("Modem Created\n");
	//modemWrite(myModem->myMdmComObj, "This is a test 2");
	return myModem;
}


//function destroyModem
//	destroys myModem object
//Input:
//	myModem:		The modem Object to be destroyed
//Output:
//	None
//
void destroyModem(pModem myModem)
{
	destroyModemCommunicationsObj(myModem->mdmComObj);
	free(myModem);
}


//function sendAtCommand
//	This function is intended to send AT commands to the modem
//Input:
//	myModem		A referance to the Modem object to send command to 
//	ATCommand command:				The AT command to be sent
//
//Returns:
//	Integer:	Error or success.
int sendATCommand(pModem myModem, ATCommand command)
{
	char commandString[128] = "AT";
	if (command.command != NULL)
	{
		strcat(commandString, command.command);
	}
	if (command.args != NULL)
	{
		strcat(commandString, command.args);
	}
	strcat(commandString, "\r");
	modemWrite(myModem->mdmComObj, commandString);
	return 0;
}

//function sendRawData
//	This function is intended to send data to the modem
//Input:
//	myModem		A referance to the Modem object to send command to 
//	data		A character string to send to modem
//
//Returns:
//	Integer:	Error or success.
int sendRawData(pModem myModem, char* data)
{
	modemWrite(myModem->mdmComObj, data);
}

//function getCommandResponse
//	This function is intended to get response from an AT command
//Input:
//	myModem:	The modem object to expect response
//
//Returns:
//	Integer:	Error or success.
resultCode getCommandResponse(pModem myModem)
{
	char *response;
	resultCode Code = AT_NOT_RESULT;

	while (Code == AT_NOT_RESULT)
	{
		response = modemReadLine(myModem->mdmComObj);
		Code = checkForResultCode(response);
	}
	return Code;
}

//function checkExpectedResponse
//	This function is intended to check for a specific expected response
//Input:
//	myModem:	The modem object to expect response
//	response:	The string to expect
//Returns:
//	Integer:	Error(-1)if expected response not found or success(1) if found.
int checkExpectedResponse(pModem myModem, char *response)
{
	char *recievedData;

	if (checkModem(myModem))
	{
		recievedData = modemReadAllData(myModem->mdmComObj);
		if (strstr(recievedData, response) != NULL)
		{
			return 1;
		}
	}
	return -1;
}

//function checkModem
//	checks for serial data from the modem
//Input:
//	myModem:		the modem object to check for data
//
//Returns:
//	Integer:	number of bytes waiting in buffer
int checkModem(pModem myModem)
{
	return modemDataReady(myModem->mdmComObj);
}


#endif