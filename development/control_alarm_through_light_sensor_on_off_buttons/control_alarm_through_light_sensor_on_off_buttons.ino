/*
 *  Control Alarm Through Light Sensor & On/Off Buttons.
 *
 *  Copyright (C) 2010 Efstathios Chatzikyriakidis (contact@efxa.org)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// include for portable "non-local" jumps.
#include <setjmp.h>

// include notes' frequencies.
#include "pitches.h"

const int buttonOnPin = 2;  // the pin number of the button "on" element.
const int buttonOnIRQ = 0;  // the IRQ number of the button "on" pin.

const int buttonOffPin = 3; // the pin number of the button "off" element.
const int buttonOffIRQ = 1; // the IRQ number of the button "off" pin.

const int ledPin = 13;   // the led pin of the alarm.
const int piezoPin = 9;  // the piezo pin of the alarm.
const int sensorPin = 0; // the sensor pin of the alarm.

const int sensorThreshold = 100; // sensor threshold value for the alarm.
const int BOUNCE_DURATION = 200; // define a bounce time for a button.
volatile long bounceTime = 0;    // ms count to debounce a pressed button.

// the state of the alarm (on = true, off = false).
volatile boolean alarmState = false;

// information to restore calling environment.
jmp_buf buf;

// notes in the alarm melody.
const int notesMelody[] = {
  NOTE_G4, NOTE_C4, NOTE_G3, NOTE_G3, NOTE_C4, NOTE_G3, NOTE_C4
};

// calculate the number of the notes in the melody in the array.
const int NUM_NOTES = (int) (sizeof (notesMelody) / sizeof (const int));

// note durations: 4 = quarter note, 8 = eighth note, etc.
const int noteDurations[] = {
  4, 8, 8, 2, 4, 2, 4
};

// startup point entry (runs once).
void setup() {
  // set buttons "on/off" elements as an input.
  pinMode(buttonOnPin, INPUT);
  pinMode(buttonOffPin, INPUT);

  // set the input sensor as input.
  pinMode(sensorPin, INPUT);

  // set the led and piezo as outputs.
  pinMode(ledPin, OUTPUT);
  pinMode(piezoPin, OUTPUT);

  // attach the ISRs for the IRQs (for buttons).
  attachInterrupt(buttonOnIRQ, buttonOnISR, RISING);
  attachInterrupt(buttonOffIRQ, buttonOffISR, RISING);
}

// loop the main sketch.
void loop() {
  // save the environment of the calling function.
  setjmp(buf);

  // if the alarm is on start working.
  if(alarmState) {
    // first light the led.
    digitalWrite(ledPin, HIGH);

    // get a value from the sensor.
    int lightLevel = analogRead(sensorPin);
    
    // check the value with the threshold value.
    if (lightLevel > sensorThreshold)
      // trigger the alarm melody.
      playMelody ();
  }
  else
    // dark the led and do nothing.
    digitalWrite(ledPin, LOW);
}

// ISR for the button "on" IRQ (is called on button presses).
void buttonOnISR () {
  // it ignores presses intervals less than the bounce time.
  if (abs(millis() - bounceTime) > BOUNCE_DURATION) {
 
    // on the alarm device.
    alarmState = true;
    
    // set whatever bounce time in ms is appropriate.
    bounceTime = millis(); 
  }
}

// ISR for the button "off" IRQ (is called on button presses).
void buttonOffISR () {
  // it ignores presses intervals less than the bounce time.
  if (abs(millis() - bounceTime) > BOUNCE_DURATION) {
    
    // off the alarm device.
    alarmState = false;
    
    // set whatever bounce time in ms is appropriate.
    bounceTime = millis(); 

    // go to the main loop and start again.
    longjmp(buf, 0);
  }
}

// play a melody and return immediately.
void playMelody () {
  // iterate over the notes of the melody.
  for (int thisNote = 0; thisNote < NUM_NOTES; thisNote++) {
    // to calculate the note duration, take one second divided by the note type.
    // e.g. quarter note = 1000/4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];

    // play the tone.
    tone(piezoPin, notesMelody[thisNote], noteDuration);

    // to distinguish notes, set a minimum time between them.
    // the note's duration plus 30% seems to work well enough.
    int pauseBetweenNotes = noteDuration * 1.30;

    // delay some time.
    delay(pauseBetweenNotes);
  }
}
