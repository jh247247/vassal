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
  USART2_PutString("AT+CWJAP=\"");
  USART2_PutString(ssid);
  USART2_PutString("\",\"");
  USART2_PutString(pass);
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
  USART2_PutString("AT+CIPSTART=\"");
  USART2_PutString(type);
  USART2_PutString("\",\"");
  USART2_PutString(ip);
  USART2_PutString("\",");
  USART2_PutString(port);
  if(!ESP8266_sendCommand("\r\n",5000)) {
    // module timed out, send close just in case
    ESP8266_sendCommand("AT+CIPCLOSE\r\n", 0);
    return 0;
  }

  // assume that the data is not null terminated,
  itoa(buf,length,10);
  USART2_PutString("AT+CIPSEND=");
  USART2_PutString(buf);
  USART2_PutString("\r\n");
  if(!ESP8266_waitForPacketStart(5000)) {
    ESP8266_sendCommand("AT+CIPCLOSE\r\n", 0);
    return 0;
  }
  for(i = 0; i < length; i++) {
    USART2_PutChar(data[i]);
  }
  if(!ESP8266_sendCommand("\r\n",5000)) {
    ESP8266_sendCommand("AT+CIPCLOSE\r\n", 0);
    return 0;
  }
  return ESP8266_sendCommand("AT+CIPCLOSE\r\n", 5000);
}

// timeout in ms
unsigned char ESP8266_sendCommand(char* command, int timeout) {
  USART2_PutString(command);

  // start checking for timeout
  // this may have to be a bit smarter in the end
  return ESP8266_waitForOK(timeout);

}


unsigned char ESP8266_waitForOK(int timeout) {
  return USART_waitForString(USART2, "OK", timeout*sysTicksPerMillisecond);
}


unsigned char ESP8266_waitForReady(int timeout) {
  return USART_waitForString(USART2, "ready", timeout*sysTicksPerMillisecond);
}

unsigned char ESP8266_waitForPacketStart(int timeout) {
  return USART_waitForString(USART2, ">", timeout*sysTicksPerMillisecond);
}
