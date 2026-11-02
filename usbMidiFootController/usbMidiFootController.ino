/*
  Based on Sketch built by Gustavo Silveira (aka Music Nerd)
  Modified by Dolce Wang

  This code is only for Arduinos that use ATmega328 (like Uno, Mega, Nano, Beetle...)

*/


// Change values with //*** 


// LIBRARIES

#include <MIDI.h> // by Francois Best
MIDI_CREATE_DEFAULT_INSTANCE(); 


// BUTTONS
const int NButtons = 4; //***  total number of push buttons
const int buttonPin[NButtons] = {2, 3, 5, 7}; //*** define digital pins connected from button to Arduino; (ie {10, 16, 14, 15, 6, 7, 8, 9, 2, 3, 4, 5}; 12 buttons)
                                        //** Button NOTE will go up chromatically.  ie. if button is digi pin 2, C; Pin 3, C#; Pin 3, D; etc
const byte controlChangeNumber[NButtons] = {3, 9, 14, 15};
                                   
int buttonCState[NButtons] = {};        // stores the button current value
int buttonPState[NButtons] = {};        // stores the button previous value


// debounce
unsigned long lastDebounceTime[NButtons] = {0};  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    //* the debounce time; increase if the output flickers


// MIDI
byte midiCh = 1; //** MIDI channel to be used; You can add more if you need to reorganize or have a billion buttons/pots
byte note = 36; //** First note to be used for digital buttons, then go up chromatically in scale according to the sequence in your "buttonPin" array
                // you can look up on a Midi Note chart; 36=C2; 60=Middle C
byte cc = 1; //** First MIDI CC to be used for pots on Analog Pins in order of the "potPin" array; then goes up by 1

// SETUP
void setup() { 

  Serial.begin(115200); //**  Baud Rate 31250 for MIDI class compliant jack | 115200 for Hairless MIDI

  
  // Buttons
  // Initialize buttons with pull up resistors
  for (int i = 0; i < NButtons; i++) {
    pinMode(buttonPin[i], INPUT_PULLUP);
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


    if ((millis() - lastDebounceTime[i]) > debounceDelay) {

      if (buttonPState[i] != buttonCState[i]) {
        lastDebounceTime[i] = millis();

        if (buttonCState[i] == LOW) {

          // Sends the MIDI note ON accordingly to the chosen board

          // MIDI.sendNoteOn(note + i, 127, midiCh); // note, velocity, channel
          MIDI.sendControlChange(controlChangeNumber[i],127, midiCh);


        }
        else {

          // Sends the MIDI note OFF accordingly to the chosen board

          // MIDI.sendNoteOn(note + i, 0, midiCh); // note, velocity, channel
          MIDI.sendControlChange(controlChangeNumber[i], 0, midiCh);

        }
        buttonPState[i] = buttonCState[i];
      }
    }
  }
}
