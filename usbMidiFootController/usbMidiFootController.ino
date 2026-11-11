/*
  Based on Sketch built by Gustavo Silveira (aka Music Nerd)
  Modified by Dolce Wang

  This code is only for Arduinos that use ATmega328 (like Uno, Mega, Nano, Beetle...)

*/

// LIBRARIES

#include <MIDI.h> // by Francois Best
MIDI_CREATE_DEFAULT_INSTANCE(); 

// Button Modes:
// MOMENTARY: Sends CC value=127 on press, Sends CC value=0 on release.
// MOMENTARY_WITH_LONG_PRESS: Sends CC value=127 and value=0 in quick succession on release only. Sends on alternate CC value if held >1s.
// TOGGLE: Alternates between sending CC value=127 and value=0 on press.
// TOGGLE_WITH_LONG_PRESS: Alternates between sending CC value=127 and value=0 on release, or sends momentary pulse on alternate CC if held >1s.
// TOGGLE_WITH_MOMENTARY_LEDS: Same as TOGGLE, but LEDs are active only when pressed.
// TOGGLE_WITH_LONG_PRESS_AND_MOMENTARY_LEDS: Same as TOGGLE_WITH_LONG_PRESS, but LEDs are active only when pressed, and turn off if long press is triggered (held >1s).

enum ButtonType {
  MOMENTARY, 
  MOMENTARY_WITH_LONG_PRESS, 
  TOGGLE, 
  TOGGLE_WITH_LONG_PRESS,
  TOGGLE_WITH_MOMENTARY_LEDS,
  TOGGLE_WITH_LONG_PRESS_AND_MOMENTARY_LEDS
};

const int NButtons = 4; //***  total number of push buttons
const int NLeds = 2;

// Configure buttons below. Buttons are indexed from left to right on master pedal and then slave pedal.
const ButtonType buttonType[NButtons] = {
  TOGGLE_WITH_LONG_PRESS_AND_MOMENTARY_LEDS, 
  TOGGLE_WITH_LONG_PRESS_AND_MOMENTARY_LEDS,
  MOMENTARY,
  MOMENTARY
};

const int buttonPin[NButtons] = {2, 3, 4, 5}; 
const int ledPin[NLeds] = {6, 7};

// MIDI
const byte shortPressCC[NButtons] = {3, 9, 14, 15}; 
const byte longPressCC[NButtons] = {85, 86, 87, 88}; 
byte midiCh = 1; //** MIDI channel to be used; You can add more if you need to reorganize or have a billion buttons/pots

const unsigned long debounceDelay = 50;   
const unsigned long longPressDelay = 1000;   
const int NIterationsPendingCC0 = 1000;
                                  
int buttonCurrentState[NButtons] = {0};  
int buttonPreviousState[NButtons] = {0};
int pendingSendCC[NButtons] = {0};  // For buttons with long press active, this flag =1 when awaiting either button release or long-press delay expiry.
int toggleState[NButtons] = {0}; // For toggle buttons, this flag tracks the current toggle state.
int pendingShortPressCC0[NButtons] = {0}; // For buttons with long press active, these flags =1 when a CC127 message has just been sent, and is awaiting CC0.
int pendingLongPressCC0[NButtons] = {0}; //
int pendingShortPressCC0Iterations[NButtons] = {0};
int pendingLongPressCC0Iterations[NButtons] = {0};
unsigned long lastDebounceTime[NButtons] = {0};  // the last time the output pin was toggled.
unsigned long lastFrontEdgeTime[NButtons] = {0}; // the last time the button was pressed down.





void setup() { 
  Serial.begin(115200); //**  Baud Rate 31250 for MIDI class compliant jack | 115200 for Hairless MIDI

  // Initialize buttons with pull up resistors
  for (int i = 0; i < NButtons; i++) {
    pinMode(buttonPin[i], INPUT_PULLUP);
  }
  for (int i = 0; i < NLeds; i++) {
    pinMode(ledPin[i], OUTPUT);
  }
}

void writeLED(int i, byte state){
  if(i< NLeds){
    digitalWrite(ledPin[i], state);
  }
}

void toggleButtonsWithLongPress(int i){
  if (pendingLongPressCC0[i]){
    pendingLongPressCC0Iterations[i]++;
    if(pendingLongPressCC0Iterations[i] >= NIterationsPendingCC0){
      pendingLongPressCC0[i] = 0;
      pendingLongPressCC0Iterations[i] = 0;
      MIDI.sendControlChange(longPressCC[i], 0, midiCh);
      digitalWrite(i, LOW);
    }
  }
  if ((millis() - lastDebounceTime[i]) > debounceDelay) {
    if (buttonPreviousState[i] != buttonCurrentState[i]) { // Button State is CHANGED
      lastDebounceTime[i] = millis();
      if (buttonCurrentState[i] == LOW) { // Button just pressed
        pendingSendCC[i] = 1;
        lastFrontEdgeTime[i] = millis();

        if(buttonType[i] == TOGGLE_WITH_LONG_PRESS){
          writeLED(i, !toggleState[i]);
        } else if (buttonType[i] == TOGGLE_WITH_LONG_PRESS_AND_MOMENTARY_LEDS){
          writeLED(i, HIGH);
        }
      } else if (buttonCurrentState[i] == HIGH){ // button just released
        if(pendingSendCC[i]){
          toggleState[i] = !toggleState[i];
          MIDI.sendControlChange(shortPressCC[i], toggleState[i] ? 127 : 0, midiCh);
          if(buttonType[i] == TOGGLE_WITH_LONG_PRESS){
            writeLED(i, toggleState[i]);
          } else if (buttonType[i] == TOGGLE_WITH_LONG_PRESS_AND_MOMENTARY_LEDS) {
            writeLED(i, LOW);
          }
        }

        
      }
      buttonPreviousState[i] = buttonCurrentState[i];
    } else { // Button State is UNCHANGED
      if (buttonCurrentState[i] == LOW && pendingSendCC[i] == 1) { // Button is being held
        if ((millis() - lastFrontEdgeTime[i]) > longPressDelay){
          pendingSendCC[i] = 0;
          pendingLongPressCC0[i] = 1;
          MIDI.sendControlChange(longPressCC[i], 127, midiCh);

          if(buttonType[i] == TOGGLE_WITH_LONG_PRESS){
            writeLED(i, !toggleState[i]);
          } else if (buttonType[i] == TOGGLE_WITH_LONG_PRESS_AND_MOMENTARY_LEDS){
            writeLED(i, LOW);
          }
        }
      }

    }
  }
}

