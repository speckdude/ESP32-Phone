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

//~~~~~~~~~~~~~~~~~~~~~~~~local Defines~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define LIST_ENTRY_SIZE 16
//~~~~~~~~~~~~~~~~~~~~~~~~Variables~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Modem myModem;
SemaphoreHandle_t modemDataAccessMutex;	//control access to last command string

char		ATResponseList[][LIST_ENTRY_SIZE] = {"OK\r\n", "ERROR\r\n", "> "};
CommandResultCode	AtResponseEnumList[]	  = { AT_OK,	 AT_ERROR, AT_WAITING_FOR_INPUT};

char				unsolicitedResponseList[][LIST_ENTRY_SIZE] = { "+CME ERROR: ", "+CMS ERROR: ", "RING" };
UnsolicitedDataType	unsolicitedResponseEnumList[]			   = {  CME_ERROR,       CMS_ERROR,     RING };


char overflow[MODEM_MAX_COMMAND_SIZE];	//not too sure on the max size of an output line, but this seems like a decent guess.
char lastCommandSent[MODEM_MAX_COMMAND_SIZE]; //to store the last data sent to the modem. Used to determine if incoming data is a response or unsolicited
//~~~~~~~~~~~~~~~~~~~static function prototypes~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static bool checkIfCommandResponse(char* input);
static void setLastCommandSent(char* data);

static MessageType			getMessageType(char* input);
static CommandResultCode	checkForResultCode(char* input);
static UnsolicitedDataType	getUnsolicitedResponseType(char* input);
//~~~~~~~~~~~~~~~~~~~static functions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static MessageType getMessageType(char* input)
{
	//check if empty string
	if (strcmp(input, "\r\n") == 0) return EMPTY_MESSAGE;
	if (checkIfCommandResponse(input)) return COMMAND_RESPONSE;
	return UNSOLICITED_DATA;
}

static CommandResultCode checkForResultCode(char *input)
{
	for (int i = 0; i < (sizeof(ATResponseList) / LIST_ENTRY_SIZE); i++)
	{
		if (strncmp(input, ATResponseList[i], strlen(ATResponseList[i])) == 0)
		{
			return AtResponseEnumList[i];
		}
	}
	return AT_NOT_RESULT;
}

static UnsolicitedDataType getUnsolicitedResponseType(char* input)
{
	for (int i = 0; i < (sizeof(unsolicitedResponseList)/LIST_ENTRY_SIZE); i++)
	{
		if (strncmp(input, unsolicitedResponseList[i], strlen(unsolicitedResponseList[i])) == 0)
		{
			return unsolicitedResponseEnumList[i];
		}
	}
	return UNKNOWN_UNSOLICITED_DATA;
}

static bool checkIfCommandResponse(char* input)
{
	bool isResponse = false;
	//reserve mutex
	xSemaphoreTake(modemDataAccessMutex, portMAX_DELAY);
	//check if string is same as lastDataSent
	if(lastCommandSent[0] != '\0' && 
		strncmp(lastCommandSent, input, strlen(lastCommandSent) - 2) == 0) 
	{ 
		isResponse = true; 
	}
	//release mutex
	xSemaphoreGive(modemDataAccessMutex);

	return isResponse;
}

static void setLastCommandSent(char* data)
{
	//take mutex, and set data
	xSemaphoreTake(modemDataAccessMutex, portMAX_DELAY);
	strcpy(lastCommandSent, data);
	//release mutex
	xSemaphoreGive(modemDataAccessMutex);
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
	myModem.mdmComObj = createModemCommunicationsObj(RXPin, TXPin);
	modemDataAccessMutex = xSemaphoreCreateMutex();
	setLastCommandSent("");
	PRINTS("Modem Created\n", STARTUP);
	return &myModem;
}


//function destroyModem
//	destroys myModem object
//Input:
//	myModem:		The modem Object to be destroyed
//Output:
//	None
//
void destroyModem()
{
	destroyModemCommunicationsObj(myModem.mdmComObj);
}


