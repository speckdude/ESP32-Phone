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

#include <textMessages.h>


//Function defintions

int configureTextMessaging(pModem myModem)
{
	setPDUMode(myModem, ASCII_TEXT_MODE);
	setCNMIMode(myModem, MODE_LEAVE_BLANK, MT_LEAVE_BLANK, BM_LEAVE_BLANK, DS_LEAVE_BLANK, BFR_LEAVE_BLANK);
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
int setCNMIMode(pModem myModem, CNMI_MODE cnmi_mode, CNMI_MT cnmi_mt, CNMI_BM cnmi_bm, CNMI_DS cnmi_ds, CNMI_BFR cnmi_bfr)
{
	bool cont = true;
	char tempBuf[8] = "";
	char command[16] = "";
	char args[16] = "";
	ATCommand myATCom = {
		.command = command,
		.args = args
	};
	//create command
	strcpy(command, "+CNMI");
	if(cnmi_mode == LEAVE_BLANK) //this sets to modem default values
	{
		cont = false;
	}
	else
	{
		strcpy(args, "=");
		strcat(args, itoa(cnmi_mode, tempBuf, 10));
	}
	if (cnmi_mt == LEAVE_BLANK && cont) //this sets to modem default values
	{
		cont = false;
	}
	else if (cont)
	{
		strcat(args, ",");
		strcat(args, itoa(cnmi_mt, tempBuf, 10));
	}
	if (cnmi_bm == LEAVE_BLANK && cont) //this sets to modem default values
	{
		cont = false;
	}
	else if (cont)
	{
		strcat(args, ",");
		strcat(args, itoa(cnmi_bm, tempBuf, 10));
	}
	if (cnmi_ds == LEAVE_BLANK && cont) //this sets to modem default values
	{
		cont = false;
	}
	else if (cont)
	{
		strcat(args, ",");
		strcat(args, itoa(cnmi_ds, tempBuf, 10));
	}
	if (cnmi_bfr == LEAVE_BLANK && cont) //this sets to modem default values
	{
		cont = false;
	}
	else if (cont)
	{
		strcat(args, ",");
		strcat(args, itoa(cnmi_bfr, tempBuf, 10));
	}
	//send command
	return sendATCommand(myModem, myATCom);
}

//function setPDUMode
//this function sets the Modem text message mode
//input:
//	pModem myModem:			Modem Object to set to text mode
//	textMessageMode mode:	Mode to set modem to
//
//returns:
//	integer:	error(-1) or success(1).
int setPDUMode(pModem myModem, PDUMode mode)
{
	char command[16] = "";
	char args[8] = "";
	ATCommand myATCom = {
		.command = command,
		.args = args
	};

	//send AT+CMGF=1 command, recieve response
	strcpy(command, "+CMGF");
	if(mode = PDU_TEXT_MODE)		strcpy(args, "=0");
	else if(mode = ASCII_TEXT_MODE)	strcpy(args, "=1");

	sendATCommand(myModem, myATCom);
	if (getCommandResponse(myModem) == AT_ERROR) //if error return
	{
		PRINTS("PDU Response ERROR\n");
		return -1;
	}
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

int sendASCIITextMessage(pModem myModem, char *phoneNumber, char *message)
{
	char command[16];
	char args[256];
	ATCommand myATCom = { 
		.command = command,//"+CMGF",
		.args = args//"=1" 
	};

	strcpy(command, "+CMGS");
	strcpy(args, "=\"");
	strcat(myATCom.args, phoneNumber);
	strcat(myATCom.args, "\"");

	sendATCommand(myModem, myATCom);
	//check for > indication
	while (checkModem(myModem) == 0)
	{
		//stall
	}
	if(checkExpectedResponse(myModem, ">") < 1)
	{
		PRINTS("Expected Response Not recieved\n");
		return -1;
	}
	sendRawData(myModem, message);
	sendRawData(myModem, "\x1A");	//to indicate end of message
	return 0;
}

int readAllUnreadMessages(pModem myModem)
{
	char command[16];
	char args[32];
	ATCommand myATCom = {
		.command = command,
		.args = args
	};

	strcpy(command, "+CMGL");
	strcpy(args, "=\"REC UNREAD\"");
	return sendATCommand(myModem, myATCom);
}

int readAllReadMessages(pModem myModem)
{
	char command[16];
	char args[32];
	ATCommand myATCom = {
		.command = command,
		.args = args
	};

	strcpy(command, "+CMGL");
	strcpy(args, "=\"REC READ\"");
	return sendATCommand(myModem, myATCom);
}
