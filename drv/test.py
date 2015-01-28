#!/usr/bin/python2.7

import serial
import time
ser = serial.Serial('/dev/ttyUSB0',115200,timeout=1)
for i in range(100):
    time.sleep(0.025)
    ser.write("{\"C\":\"65535\"}\0");
    print("a")
    time.sleep(0.025)
    ser.write("{\"C\":\"0\"}\0");
    print("b")
ser.close()
