# NodeMCU based light level ,temperature and pressure sensor based on the BMP180
 
 This sensor collects data from a light dependent resistor through the NodeMCU's ADC port as well
 as temperature and pressure values from the BMP180 sensor through i2c.
 
 circuit diagram
 ===============

![circuit](images/circuit.svg)

Description
===========
The Wifi ssid, it's password and the post url are configurable through a setup menu which is 
accessible theough the virtual serial port which is available when the NodeMcu is connected to
the computer and powered via the Micro USB connection. 

Find the available COM port using the Device manager (in windows).The default baud rate is 115200.

![Setup Menu](images/SetupMenu.png)



 
 