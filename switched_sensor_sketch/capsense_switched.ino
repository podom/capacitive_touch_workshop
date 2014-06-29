#include <TimerOne.h>
#include <CapacitiveSensor.h>
#include "CapacitiveTouchSwitch.h"
#include "SwitchedCapSensor.h"

const int midi_min=60;
const int midi_max=82;
long int timer=0;
int current_note;
bool sns01_on=false;
long threshold=100;
const uint8_t samples=30;

SwitchedCapSensor sns10(0,1,samples,threshold);
SwitchedCapSensor sns01(2,3,samples,threshold);
SwitchedCapSensor sns32(4,5,samples,threshold);
SwitchedCapSensor sns23(6,7,samples,threshold);
SwitchedCapSensor sns54(8,9,samples,threshold);
//CapacitiveTouchSwitch snsTest1(16,11,samples,threshold);
CapacitiveTouchSwitch snsTest2(16,12,samples,threshold);

const int numSensors=5;
SwitchedCapSensor *sensors[numSensors]={&sns10, &sns01, &sns32, &sns23, &sns54};
const uint8_t midi_notes[numSensors]={64, 66, 68, 69, 71};
const uint8_t led_pins[numSensors]={21,20,19,18,17};

void setup () {
  //Start serial. Baud rate is ignored with USB.
  Serial.begin(9600);
  Serial.println("Capacitive Sensing Demo");
  //set led pins as output low
  for (int i=0; i<numSensors; i++) {
    pinMode(led_pins[i],OUTPUT);
    digitalWrite(led_pins[i],LOW);
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
    if (sensors[i]->Changed()) {
      if (sensors[i]->State()) {
        usbMIDI.sendNoteOn(midi_notes[i], 127, 1);
        digitalWrite(led_pins[i],HIGH);
      } else {
        usbMIDI.sendNoteOff(midi_notes[i], 0, 1);
        digitalWrite(led_pins[i],LOW);
      }
    }
  }
  //snsTest1.Update();
  snsTest2.Update();
  //Serial.print(snsTest1.LastResult());
  Serial.print('\t');
  Serial.print(snsTest2.LastResult());
  Serial.print('\t');

  Serial.println();
  delay(10);
}

