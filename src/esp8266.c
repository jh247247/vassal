#include "usart.h"
#include "timer.h"
#include "esp8266.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static char *i2a(unsigned i, char *a, unsigned r)
{
  if (i / r > 0)
    a = i2a(i / r, a, r);
  *a = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[i % r];
  return a + 1;
}

char *itoa(char *a,int i, int r)
{
  if ((r < 2) || (r > 36))
    r = 10;
  if (i < 0) {
    *a = '-';
    *i2a(-(unsigned) i, a + 1, r) = 0;
  } else
    *i2a(i, a, r) = 0;

  return a;
}

unsigned char ESP8266_sendCommand(char* command, int timeout);

unsigned char ESP8266_init(void) {
  ESP8266_waitForReady(5000);
  if(ESP8266_test()) {
    ESP8266_sendCommand("ATE0\r\n",100); // disable echo back

    return 1;
  }
  return 0;
}

// test the
unsigned char ESP8266_test(void) {
  return ESP8266_sendCommand("AT\r\n",500);
}

unsigned char ESP8266_connect(char* ssid, char* pass) {
  USART1_PutString("AT+CWJAP=\"");
  USART1_PutString(ssid);
  USART1_PutString("\",\"");
  USART1_PutString(pass);
  return ESP8266_sendCommand("\"\r\n",20000);
}

unsigned char ESP8266_isConnected(void) {
  return ESP8266_sendCommand("AT+CWJAP?\r\n",500);
}

unsigned char ESP8266_sendPacket(char* type, char* ip, char* port,
                                 char* data, int length) {
  char buf[6];
  int i;
  // start session
  USART1_PutString("AT+CIPSTART=\"");
  USART1_PutString(type);
  USART1_PutString("\",\"");
  USART1_PutString(ip);
  USART1_PutString("\",");
  USART1_PutString(port);
  if(!ESP8266_sendCommand("\r\n",5000)) {
    // module timed out, send close just in case
    ESP8266_sendCommand("AT+CIPCLOSE\r\n", 0);
    return 0;
  }

  // assume that the data is not null terminated,
  itoa(buf,length,10);
  USART1_PutString("AT+CIPSEND=");
  USART1_PutString(buf);
  USART1_PutString("\r\n");
  if(!ESP8266_waitForPacketStart(5000)) {
    ESP8266_sendCommand("AT+CIPCLOSE\r\n", 0);
    return 0;
  }
  for(i = 0; i < length; i++) {
    USART1_PutChar(data[i]);
  }
  if(!ESP8266_sendCommand("\r\n",5000)) {
    ESP8266_sendCommand("AT+CIPCLOSE\r\n", 0);
    return 0;
  }
  return ESP8266_sendCommand("AT+CIPCLOSE\r\n", 5000);
}

// timeout in ms
unsigned char ESP8266_sendCommand(char* command, int timeout) {
  USART1_PutString(command);

  // start checking for timeout
  // this may have to be a bit smarter in the end
  return ESP8266_waitForOK(timeout);

}


unsigned char ESP8266_waitForOK(int timeout) {
  return USART_waitForString(USART1, "OK", timeout*sysTicksPerMillisecond);
}


unsigned char ESP8266_waitForReady(int timeout) {
  return USART_waitForString(USART1, "ready", timeout*sysTicksPerMillisecond);
}

unsigned char ESP8266_waitForPacketStart(int timeout) {
  return USART_waitForString(USART1, ">", timeout*sysTicksPerMillisecond);
}
