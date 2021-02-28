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

/*This body contains text message handling for esp32 cellphone project
*
*
*
*
*
*/

#include "textMessages.h"


//Function defintions

int configureTextMessaging()
{
	setPDUMode(ASCII_TEXT_MODE);
	setCNMIMode(MODE_LEAVE_BLANK, MT_LEAVE_BLANK, BM_LEAVE_BLANK, DS_LEAVE_BLANK, BFR_LEAVE_BLANK);
}


//function setCNMIMode
//this function sets the Modem text message recieve notification modes
//input:
//	pModem myModem:			Modem Object to set to text mode
//	CNMI_MODE cnmi_mode 
//  CNMI_MT cnmi_mt, CNMI_BM cnmi_bm, 
//  CNMI_DS cnmi_ds, CNMI_BFR cnmi_bfr	: settings for modem
//
//returns:
//	integer:	error(-1) or success(1).
int setCNMIMode(CNMI_MODE cnmi_mode, CNMI_MT cnmi_mt, CNMI_BM cnmi_bm, CNMI_DS cnmi_ds, CNMI_BFR cnmi_bfr)
{
	bool cont = true;
	char command[128] = "";
	char tempBuf[8] = "";

	//create command
	strcpy(command, "AT+CNMI");
	if(cnmi_mode == LEAVE_BLANK) //this sets to modem default values
	{
		cont = false;
	}
	else
	{
		strcat(command, "=");
		strcat(command, itoa(cnmi_mode, tempBuf, 10));
	}
	if (cnmi_mt == LEAVE_BLANK && cont) //this sets to modem default values
	{
		cont = false;
	}
	else if (cont)
	{
		strcat(command, ",");
		strcat(command, itoa(cnmi_mt, tempBuf, 10));
	}
	if (cnmi_bm == LEAVE_BLANK && cont) //this sets to modem default values
	{
		cont = false;
	}
	else if (cont)
	{
		strcat(command, ",");
		strcat(command, itoa(cnmi_bm, tempBuf, 10));
	}
	if (cnmi_ds == LEAVE_BLANK && cont) //this sets to modem default values
	{
		cont = false;
	}
	else if (cont)
	{
		strcat(command, ",");
		strcat(command, itoa(cnmi_ds, tempBuf, 10));
	}
	if (cnmi_bfr == LEAVE_BLANK && cont) //this sets to modem default values
	{
		cont = false;
	}
	else if (cont)
	{
		strcat(command, ",");
		strcat(command, itoa(cnmi_bfr, tempBuf, 10));
	}
	strcat(command, "\r");
	//send command
	return sendModemCommand(command, 0, COMMAND_DATA);
}

//function setPDUMode
//this function sets the Modem text message mode
//input:
//	pModem myModem:			Modem Object to set to text mode
//	textMessageMode mode:	Mode to set modem to
//
//returns:
//	integer:	error(-1) or success(1).
int setPDUMode(PDUMode mode)
{
	char command[16] = "";

	//send AT+CMGF=1 command, recieve response
	strcpy(command, "AT+CMGF");
	if(mode = PDU_TEXT_MODE)		strcpy(command, "=0\r");
	else if(mode = ASCII_TEXT_MODE)	strcpy(command, "=1\r");

	sendModemCommand(command, 0, COMMAND_DATA);
	/*old code....not super relevant. Test case for callbacks?
	if (getCommandResponse(myModem) == AT_ERROR) //if error return
	{
		PRINTS("PDU Response ERROR\n", ERROR);
		return -1;
	}
	*/
	return 1;
}


//function sendtextmessage
//this function is intended to command the sim5320a to send a text message
//input:
//hardware serial refser:		a referance to the serial port connected to the sim5320a
//	char* phonenumber:			a phone number to send the message to
//	char* message:				the message to send
//
//returns:
//	integer:	error(-1) or success(1).

int sendASCIITextMessage(char *phoneNumber, char *message)
{
	char command[32];
	char tempMessage[strlen(message) + 1];
	modemQueueResult commandResult;

	strcpy(command, "AT+CMGS=\"");
	strcat(command, phoneNumber);
	strcat(command, "\" \r");

	strcpy(tempMessage, message);
	strcat(tempMessage, "\x1A");
	//todo, figure out PDU message encoding....
	//Workaround for now
	if (sendModemCommand(command, 1000, COMMAND_DATA) == MODEM_QUEUE_SUCESS)
	{
		//just *PRAY* to the Cell phone gods that the text data is entered into the command array one behind the modemCommand
		return sendModemCommand(tempMessage, 1000, RAW_DATA);
	}
	return MODEM_QUEUE_TIMEOUT;
}

int readAllUnreadMessages()
{
	char command[]= "AT+CMGL=\"REC UNREAD\"\r";
	return sendModemCommand(command, 0, COMMAND_DATA);
}

int readAllReadMessages()
{
	char command[]= "AT+CMGL=\"REC READ\"\r";
	return sendModemCommand(command, 0, COMMAND_DATA);
}
