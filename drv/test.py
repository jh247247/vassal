#!/usr/bin/python2.7

import serial
import time
import random

delay = 1.1

ser = serial.Serial('/dev/ttyUSB0',115200,timeout=1)
ser.write("{\"C\":\"0\"}\0");
time.sleep(0.1)
ser.write("{\"C\":\"65535\"}\0");
time.sleep(0.1)

# for i in range(320/5):
#     time.sleep(delay)
#     ser.write("{\"f\":\"0,240,0,"+str(i*5)+","+str(random.randrange(65535))+"\"}\0");
#     print(i*5)

# for i in range(320/8):
#     time.sleep(delay)
#     ser.write("{\"s\":\""+str(i*8)+",0,65535,0,0,Hello World!\"}\0");
#     print(i*8)

for i in range(5):
    ser.write("""{
    {\"t\":\"s\",\"l\":\"512\",\"x0\":\"0\",\"y0\":\"5,320\",\"c0\":\"0\",\"s\":\"Hello!\",\"b\":\"1000\"},
    {\"t\":\"s\",\"l\":\"384\",\"x0\":\"120\",\"y0\":\"5,320\",\"c0\":\"0\",\"s\":\"Goodbye!\",\"b\":\"655\"}}\0""");
    time.sleep(delay)
    ser.write("""{{\"t\":\"f\", \"l\":\"30\", \"x0\":\"0\", \"x1\":\"240\",
    \"y0\":\"320\", \"y1\":\"310,0\", \"c0\":\"0\",\"b\":\"1000\"}}\0""");
    time.sleep(delay)
    ser.write("""{{\"t\":\"f\", \"l\":\"30\", \"x0\":\"0\", \"x1\":\"240\",
    \"y0\":\"0\", \"y1\":\"0,320\", \"c0\":\"65535\",\"b\":\"20000\"}}\0""");
    time.sleep(delay)

ser.close()
