#include <TimerOne.h>
#include <CapacitiveSensor.h>
#include "CapacitiveTouchSwitch.h"

const int midi_min=60;
const int midi_max=82;
long int timer=0;
int current_note;
bool sns01_on=false;
long threshold=40;
const uint8_t samples=30;

CapacitiveTouchSwitch sns0(1,0,samples,threshold);
CapacitiveTouchSwitch sns2(1,2,samples,threshold);
CapacitiveTouchSwitch sns3(4,3,samples,threshold);
CapacitiveTouchSwitch sns5(4,5,samples,threshold);
CapacitiveTouchSwitch sns6(7,6,samples,threshold);
CapacitiveTouchSwitch sns8(7,8,samples,threshold);

const int numSensors=6;
CapacitiveTouchSwitch *sensors[numSensors]={&sns0, &sns2, &sns3, &sns5, &sns6, &sns8};
const uint8_t midi_notes[numSensors]={64, 66, 68, 69, 71, 73};
const uint8_t led_pins[numSensors]={21,20,19,18,17,16};

void setup () {
  //Start serial. Baud rate is ignored with USB.
  Serial.begin(9600);
  Serial.println("Capacitive Sensing Demo");
  //set led pins as output low
  for (int i=0; i<numSensors; i++) {
    pinMode(led_pins[i],OUTPUT);
    digitalWrite(led_pins[i],LOW);
    //configure sensor Hysteresis and timeout
    sensors[i]->SetHysteresis(threshold/2); 
    sensors[i]->SetTimeoutMs(5000); //5 seconds, for now
    sensors[i]->SetBaselineAlways(false); //don't adjust baseline while on 
  }
}

void loop () {
  for (int i=0; i<numSensors; i++) {
    sensors[i]->Update();
    Serial.print(i);
    Serial.print(':');
    Serial.print(sensors[i]->LastResult());
    Serial.print(',');
    Serial.print(sensors[i]->Baseline());
    Serial.print('\t');
    //look for a change in switch state
    if (sensors[i]->Changed()) {
      //if changed and now on, send a MIDI note on event
      if (sensors[i]->State()) {
        usbMIDI.sendNoteOn(midi_notes[i], 127, 1);
        digitalWrite(led_pins[i],HIGH);
      //if changed and off, send a note off
      } else {
        usbMIDI.sendNoteOff(midi_notes[i], 0, 1);
        digitalWrite(led_pins[i],LOW);
      }
    }
  }
  Serial.println();
  delay(15);
}

