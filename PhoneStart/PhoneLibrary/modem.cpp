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

//~~~~~~~~~~~~~~~~~~~~~~~~Variables~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
char responseList[][16] = {"OK\r\n", "ERROR\r\n", "+CME ERROR\r\n", "+CMS ERROR\r\n"};

char overflow[MODEM_MAX_COMMAND_SIZE];	//not too sure on the max size of an output line, but this seems like a decent guess.

//~~~~~~~~~~~~~~~~~~~static function prototypes~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//~~~~~~~~~~~~~~~~~~~static functions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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



//~~~~~~~~~~~~~~~~~~~~~~~functions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//function createmodem
//	creates modem object
//input:
//	rxpin:		integer value for a recieve pin
//	txpin:		integer value for a transmit pin
//output:
//	pmodem object

pModem createModem(int RXPin, int TXPin)
{
	pModem myModem = (pModem)malloc(sizeof(modem));
	if (myModem == NULL) {
		return NULL;
	}

	myModem->mdmComObj = createModemCommunicationsObj(RXPin, TXPin);
	PRINTS("Modem Created\n", STARTUP);
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


//function sendModemData
//	This function is intended to send data to the modem
//Input:
//	myModem		A referance to the Modem object to send command to 
//	data		A character string to send to modem
//
//Returns:
//	Integer:	Error or success.
int sendModemData(pModem myModem, char* data)
{
	modemWrite(myModem->mdmComObj, data);
}

//function readModemMessage
//	This function is intended to read Messages from the modem
//Input:
//	myModem:			The modem object to expect response
//	messageDataStorage:	A pointer to memory to store the Modem data. Memory must already be allocated
//Returns:
//	result code:	Error or success.
resultCode readModemMessage(pModem myModem, char* messageDataStorage, int storageSize)
{
	char* temp;
	resultCode result = AT_NOT_RESULT;
	int charsApnd = 0;

	//read until either storage is full or response code is read.
	while (true)
	{
		temp = modemReadLine(myModem->mdmComObj);
		result = checkForResultCode(temp);
		//if we have a result, return
		if (result != AT_NOT_RESULT)
		{
			break;
		}
		//else check to see if there is space to append to data storage
		if ((strlen(temp) + charsApnd) > storageSize)
		{
			result = AT_INPUT_OVERFLOW;
			//What to do with last string read? hmmm... store in a static buffer for now I suppose
			strcpy(overflow, temp);
			PRINTS("Modem Read Buffer Overflow", ERROR);
			break;
		}
		//Append data
		strcpy(messageDataStorage + charsApnd, temp);
		charsApnd += strlen(temp);
	}
	return result;
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

//function flushModemOutput
//	clears all serial data from the modem
//Input:
//	myModem:		the modem object to check for data
void flushModemOutput(pModem myModem)
{
	modemReadAllData(myModem->mdmComObj);
}

#endif