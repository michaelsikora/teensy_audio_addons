# Sikora Teensy Audio Add-ons

## Installation

This repository is intended to be used as an Arduino Library. It is used with the Teensy Audio Library to add on new audio features.

To install:

1. git clone https://github.com/michaelsikora/teensy_audio_addons.git to the location of your arduino libraries. For me this is:  ~\Documents\Arduino\libraries\

2. Install the Teensy software: [Teensyduino: Download and Install Teensy support into the Arduino IDE](https://www.pjrc.com/teensy/td_download.html)

3. You can then open the Arduino IDE and find the examples under File > Examples > Sikora_Teensy_NS > ...



# Addons:

## 1. Noise Suppressor

implemented in src/effect_sikora_antinoise.cpp & .h . The algorithm was developed in the python notebook dev/teensyNSpythonTests/testing.ipynb . The algorithm sets the gain of a buffer based on a correlation. 



## 2. PCM1865 Controller (Work In Progress)

This still needs developed. The audio library has several controllers for other audio codecs. To start the PCM1865 driver I have made copies of one of the controller .cpp/.h files. The PCM1865 driver will need to define the I2C communication signals needed based on the spec sheet for the board : [PCM1865 data sheet, product information and support | TI.com](https://www.ti.com/product/PCM1865) 

In addition to the software driver, a circuit should also be defined based on the datasheet. I am considering focusing on a 4 mic-array application to setup a working design.
