#include <Adafruit_TinyUSB.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <MIDI.h>
#include <U8g2lib.h>

// Créez l'objet MIDI USB avec la bibliothèque TinyUSB
Adafruit_USBD_MIDI usb_midi;

// Paramètres pour les faders et le filtrage
const int ccNumbers[4] = {7, 10, 74, 71};  // Numéros de Control Change pour les 4 potentiomètres
int lastValues[4] = {0, 0, 0, 0};          // Stocker les dernières valeurs envoyées
const int threshold = 2;                   // Seuil de détection de changement significatif
const unsigned long screenTimeout = 5000;  // Durée avant extinction de l'écran (5 secondes)

Adafruit_ADS1015 ads;  // ADS1015
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE); // SCL = 3, SDA = 2

unsigned long previousMillis = 0;          // Temps du dernier changement
bool screenOn = false;                     // État de l'écran

// Déclaration des fonctions
void processMidiInput();
void updateDisplay(const String &controle, float value);
void handleFaderInput();
float fmap(float x, float in_min, float in_max, float out_min, float out_max);
void resetScreenTimeout();

void setup() {
  // Initialiser le bus I2C pour l'ADS1015
  /*
  Serial.begin(115200);
  */
  Serial.end(); // Désactive explicitement le port série USB (CDC)
  Wire.setSDA(4);
  Wire.setSCL(5);
  Wire.begin();

  // Initialiser le deuxième bus I2C pour l'écran SH1106
  Wire1.setSDA(26);
  Wire1.setSCL(27);
  Wire1.begin();

  // Initialiser l'ADS1015
  if (!ads.begin(0x48, &Wire1)) {
    Serial.println("Erreur de connexion à l'ADS1015");
    while (1);
  }

  // Initialiser l'écran SH1106
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr); // Exemple de police
  u8g2.clearBuffer();
  u8g2.setCursor(30, 30);
  u8g2.print("Jaimo MIDI");
  u8g2.sendBuffer();  // Envoyer le buffer à l'écran

  // Modifier le nom de l'interface MIDI
  TinyUSBDevice.setProductDescriptor("Jaimo MIDI");

  // Initialiser l'interface MIDI USB
  usb_midi.begin();
}

void loop() {

  
  //unsigned long currentMillis = millis();

  // Traitement des messages MIDI entrants
  processMidiInput();
  unsigned long currentMillis = millis();
  // Gestion de l'écran (extinction après inactivité)
  
  if (screenOn && (currentMillis - previousMillis >= screenTimeout)) {
    u8g2.setPowerSave(1);  // Éteindre l'écran
    screenOn = false;
  }
  
  // Traitement des faders
  handleFaderInput();
}

// Fonction pour traiter l'entrée MIDI
void processMidiInput() {
  if (usb_midi.available()) {
    Serial.print("ScreeOn : ");
    Serial.println(screenOn);
    if (!screenOn) {
      u8g2.setPowerSave(0);
      Serial.println("activation écran");
      screenOn = true;
    }

    uint8_t packet[4];
    usb_midi.readPacket(packet);

    // Affichage du contenu du paquet MIDI reçu pour débogage
    Serial.print("Paquet MIDI reçu: ");
    for (int i = 0; i < 4; i++) {
      Serial.print(packet[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    uint8_t type = packet[1] & 0xF0;  // Type de message MIDI
    uint8_t channel = packet[1] & 0x0F;  // Canal MIDI
    uint8_t ccNumber = packet[2];  // Numéro du Control Change
    uint8_t value = packet[3];  // Valeur du Control Change

    Serial.print("Type: 0x");
    Serial.print(type, HEX);
    Serial.print(", Canal: ");
    Serial.print(channel);
    Serial.print(", CC Numéro: ");
    Serial.print(ccNumber);
    Serial.print(", Valeur: ");
    Serial.println(value);

    // Vérifier si c'est un message Control Change (0xB0)
    if (type == 0xB0) {
      Serial.println("Control Change reçu");

      for (int i = 0; i < 4; i++) {
        if (ccNumber == ccNumbers[i]) {
          String controle;
          float digitValue;

          switch (i) {
            case 0: controle = "Volume"; digitValue = fmap(value, 0, 127, -60, 12); break;
            case 1: controle = "Bass"; digitValue = fmap(value, 0, 127, -12, 12); break;
            case 2: controle = "Medium"; digitValue = fmap(value, 0, 127, -12, 12); break;
            case 3: controle = "Treble"; digitValue = fmap(value, 0, 127, -12, 12); break;
          }

          // Mettre à jour l'écran avec les nouvelles valeurs
          resetScreenTimeout();
          updateDisplay(controle, digitValue);

          Serial.print("Contrôle: ");
          Serial.print(controle);
          Serial.print(", Valeur mappée: ");
          Serial.println(digitValue);
        }
      }

      resetScreenTimeout();
    } else {
      Serial.println("Message MIDI non traité");
    }
  }
}



// Fonction pour mettre à jour l'affichage

void updateDisplay(const String &controle, float value) {
  u8g2.clearBuffer();  // Nettoyer l'écran avant d'afficher les nouvelles données
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(0, 10);
  u8g2.print(controle);

  u8g2.setCursor(20, 50);
  u8g2.setFont(u8g2_font_ncenB24_tr);
  u8g2.print(value);
  u8g2.sendBuffer();  // Envoyer le buffer à l'écran
}




// Fonction pour traiter l'entrée des faders
void handleFaderInput() {
  for (int i = 0; i < 4; i++) {
    int16_t faderValue = ads.readADC_SingleEnded(i);
    int midiValue = map(faderValue, 0, 1095, 0, 127);

    if (abs(midiValue - lastValues[i]) > threshold) {
      uint8_t midiData[3] = {
        static_cast<uint8_t>(0xB0 | 0),  // Control Change sur le canal 1
        static_cast<uint8_t>(ccNumbers[i]),  // Numéro du Control Change
        static_cast<uint8_t>(midiValue)  // Valeur du Control Change
      };

      usb_midi.write(midiData, 3);
      lastValues[i] = midiValue;
    }
  }
}

// Mapper une valeur flottante dans une nouvelle plage
float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Réinitialiser le délai de mise en veille de l'écran
void resetScreenTimeout() {
  previousMillis = millis();
}
