
#include <Bounce.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include <effect_sikora_antinoise.h>

AudioSikoraAntiNoise  ns;
AudioInputAnalog      adcs1(A14);
AudioAmplifier        amp1;
AudioFilterBiquad     biquad1;
AudioOutputUSB        usb1;
AudioConnection       patchCord1(adcs1, 0, ns, 0);

// SINGLE BIQUAD FILTER
AudioConnection       patchCord2(ns, 0, biquad1, 0);
AudioConnection       patchCord3(biquad1, 0, amp1, 0);

// NO BIQUAD FILTER
//AudioConnection       patchCord5(ns, 0, amp1, 0);

// DOUBLE BIQUAD FILTERS
//AudioFilterBiquad     biquad2;
//AudioConnection       patchCord2(ns, 0, biquad1, 0);
//AudioConnection       patchCord5(biquad1, 0, biquad2, 0);
//AudioConnection       patchCord3(biquad2, 0, amp1, 0);

AudioConnection       patchCord4(amp1, 0, usb1, 0);

Bounce buttonNSON = Bounce(0, 8);
Bounce buttonNSOFF = Bounce(1, 8);
int mode = 1;

void setup() {
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  
  AudioMemory(64);
//  ns.begin(NS_PASSTHRU, 0);
  ns.begin(AudioNoiseProfile1, 257);
  ns.set_alpha(0.1);

  amp1.gain(2);

  // HOME LAPTOP E580
//  biquad1.setNotch(0, 50, 1);
//  biquad1.setNotch(1, 150, 4);
//  biquad1.setNotch(2, 250, 8);
//  biquad1.setNotch(3, 1000, 16);

// HOME WORK LAPTOP HP
  biquad1.setHighpass(0, 60, 0.707);
  biquad1.setNotch(1, 100, 16);
  biquad1.setNotch(2, 230, 8);
  biquad1.setNotch(3, 1000, 16);

//  biquad2.setNotch(0, 1540, 2);
//  biquad2.setHighShelf(1, 2000, 1, 0.5);
//  biquad2.setNotch(1, 100, 16);
//  biquad2.setNotch(2, 1540, 16);
//  biquad2.setNotch(3, 1000, 16);

}

void loop() {
  // First, read the buttons
  buttonNSON.update();
  buttonNSOFF.update();

  // Respond to button presses
  if (buttonNSON.fallingEdge()) {
    Serial.print(mode);
    Serial.println(" : NS ON button Pressed");
//    if (mode == 1) stopNS();
    if (mode == 0) startNS();
  }
  
  if (buttonNSOFF.fallingEdge()) {
    Serial.print(mode);
    Serial.println(" : NS OFF button Pressed");
    if (mode == 1) stopNS();
//    if (mode == 0) startNS();
  }

  if (mode == 1) {
    Serial.print(ns.multiplier);
    Serial.print(" : ");
    Serial.println(ns.state);
    delay(2);
  }
}

void stopNS(){
  Serial.println("STOP NS");
  ns.end();
  ns.begin(NS_PASSTHRU, 0);
  mode = 0;
}

void startNS(){
  mode = 1;
  ns.end();
  ns.begin(AudioNoiseProfile1, 257);
  ns.set_alpha(0.1);
}
