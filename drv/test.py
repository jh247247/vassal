#!/usr/bin/python2.7

import serial
import time
ser = serial.Serial('/dev/ttyUSB0',115200,timeout=1)
for i in range(100):
    time.sleep(0.1)
    ser.write("{\"C\":\"65535\"}\0");
    time.sleep(0.1)
    ser.write("{\"C\":\"0\"}\0");
ser.close()
