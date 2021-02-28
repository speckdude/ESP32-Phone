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
typedef struct modemData {
	char data[MODEM_MAX_COMMAND_SIZE];
	modemDataType dataType;
};
typedef modemData* pmodemData;

typedef short modemDataLoc;

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
modemData modemDataArray[QUEUE_SIZE];	//Allocated space for our modemData
char modemResponseBuffer[MODEM_IN_BUFFER_SIZE]; //Allocated space for data coming from the Modem

//~~~~~~~~~~~~~~~~~~~static function declarations~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void handleIncomingModemData();
static void handleOutgoingModemData(modemDataLoc mdmCmd);
static void runModemReader(void* myModemInfo);
static void runModemWriter(void* info);

static modemQueueResult enqueueCommand(modemDataLoc modemDataArrayLoc, int timeout);
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
	modemDataLoc currentCommand;
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
	modemDataLoc recievedCommandLoc;
	modemMessage result;

	//Todo, work in progress
	result = readModemMessage(modemResponseBuffer, MODEM_IN_BUFFER_SIZE);
	
	switch (result.type)	//handle the result
	{
		case COMMAND_RESPONSE:
			PRINT("Modem Command Response Recieved:\n", modemResponseBuffer, TESTING);
			//check if the message is a result of a command
			xQueueReceive(sentCommand, &recievedCommandLoc, 0);
			//todo, handle callback if one exists
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
static void handleOutgoingModemData(modemDataLoc mdmCmd)
{
	modemDataLoc failedCommandLoc;
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
	isCommand = (modemDataArray[mdmCmd].dataType == COMMAND_DATA) ? true : false;
	sendModemData(modemDataArray[mdmCmd].data, isCommand);
}

//~~~~~~~~~~~~~~~~~~~~~~~functions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//function setupModemManager
//	setup function for modem manager
//Input:
// Pins to use for modem serial.
void setupModemManager(int RXPin, int TXPin)
{
	//create atomic queues
	availablemodemData = xQueueCreate(QUEUE_SIZE, sizeof(modemDataLoc));
	commandWriteQueue = xQueueCreate(QUEUE_SIZE, sizeof(modemDataLoc));
	sentCommand = xQueueCreate(1, sizeof(modemDataLoc));

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
		10000,
		NULL,
		1,
		&modemWriterManager,
		0);
}

//function sendModemCommand
//	Requests and populates a ModemCommand, then queues it to be sent
//Input:
//	command:		pointer to a character array
//	timeout:		max time to wait to Enqueue a command. 0 means return immediatly if queue already full
//returns:
//	modemQueueResult enum
modemQueueResult sendModemCommand(char* data, int timeout,  modemDataType dataType)
{
	modemDataLoc arrayLoc;

	//check if command is longer than allowed length
	if (strlen(data) > MODEM_MAX_COMMAND_SIZE)
	{
		return MODEM_COMMAND_OVERSIZED;
	}

	//request a modemData
	BaseType_t res = xQueueReceive(availablemodemData, &arrayLoc, pdMS_TO_TICKS(timeout));
	//if not timed out
	if (res == pdPASS)
	{
		//if we have a queue position, settup and send command
		strcpy(modemDataArray[arrayLoc].data, data);
		modemDataArray[arrayLoc].dataType = dataType;

		return enqueueCommand(arrayLoc, 0);
	}

	return MODEM_QUEUE_TIMEOUT;
}

//function enqueuemodemData
//	Queues modemData to be sent
//Input:
//	modemDataArrayLoc: Location of the command to be sent
//	maxTime:	max time to wait to Enqueue a command. 0 means return immediatly if queue already full
//returns:
//	modemQueueResult enum
modemQueueResult enqueueCommand(modemDataLoc modemDataArrayLoc, int timeout)
{
	BaseType_t res = xQueueSend(commandWriteQueue, &modemDataArrayLoc, pdMS_TO_TICKS(timeout));
	// if for some reason res fails (should NOT happen at this point)
	if (res == errQUEUE_FULL)
	{
		PRINTS("ERROR: modemData location reserved but unable to be sent\n", ERROR);
		xQueueSend(availablemodemData, &modemDataArrayLoc, 0);
	}

	return ((res == pdPASS) ? MODEM_QUEUE_SUCESS : MODEM_QUEUE_TIMEOUT);
}



#endif