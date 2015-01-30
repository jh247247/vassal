#!/usr/bin/python2.7

import serial
import time
ser = serial.Serial('/dev/ttyUSB0',921600,timeout=1)
ser.write("{\"C\":\"0\"}\0");
time.sleep(0.1)
ser.write("{\"C\":\"65535\"}\0");
time.sleep(0.1)
ser.write("{\"C\":\"0\"}\0");
for i in range(320/5):
    time.sleep(0.01)
    ser.write("{\"f\":\"0,240,0,"+str(i*5)+",6000\"}\0");
    print(i*5)
ser.close()
