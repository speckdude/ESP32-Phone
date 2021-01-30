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

/*This header contains text message handling for esp32 cellphone project
 * 
 * 
 * 
 * 
 * 
 */


//includes
//#include <hardwareSerial.h>	//for serial support
//#include <stream.h>			//for serial support
#include <phone_debug.h> //for debug support
#include <modem.h> //for modem handleing

//~~~~~~~~~~~~~~~~~~~~~~~~defines & enums~~~~~~~~~~~~~~~~~~~~~~~
enum PDUMode { //defined in modem now
	PDU_TEXT_MODE = 0,
	ASCII_TEXT_MODE
};

#define LEAVE_BLANK -1

enum CNMI_MODE { //see https://drive.google.com/file/d/1bJ0kEDtORWAzH9LWy8Kefj69SbpKb2wh/view?usp=sharing PG 169 for definition
	MODE_LEAVE_BLANK = LEAVE_BLANK, //this will leave the whole message blank
	BUFFER_ALWAYS = 0,
	DISCARD_IF_BUSY,
	BUFFER_IF_BUSY
};

enum CNMI_MT { //see https://drive.google.com/file/d/1bJ0kEDtORWAzH9LWy8Kefj69SbpKb2wh/view?usp=sharing PG 170 for definition
	MT_LEAVE_BLANK = LEAVE_BLANK,
	NO_DELIVER_INDICATIONS = 0,
	INDICATE_LOCATION,
	SEND_UNSOLICITED,
	SEND_CLS_3_UNSOLICITED
};

enum CNMI_BM {	//see https://drive.google.com/file/d/1bJ0kEDtORWAzH9LWy8Kefj69SbpKb2wh/view?usp=sharing PG 170 for definition
	BM_LEAVE_BLANK = LEAVE_BLANK,
	NO_CBM_INDICATIONS = 0,
	NO_CBM = 0,
	NEW_CBM_UNSOLICITED = 2
};

enum CNMI_DS {	//see https://drive.google.com/file/d/1bJ0kEDtORWAzH9LWy8Kefj69SbpKb2wh/view?usp=sharing PG 170 for definition
	DS_LEAVE_BLANK = LEAVE_BLANK,
	NO_STATUS_REPORTS = 0,
	SMS_STATUS_DIRECT_UNSOLICITED,
	SMS_STATUS_MEMORY_UNSOLICITED
};

enum CNMI_BFR {	//see https://drive.google.com/file/d/1bJ0kEDtORWAzH9LWy8Kefj69SbpKb2wh/view?usp=sharing PG 170 for definition
	BFR_LEAVE_BLANK = LEAVE_BLANK,
	BFR_FLISH = 0,
	BFR_CLEAR
};

//~~~~~~~~~~~~~~~~~~~~~~~~function prototypes~~~~~~~~~~~~~~~~~~~
//text message modes
int configureTextMessaging(pModem myModem);
int setPDUMode(pModem myModem, PDUMode mode);
int setCNMIMode(pModem myModem, CNMI_MODE cnmi_mode, CNMI_MT cnmi_mt, CNMI_BM cnmi_bm, CNMI_DS cnmi_ds, CNMI_BFR cnmi_bfr);

//send/receive
int sendASCIITextMessage(pModem myModem, char *phoneNumber, char *message);

int readAllUnreadMessages(pModem myModem);
int readAllReadMessages(pModem myModem);
//int readRecievedMessage(pModem myModem, int msgAddrs);