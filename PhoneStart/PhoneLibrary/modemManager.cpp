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
//modemCommand data struct
typedef struct modemCommand {
	ATCommand command;
	char modemResponseBuffer[MODEM_IN_BUFFER_SIZE];
}ModemCommand;
typedef ModemCommand* pModemCommand;

typedef short modemCommandLoc;

//~~~~~~~~~~~~~~~~~~~~~~~~Variables~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//modem
pModem myModem;
//thread handles
TaskHandle_t modemReaderManager;
TaskHandle_t modemWriterManager;

//Queue Handles
QueueHandle_t availableModemCommands;	//FIFO queue for available command space
QueueHandle_t commandWriteQueue;		//FIFO queue of commands to be written to modem
QueueHandle_t sentCommand;				//Mailbox Queue of last sent command. Used to guaruntee only one command sent at a time

//buffer
modemCommand commandArray[QUEUE_SIZE];	//Allocated space for our modemCommands

//~~~~~~~~~~~~~~~~~~~static function declarations~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void handleIncomingModemData();
static void handleOutgoingModemData(modemCommandLoc mdmCmd);
static void runModemReader(void* myModemInfo);
static void runModemWriter(void* info);
//~~~~~~~~~~~~~~~~~~~static functions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//function runModemResponse
// thread for modem response handleing (thread)
//Input:
// Void* info (this is currently null)
static void runModemReader(void* info)
{
	PRINTS("Modem Response Thread Started\n");
	//deal with unsolicited modem communications
	while (true)
	{
		if (checkModem(myModem))
		{
			handleIncomingModemData();
		}
		vTaskDelay(10);
	}
}

//function runModemOutgoing
// thread for sending modem commands
//Input:
// Void* info (this is currently null)
static void runModemWriter(void* info)
{
	PRINTS("Modem Outgoing Thread Started\n");
	modemCommandLoc currentCommand;
	while (true)
	{
		xQueueReceive(commandWriteQueue, &currentCommand, portMAX_DELAY);
		if (currentCommand == NULL)
		{
			PRINTS("recieve command is null\n");
		}
		handleOutgoingModemData(currentCommand);
		//vTaskDelay(10);
	}
}

//function handleIncomingModemData
//	handles incomming modem communications
//Input:
//	None
static void handleIncomingModemData()
{
	//todo, this works for now
	while (checkModem(myModem))
	{
		modemReadLine(myModem->mdmComObj);
	}
}

//function handleOutgoingModemData
//	handles Outgoing modem communications
//Input:
//	None
static void handleOutgoingModemData(modemCommandLoc mdmCmd)
{
	modemCommandLoc failedCommandLoc;
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

	sendATCommand(myModem, commandArray[mdmCmd].command);
}

//~~~~~~~~~~~~~~~~~~~~~~~functions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//function setupModemManager
//	setup function for modem manager
//Input:
// Pins to use for modem serial.
void setupModemManager(int RXPin, int TXPin)
{
	//create atomic queues
	availableModemCommands = xQueueCreate(QUEUE_SIZE, sizeof(modemCommandLoc));
	commandWriteQueue = xQueueCreate(QUEUE_SIZE, sizeof(modemCommandLoc));
	sentCommand = xQueueCreate(1, sizeof(modemCommandLoc));

	//populate available modem commands array
	for (short i = 0; i < QUEUE_SIZE; i++)
	{
		xQueueSend(availableModemCommands, &i, 0);
	}

	myModem = createModem(RXPin, TXPin);
	PRINTS("My Modem Created Sucessfully\n");
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
//	command:	pointer to a character array
//	arguments:	pointer to a character array containing modem command arguments
//	maxTime:	max time to wait to Enqueue a command. 0 means return immediatly if queue already full
//returns:
//	result, either success or timeout
modemQueueResult  sendModemCommand(char* command, char* arguments, int timeout)
{
	modemCommandLoc arrayLoc;
	//request a ModemCommand
	BaseType_t res = xQueueReceive(availableModemCommands, &arrayLoc, pdMS_TO_TICKS(timeout));
	PRINTS("modem queue recieved \n");
	//if timeout
	if (res == pdPASS)
	{
		//if we have a queue position, settup and send command
		commandArray[arrayLoc].command.command = command;
		commandArray[arrayLoc].command.args = arguments;
		return enqueueCommand(arrayLoc, 0);
	}

	return modemQueueTimeout;
}

//function enqueueModemCommand
//	Queues modemCommand to be sent
//Input:
//	commandArrayLoc: Location of the command to be sent
//	maxTime:	max time to wait to Enqueue a command. 0 means return immediatly if queue already full
//returns:
//	result, either success or timeout
modemQueueResult enqueueCommand(modemCommandLoc commandArrayLoc, int timeout)
{
	BaseType_t res = xQueueSend(commandWriteQueue, &commandArrayLoc, pdMS_TO_TICKS(timeout));
	// if for some reason res fails (should NOT happen at this point)
	if (res == errQUEUE_FULL)
	{
		PRINTS("ERROR: modemCommand location reserved but unable to be sent\n");
		xQueueSend(availableModemCommands, &commandArrayLoc, 0);
	}

	return ((res == pdPASS) ? modemQueueSuccess : modemQueueTimeout);
}



#endif