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

/*This header contains modem management for esp32 cellphone project
*
*
*
*
*
*/

#ifndef MODEMMANAGER_H
#define MODEMMANAGER_H

#include <FreeRTOS.h> //for queue and threads

#include <modem.h>
#include <phone_debug.h>
#include <constants.h>



#define QUEUE_SIZE 5

//~~~~~~~~~~~~~~~~~~~~~~~~~type definitions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
enum modemQueueResult 
{
	modemQueueTimeout = -1,
	modemQueueSuccess
};


//~~~~~~~~~~~~~~~~~~~~~~~~Variables~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
extern TaskHandle_t modemReadereManager;  //Not sure if other tasks will need these handles rn
extern TaskHandle_t modemWriterManager;

//~~~~~~~~~~~~~~~~~~~~~~~function prototypes~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setupModemManager(int RXPin, int TXPin);
modemQueueResult  sendModemCommand(char* command, char* arguments, int timeout);
modemQueueResult  enqueueCommand(short commandArrayLoc, int timeout);



#endif