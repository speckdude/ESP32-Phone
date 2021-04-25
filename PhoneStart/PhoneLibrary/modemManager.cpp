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


/*This Body contains modem Management for esp32 cellphone project
*
*
*
*
*
*/

#ifndef MODEMMANAGER_C
#define MODEMMANAGER_C

#include "modemManager.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~type definitions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//modemData data struct
typedef struct ModemData {
	char data[MODEM_MAX_COMMAND_SIZE];
	ModemDataType dataType;
	ModemCommandState dataState;
	TaskHandle_t xTaskToNotify;
	CommandResultCode commandResponse;
	char* modemResponse;
};
typedef ModemData* pModemData;

//~~~~~~~~~~~~~~~~~~~~~~~~Variables~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//modem
pModem pMyModem; //handle to our modem.

//thread handles
TaskHandle_t modemReaderManager;
TaskHandle_t modemWriterManager;

//Queue Handles
QueueHandle_t availablemodemData;	//FIFO queue for available command space
QueueHandle_t commandWriteQueue;		//FIFO queue of commands to be written to modem
QueueHandle_t sentCommand;				//Mailbox Queue of last sent command. Used to guaruntee only one command sent at a time

//buffers/Arrays
volatile ModemData modemDataArray[QUEUE_SIZE];	//Allocated space for our modemData
char modemResponseBuffer[MODEM_IN_BUFFER_SIZE]; //Allocated space for data coming from the Modem

//~~~~~~~~~~~~~~~~~~~static function declarations~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void handleIncomingModemData();
static void handleOutgoingModemData(ModemDataLoc mdmCmd);
static void runModemReader(void* myModemInfo);
static void runModemWriter(void* info);

static ModemQueueResult enqueueCommand(ModemDataLoc modemDataArrayLoc, int timeout);
//~~~~~~~~~~~~~~~~~~~static functions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//function runModemResponse
// thread for modem response handleing (thread)
//Input:
// Void* info (this is currently null)
static void runModemReader(void* info)
{
	PRINTS("Modem Response Thread Started\n", STARTUP);

	//deal with modem communications
	while (true)
	{
		if (checkModem())
		{
			handleIncomingModemData();
		}
		//todo, theres got to be a better way than just delays...
		vTaskDelay(10);
	}
}

//function runModemOutgoing
// thread for sending modem commands
//Input:
// Void* info (this is currently null)
static void runModemWriter(void* info)
{
	PRINTS("Modem Outgoing Thread Started\n", STARTUP);
	ModemDataLoc currentCommand;
	vTaskDelay(10000);
	while (true)
	{
		xQueueReceive(commandWriteQueue, &currentCommand, portMAX_DELAY);
		handleOutgoingModemData(currentCommand);
	}
}

//function handleIncomingModemData
//	handles incomming modem communications
//Input:
//	None
static void handleIncomingModemData()
{
	ModemDataLoc recievedCommandLoc;
	ModemMessage result;

	//Todo, work in progress
	result = readModemMessage(modemResponseBuffer, MODEM_IN_BUFFER_SIZE);
	
	switch (result.type)	//handle the result
	{
		case COMMAND_RESPONSE:
			//get last sent command location & update with recieved data
			xQueueReceive(sentCommand, &recievedCommandLoc, 0);

			//if we want the output data from the array
			if (modemDataArray[recievedCommandLoc].dataType == COMMAND_DATA_RESULT_WANTED)
			{
				modemDataArray[recievedCommandLoc].dataState = READING_RESPONSE;
				modemDataArray[recievedCommandLoc].modemResponse = strdup(modemResponseBuffer);
				while(result.result.commandResult == AT_BUFFER_OVERFLOW) //need to go back for more data
				{
					result.result.commandResult = continueReadModemMessage(modemResponseBuffer, MODEM_IN_BUFFER_SIZE);
					modemDataArray[recievedCommandLoc].modemResponse = (char*)realloc(modemDataArray[recievedCommandLoc].modemResponse, 
						sizeof(char) * (strlen(modemDataArray[recievedCommandLoc].modemResponse) + strlen(modemResponseBuffer) + 1));
					strcat(modemDataArray[recievedCommandLoc].modemResponse, modemResponseBuffer);
				}
				modemDataArray[recievedCommandLoc].commandResponse = result.result.commandResult;
				//PRINT("Modem Command Response Recieved:\n", modemDataArray[recievedCommandLoc].modemResponse, TESTING);
				modemDataArray[recievedCommandLoc].dataState = RESPONSE_READY;
				//notify task
				xTaskNotifyGive(modemDataArray[recievedCommandLoc].xTaskToNotify);
			}
			//otherwise, put command back into the array
			else
			{
				modemDataArray[recievedCommandLoc].dataState = COMMAND_EMPTY;
				xQueueSend(availablemodemData, &recievedCommandLoc, 0);
			}
			break;

		case UNSOLICITED_DATA:
			PRINT("Unsolicited Data Recieved:\n", modemResponseBuffer, TESTING);
			break;

		case EMPTY_MESSAGE:
			break;

		case UNKNOWN_MESSAGE:
			PRINT("Unknown Data Recieved:\n", modemResponseBuffer, TESTING);
			break;
	}

}

