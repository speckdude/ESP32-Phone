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


/*
  Starting code for esp32 Cellphone project. 
*/
#include <FreeRTOS.h>
#include <string.h>
#include <textMessages.h>
#include <modemManager.h>
#include <phone_debug.h>

#define RX 16
#define TX 17

int readCount = 0;
char input[256];


void setup() {
  // initialize both serial ports:
  Serial.begin(115200);
  setupModemManager(RX,TX);
  //configureTextMessaging(SIM);
  pinMode(LED_BUILTIN, OUTPUT);
}
  
void loop()
{
  // read from port 0, send to port 1:
  if (Serial.available()) 
  {
    Serial.readBytes(input, sizeof(input));
    sendModemCommand(NULL, NULL, 0);
    //parseInput(input);
    //sendTextMessage(SIM,"19515512292","This is a test");
  }
}

void changeLEDState()
{
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void parseInput(char *input)
{
  //check for all cases with if statements because case do not work
  if(strstr(input, "send text") != NULL)  sendTextRoutine();
  //if(strstr(input, "read unread") !=NULL) readAllUnreadMessages(SIM);
  //if(strstr(input, "read read") != NULL)  readAllReadMessages(SIM);
}

void sendTextRoutine()
{
  char phoneNumber[15];
  char message[161];
  int messageLen;
  Serial.print("enter phone Number\n");
  while(Serial.available() < 1)  { /*just wait for input. this is bad but whatevs, todo*/}
  messageLen = Serial.available();
  Serial.readBytes(phoneNumber, messageLen);
  phoneNumber[messageLen] = '\0';
  messageLen = 0;
  Serial.print("enter message to send\n");
  while(Serial.available() < 1)  { /*just wait for input. this is bad but whatevs, todo*/}
  messageLen = Serial.available();
  Serial.readBytes(message, messageLen);
  message[messageLen] = '\0';
  //sendASCIITextMessage(SIM, phoneNumber, message);
}