void toggleButtons(int i){
  if ((millis() - lastDebounceTime[i]) > debounceDelay) {
    if (buttonPreviousState[i] != buttonCurrentState[i]) { // Button State is CHANGED
      lastDebounceTime[i] = millis();
      if (buttonCurrentState[i] == LOW) { // Button just pressed
        toggleState[i] = !toggleState[i];
        MIDI.sendControlChange(shortPressCC[i], toggleState[i] ? 127 : 0, midiCh);
        if(buttonType[i] == TOGGLE){
          writeLED(i, toggleState[i]);
        } else if (buttonType[i] == TOGGLE_WITH_MOMENTARY_LEDS){
          writeLED(i, HIGH);
        }
      } else if (buttonCurrentState[i] == HIGH){ // button just released
        if(buttonType[i] == TOGGLE){
        
        } else if (buttonType[i] == TOGGLE_WITH_MOMENTARY_LEDS) {
          writeLED(i, LOW);
        }
      }
      buttonPreviousState[i] = buttonCurrentState[i];
    } else { // Button State is UNCHANGED

    }
  }
}

void momentaryButtonsWithLongPress(int i){
  if (pendingShortPressCC0[i]){
    pendingShortPressCC0Iterations[i]++;
    if(pendingShortPressCC0Iterations[i] >= NIterationsPendingCC0){
      pendingShortPressCC0[i] = 0;
      pendingShortPressCC0Iterations[i] = 0;
      MIDI.sendControlChange(shortPressCC[i], 0, midiCh);
      writeLED(i,LOW);
    }
  }

  if (pendingLongPressCC0[i]){
    pendingLongPressCC0Iterations[i]++;
    if(pendingLongPressCC0Iterations[i] >= NIterationsPendingCC0){
      pendingLongPressCC0[i] = 0;
      pendingLongPressCC0Iterations[i] = 0;
      MIDI.sendControlChange(longPressCC[i], 0, midiCh);
      writeLED(i,LOW);
    }
  }
  if ((millis() - lastDebounceTime[i]) > debounceDelay) {
    if (buttonPreviousState[i] != buttonCurrentState[i]) { // Button State is CHANGED
      lastDebounceTime[i] = millis();
      if (buttonCurrentState[i] == LOW) { // Button just pressed
        writeLED(i,HIGH);
        pendingSendCC[i] = 1;
        lastFrontEdgeTime[i] = millis();
      } else if (buttonCurrentState[i] == HIGH){ // button just released
        if (pendingSendCC[i] == 1){
          pendingSendCC[i] = 0;
          pendingShortPressCC0[i] = 1;
          MIDI.sendControlChange(shortPressCC[i], 127, midiCh);
          writeLED(i,LOW);
        }
      }
      buttonPreviousState[i] = buttonCurrentState[i];
    } else { // Button State is UNCHANGED
      if (buttonCurrentState[i] == LOW && pendingSendCC[i] == 1) { // Button is being held
        if ((millis() - lastFrontEdgeTime[i]) > longPressDelay){
          pendingSendCC[i] = 0;
          pendingLongPressCC0[i] = 1;
          MIDI.sendControlChange(longPressCC[i], 127, midiCh);
          writeLED(i,LOW);
        }
      }

    }
  }
}

void momentaryButtons(int i){
  if ((millis() - lastDebounceTime[i]) > debounceDelay) {
    if (buttonPreviousState[i] != buttonCurrentState[i]) { // Button State is CHANGED
      lastDebounceTime[i] = millis();
      if (buttonCurrentState[i] == LOW) { // Button just pressed
        writeLED(i,HIGH);
        MIDI.sendControlChange(shortPressCC[i], 127, midiCh);
      } else if (buttonCurrentState[i] == HIGH){ // button just released
        writeLED(i,LOW);
        MIDI.sendControlChange(shortPressCC[i], 0, midiCh);
      }
      buttonPreviousState[i] = buttonCurrentState[i];      
    } else { // Button State is UNCHANGED
    }
  }
}

// LOOP
void loop() {
  for (int i = 0; i < NButtons; i++) {
    buttonCurrentState[i] = digitalRead(buttonPin[i]);
    if(buttonType[i] == MOMENTARY){
      momentaryButtons(i);
    } else if (buttonType[i] == MOMENTARY_WITH_LONG_PRESS){
      momentaryButtonsWithLongPress(i);
    } else if (buttonType[i] == TOGGLE || buttonType[i] == TOGGLE_WITH_MOMENTARY_LEDS){
      toggleButtons(i);
    } else if (buttonType[i] == TOGGLE_WITH_LONG_PRESS || buttonType[i] == TOGGLE_WITH_LONG_PRESS_AND_MOMENTARY_LEDS){
      toggleButtonsWithLongPress(i);
    }
  }
}