//function handleOutgoingModemData
//	handles Outgoing modem communications
//Input:
//	None
static void handleOutgoingModemData(ModemDataLoc mdmCmd)
{
	ModemDataLoc failedCommandLoc;
	bool isCommand;
	//first send command to sentCommand Mailbox (blocking for maximum response wait time)
	BaseType_t res = xQueueSend(sentCommand, &mdmCmd, pdMS_TO_TICKS(MODEM_RESPONSE_WAIT_TIME));
	//check to see if we timed out...This shows the previous command response was never recieved...
	if (res == errQUEUE_FULL)
	{
		//pop out the timedOutCommand, and notify the thread that sent it that it failed.
		//No need to wait because the only way to get here is if the Queue contains something
		xQueueReceive(sentCommand, &failedCommandLoc, 0);
		//todo: notify timed out command that no response was ever recieved

		//should need no wait since we just emptied queue
		xQueueSend(sentCommand, &mdmCmd, 0);
	}
	isCommand = (modemDataArray[mdmCmd].dataType != RAW_DATA) ? true : false;
	sendModemData((char*)modemDataArray[mdmCmd].data, isCommand);
	if (isCommand) //if its a command, change our state
	{
		modemDataArray[mdmCmd].dataState = COMMAND_SENT;
	}
	else //if its raw data, put the modemDataLoc back into the availablemodemData queue
	{
		modemDataArray[mdmCmd].dataState = COMMAND_EMPTY;
		xQueueSend(availablemodemData, &mdmCmd, 0);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~functions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//function setupModemManager
//	setup function for modem manager
//Input:
// Pins to use for modem serial.
void setupModemManager(int RXPin, int TXPin)
{
	//create atomic queues
	availablemodemData = xQueueCreate(QUEUE_SIZE, sizeof(ModemDataLoc));
	commandWriteQueue = xQueueCreate(QUEUE_SIZE, sizeof(ModemDataLoc));
	sentCommand = xQueueCreate(1, sizeof(ModemDataLoc));

	//populate available modem commands array
	for (short i = 0; i < QUEUE_SIZE; i++)
	{
		xQueueSend(availablemodemData, &i, 0);
	}

	pMyModem = createModem(RXPin, TXPin);
	PRINTS("My Modem Created Sucessfully\n", STARTUP);

	//start modem response thread
	xTaskCreatePinnedToCore(
		runModemReader,
		"modemReaderManager",
		10000,
		NULL,
		1,
		&modemReaderManager,
		0);
	//start modem sending thread
	xTaskCreatePinnedToCore(
		runModemWriter,
		"modemWriterManager",
		1000,
		NULL,
		1,
		&modemWriterManager,
		0);
}

//function sendModemCommand
//	Requests and populates a ModemCommand, then queues it to be sent
//Input:
//	data:			pointer to a character array
//	timeout:		max time to wait to Enqueue a command. 0 means return immediatly if queue already full
//	dataType:		Type of data to send (command, or raw)
//returns:
//	ModemQueueResult enum
ModemQueueResult sendModemCommand(char* data, int timeout,  ModemDataType dataType)
{
	Command command;
	command.modemDataType = dataType;
	command.data = data;
	command.timeout = timeout;
	return sendModemCommand(command, NULL);
}


ModemQueueResult sendModemCommand(Command command, ModemDataLoc* commandLoc)
{
	ModemDataLoc arrayLoc;
	ModemQueueResult queueResult = MODEM_QUEUE_TIMEOUT;

	//check if command is longer than allowed length
	if (strlen(command.data) > MODEM_MAX_COMMAND_SIZE)
	{
		queueResult = MODEM_COMMAND_OVERSIZED;
	}
	else {
		//request a modemData
		BaseType_t res = xQueueReceive(availablemodemData, &arrayLoc, pdMS_TO_TICKS(command.timeout));
		//if not timed out
		if (res == pdPASS)
		{
			//if we have a queue position, settup and send command
			strcpy((char*)modemDataArray[arrayLoc].data, command.data);
			modemDataArray[arrayLoc].dataType = command.modemDataType;
			modemDataArray[arrayLoc].xTaskToNotify = xTaskGetCurrentTaskHandle();
			queueResult = enqueueCommand(arrayLoc, 0);
		}
	}
	
	if (commandLoc != NULL)
	{
		*commandLoc = (queueResult == MODEM_QUEUE_SUCCESS) ? arrayLoc : -1;
	}

	return queueResult;
}

//function sendModemCommandGetResult
//	Requests and populates a ModemCommand, then queues it to be sent. Waits for response from Modem
//Input:
//	command:		A Command data struct containing all required info to send a command
//returns:
//	Command Result
CommandResult sendModemCommandGetResult(Command command)
{
	CommandResult result; 
	ModemDataLoc recievedCommandLoc;

	//send command
	if (sendModemCommand(command, &recievedCommandLoc) != MODEM_QUEUE_SUCCESS)
	{
		result.commandResult = result.SEND_ERROR;
		result.response = NULL;
		return result;
	}
	//wait for notification of response
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

	return getCommandResult(recievedCommandLoc);
}


//function enqueuemodemData
//	Queues modemData to be sent
//Input:
//	modemDataArrayLoc: Location of the command to be sent
//	maxTime:	max time to wait to Enqueue a command. 0 means return immediatly if queue already full
//returns:
//	ModemQueueResult enum
ModemQueueResult enqueueCommand(ModemDataLoc modemDataArrayLoc, int timeout)
{
	BaseType_t res = xQueueSend(commandWriteQueue, &modemDataArrayLoc, pdMS_TO_TICKS(timeout));
	// if for some reason res fails (should NOT happen at this point)
	if (res == errQUEUE_FULL)
	{
		PRINTS("ERROR: modemData location reserved but unable to be sent\n", ERROR);
		xQueueSend(availablemodemData, &modemDataArrayLoc, 0);
	}
	modemDataArray[modemDataArrayLoc].dataState = WAITING_TO_SEND;
	return ((res == pdPASS) ? MODEM_QUEUE_SUCCESS : MODEM_QUEUE_TIMEOUT);
}

//function checkModemCommandState
//	Checks the status of a command
//Input:
// locToCheck: the array index to get the command status from
//returns:
// dataState enum
ModemCommandState checkModemCommandState(ModemDataLoc locToCheck)
{
	return modemDataArray[locToCheck].dataState;
}

//function getCommandResult
//	reads a response from a modem command. After reading a response the location is added back
//  into the availablemodemData queue for next use
//Input:
//	modemDataArrayLoc: Location of the command to be sent
//returns:
//	CommandResult 
CommandResult getCommandResult(ModemDataLoc responseLoc)
{
	CommandResult commandResult;

	//create command result
	commandResult.commandResult = (modemDataArray[responseLoc].commandResponse == AT_OK) ? commandResult.RESULT_OK : commandResult.RESULT_ERROR;
	commandResult.response = strdup(modemDataArray[responseLoc].modemResponse);
	//clean up command
	free(modemDataArray[responseLoc].modemResponse);
	modemDataArray[responseLoc].dataState = COMMAND_EMPTY;

	//put it back into available commands
	xQueueSend(availablemodemData, &responseLoc, 0);

	return commandResult;
}

#endif