# Remote ID Module for L3Harris 
## by Conor Toole and Evan Martin 

## Background
This project was created at the University of Arizona in collaboration with L3Harris as a senior capstone project. This repository has the code written by Conor and Evan to run on a nordic nrf52840 feather development board. The arduino framework was utilized for this code as well as a few external libraries linked below. 

## how to set up 
In order to get this project running the custom libraries rimBattery and rimLED must be included into the project. In the arduino IDE these can be included as a zip file to the project using the include library feature. In another IDE the files within the rimBattery and rimLED can simply be included in the local file directory. 

## external libraries
This project used these libraries
SoftwareSerial (built in to arduino)
TinyGPSPlus
Bluefruit 

## Extra 
included in this repo is the Led_battery_test file which can be uploaded and used to verify the functionality of the leds and the battery.
