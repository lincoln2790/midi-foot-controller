/*
  Based on Sketch built by Gustavo Silveira (aka Music Nerd)
  Modified by Dolce Wang

  This code is only for Arduinos that use ATmega328 (like Uno, Mega, Nano, Beetle...)

*/

// LIBRARIES

#include <MIDI.h> // by Francois Best
MIDI_CREATE_DEFAULT_INSTANCE(); 


// BUTTONS
const int NButtons = 4; //***  total number of push buttons
const int NLeds = 2;

const int buttonPin[NButtons] = {2, 3, 5, 7}; //*** define digital pins connected from button to Arduino; (ie {10, 16, 14, 15, 6, 7, 8, 9, 2, 3, 4, 5}; 12 buttons)
                                        //** Button NOTE will go up chromatically.  ie. if button is digi pin 2, C; Pin 3, C#; Pin 3, D; etc
const int ledPin[2] = {11, 12};
const int isLongPressActive[NButtons] = {1,1,0,0}; // For each button, 1 to select whether long press is an active command.
                                          // If so, short press will trigger on the falling edge (second edge) of the press.

const byte shortPressCC[NButtons] = {3, 9, 14, 15}; 
const byte longPressCC[NButtons] = {85, 86, 87, 88}; 

                                   
int buttonCState[NButtons] = {};        // stores the button current value
int buttonPState[NButtons] = {};        // stores the button previous value
int isButtonHeld[NButtons] = {0,0,0,0};
int sendShortPressCCZeroNext[NButtons] = {0,0,0,0};
int sendLongPressCCZeroNext[NButtons] = {0,0,0,0};
int sendShortPressCCZeroIterations[NButtons] = {0,0,0,0};
int sendLongPressCCZeroIterations[NButtons] = {0,0,0,0};
int NIterations = 1000;

// debounce
unsigned long lastDebounceTime[NButtons] = {0};  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    //* the debounce time; increase if the output flickers
unsigned long lastFrontEdgeTime[NButtons] = {0};
unsigned long longPressDelay = 1000;    //* the debounce time; increase if the output flickers

// MIDI
byte midiCh = 1; //** MIDI channel to be used; You can add more if you need to reorganize or have a billion buttons/pots

// SETUP
void setup() { 
  Serial.begin(115200); //**  Baud Rate 31250 for MIDI class compliant jack | 115200 for Hairless MIDI

  // Buttons
  // Initialize buttons with pull up resistors
  for (int i = 0; i < NButtons; i++) {
    pinMode(buttonPin[i], INPUT_PULLUP);
  }
  for (int i = 0; i < NLeds; i++) {
    pinMode(ledPin[i], OUTPUT);
  }
}


// LOOP
void loop() {
  buttons();
}


// BUTTONS
void buttons() {
  for (int i = 0; i < NButtons; i++) {
    buttonCState[i] = digitalRead(buttonPin[i]);  // read pins from arduino
    if (sendShortPressCCZeroNext[i]){
      sendShortPressCCZeroIterations[i]++;
      if(sendShortPressCCZeroIterations[i] >= NIterations){
        sendShortPressCCZeroNext[i] = 0;
        sendShortPressCCZeroIterations[i] = 0;
        MIDI.sendControlChange(shortPressCC[i], 0, midiCh);
        if(i< NLeds){
          digitalWrite(ledPin[i], LOW);
        }
      }
    }
    if (sendLongPressCCZeroNext[i]){
      sendLongPressCCZeroIterations[i]++;
      if(sendLongPressCCZeroIterations[i] >= NIterations){
        sendLongPressCCZeroNext[i] = 0;
        sendLongPressCCZeroIterations[i] = 0;
        MIDI.sendControlChange(longPressCC[i], 0, midiCh);
        if(i< NLeds){
          digitalWrite(ledPin[i], LOW);
        }
      }
    }

    if ((millis() - lastDebounceTime[i]) > debounceDelay) {

      if (buttonPState[i] != buttonCState[i]) { // Button State is changed
        lastDebounceTime[i] = millis();
      
        if (buttonCState[i] == LOW) { // Button just pressed
          if(!isLongPressActive[i]){
            MIDI.sendControlChange(shortPressCC[i], 127, midiCh);
            if(i< NLeds){
              digitalWrite(ledPin[i], HIGH);
            }
          } else {
            lastFrontEdgeTime[i] =millis();
            isButtonHeld[i] = 1;
          }

        } else { // button just released
          if(!isLongPressActive[i]){
            MIDI.sendControlChange(shortPressCC[i], 0, midiCh);
            if(i< NLeds){
              digitalWrite(ledPin[i], LOW);
            }
          } else {
            if (isButtonHeld[i] == 1){
              isButtonHeld[i] = 0;
              sendShortPressCCZeroNext[i] = 1;
              MIDI.sendControlChange(shortPressCC[i], 127, midiCh);
              if(i< NLeds){
                digitalWrite(ledPin[i], HIGH);
              }
            }
          }
        } 
        buttonPState[i] = buttonCState[i];
      } else { // Button State is UNCHANGED
        if (isLongPressActive[i]){
          if (buttonCState[i] == LOW && isButtonHeld[i] == 1) { // Button is being held
            if ((millis() - lastFrontEdgeTime[i]) > longPressDelay){
              isButtonHeld[i] = 0;
              sendLongPressCCZeroNext[i] = 1;
              MIDI.sendControlChange(longPressCC[i], 127, midiCh);
              if(i< NLeds){
                digitalWrite(ledPin[i], HIGH);
              }
              
            }
          } else { // Button idling

          }
        }

      }

    }
  }
}