//function sendModemData
//	This function is intended to send data to the modem
//Input:
//	myModem		A referance to the Modem object to send command to 
//	data		A character string to send to modem
//
//Returns:
//	Integer:	Error or success.
int sendModemData(char* data, bool isCommand)
{
	if(isCommand) setLastCommandSent(data);
	modemWrite(myModem.mdmComObj, data);
}

//function readModemMessage
//	This function is intended to read Messages from the modem
//Input:
//	messageDataStorage:	A pointer to memory to store the Modem data. Memory must already be allocated
//	storageSize:		size of the storage allocated
//Returns:
//	result code:	Error or success.
ModemMessage readModemMessage(char* messageDataStorage, int storageSize)
{
	char* temp;
	ModemMessage messageInfo;
	int charsApnd = 0;
	//clear old data out
	strcpy(messageDataStorage, "\0");
	//read first line, so we can figure out type of data
	temp = modemReadLine(myModem.mdmComObj);

	switch(messageInfo.type = getMessageType(temp))
	{
		case EMPTY_MESSAGE:
			break;

		case COMMAND_RESPONSE:
			//read until either storage is full or response code is read.
			while (true)
			{
				temp = modemReadLine(myModem.mdmComObj);
				messageInfo.result.commandResult = checkForResultCode(temp);
				//if we have a result, return
				if (messageInfo.result.commandResult != AT_NOT_RESULT)
				{
					break;
				}
				//else check to see if there is space to append to data storage
				if ((strlen(temp) + charsApnd) > storageSize)
				{
					messageInfo.result.commandResult = AT_BUFFER_OVERFLOW;
					//What to do with last string read? hmmm... store in a static buffer for now I suppose
					strcpy(overflow, temp);
					PRINTS("Modem Read Buffer Overflow\n", ERROR);
					break;
				}
				//Append data
				strcpy(messageDataStorage + charsApnd, temp);
				charsApnd += strlen(temp);
			}
			break;

		case UNSOLICITED_DATA:
			messageInfo.result.unsolicited = getUnsolicitedResponseType(temp);
			if (messageInfo.result.unsolicited == UNKNOWN_UNSOLICITED_DATA)
			{
				messageInfo.type = UNKNOWN_MESSAGE;
			}

			strcpy(messageDataStorage + charsApnd, temp);
			break;

		default:
			messageInfo.type = UNKNOWN_MESSAGE;
			strcpy(messageDataStorage + charsApnd, temp);
			break;
	}

	return messageInfo;
}

//function continueReadModemMessage
//	This function is intended to read Messages from the modem
//Input:
//	myModem:			The modem object to expect response
//	messageDataStorage:	A pointer to memory to store the Modem data. Memory must already be allocated
//Returns:
//	result code:	Error or success.
CommandResultCode continueReadModemMessage(char* messageDataStorage, int storageSize)
{
	char* temp;
	int charsApnd = 0;
	CommandResultCode result;

	//copy the overflowArray first
	strcpy(messageDataStorage, overflow);
	charsApnd = strlen(overflow);

	while (true)
	{
		temp = modemReadLine(myModem.mdmComObj);
		result = checkForResultCode(temp);
		//if we have a result, return
		if (result != AT_NOT_RESULT)
		{
			break;
		}
		//else check to see if there is space to append to data storage
		if ((strlen(temp) + charsApnd) > storageSize)
		{
			result = AT_BUFFER_OVERFLOW;
			//What to do with last string read? hmmm... store in a static buffer for now I suppose
			strcpy(overflow, temp);
			PRINTS("Modem Read Buffer Overflow\n", ERROR);
			break;
		}
		//Append data
		strcpy(messageDataStorage + charsApnd, temp);
		charsApnd += strlen(temp);
	}

	return result;
}

//function checkModem
//	checks for serial data from the modem
//Input:
//	myModem:		the modem object to check for data
//
//Returns:
//	Integer:	number of bytes waiting in buffer
int checkModem()
{
	return modemDataReady(myModem.mdmComObj);
}

//function flushModemOutput
//	clears all serial data from the modem
//Input:
//	myModem:		the modem object to check for data
void flushModemOutput()
{
	modemReadAllData(myModem.mdmComObj);
}

#endif