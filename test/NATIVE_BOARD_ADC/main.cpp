
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>

#include <MIDI.h>

// MIDI
Adafruit_USBD_MIDI usbdMidi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usbdMidi, MIDI);

const int faderPin = A0;  // Pin analogique où le fader est connecté
int lastValue = 0;        // Pour stocker la dernière valeur du fader
const int ccNumber = 7;    // Numéro du Control Change (7 = volume par défaut)
const int threshold = 2; // Seuil de variation

void setup() {
  // Initialisation de l'USB
  MIDI.begin(4);
  usbdMidi.begin();
}

void loop() {
  int faderValue = analogRead(faderPin); // Lire la valeur du fader

  // Mapper la valeur de 0-1023 à 0-127 (plage MIDI CC)
  int midiValue = map(faderValue, 0, 1023, 0, 127);

  // Si la valeur a changé depuis la dernière lecture, envoyer un message MIDI
  /*
  if (midiValue != lastValue) {
    MIDI.sendControlChange(ccNumber, midiValue, 1); // Envoie du CC sur le canal 1
    lastValue = midiValue;
  }
  */

  if (abs(midiValue - lastValue) > threshold) { // Vérifier si la variation est significative
    MIDI.sendControlChange(ccNumber, midiValue, 1);
    lastValue = midiValue;
  }
  delay(10); // Petite pause pour éviter d'inonder le bus MIDI
}