#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_TinyUSB.h>

// Créez l'objet MIDI USB avec la bibliothèque TinyUSB
Adafruit_USBD_MIDI usb_midi;

// Paramètres pour le fader et le filtrage
const int ccNumber = 7;  // Numéro du Control Change (7 = volume par défaut)
int lastValue = 0;       // Pour stocker la dernière valeur envoyée
const int threshold = 2;

Adafruit_ADS1015 ads;  // Créez l'objet pour l'ADS1015

void setup() {
  // Initialiser le bus I2C
  Serial.begin(115200);
  Wire.setSDA(0);
  Wire.setSCL(1);
  Wire.begin();

  // Initialiser l'ADS1015
  if (!ads.begin()) {
    Serial.println("Erreur de connexion à l'ADS1015");
    while (1);
  }

  // Initialiser l'interface MIDI USB
  usb_midi.begin();
}

void loop() {
  // Lire la valeur analogique du fader depuis l'ADS1015 (canal 0)
  int16_t faderValue = ads.readADC_SingleEnded(0);

  // Mapper la valeur de 0-2047 (12 bits) à 0-127 (plage MIDI CC)
  int midiValue = map(faderValue, 0, 1095, 0, 127);

  // Envoyer le message MIDI uniquement si la variation est significative
  if (abs(midiValue - lastValue) > threshold) {
    // Construire le message MIDI Control Change
    uint8_t midiData[3] = {
      static_cast<uint8_t>(0xB0 | 0), // Control Change message on channel 1
      static_cast<uint8_t>(ccNumber), // Control Change number
      static_cast<uint8_t>(midiValue) // Control Change value
    };

    // Envoyer le message MIDI via USB
    usb_midi.write(midiData, 3);
    lastValue = midiValue;
  }

  delay(10); // Petite pause pour éviter d'inonder le bus MIDI
}
