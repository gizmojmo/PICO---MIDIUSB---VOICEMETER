#include <Adafruit_TinyUSB.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <MIDI.h>
#include <U8g2lib.h>

// Créez l'objet MIDI USB avec la bibliothèque TinyUSB
Adafruit_USBD_MIDI usb_midi;

// Paramètres pour les faders et le filtrage
const int ccNumbers[4] = {7, 10, 74, 71};  // Numéros de Control Change pour les 4 potentiomètres
int lastValues[4] = {0, 0, 0, 0};          // Pour stocker les dernières valeurs envoyées
const int threshold = 2;                   // Seuil pour la détection de changement significatif

Adafruit_ADS1015 ads;  // Créez l'objet pour l'ADS1015

// Utilisez U8G2 avec SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE); // SCL = 3, SDA = 2

unsigned long previousMillis = 0;  // Stocke le temps du dernier changement
const unsigned long interval = 5000; // Durée avant d'éteindre l'écran (5 secondes)
bool screenOn = false;  // État actuel de l'écran

float fmap(float x, float a, float b, float c, float d);

void setup() {
  // Initialiser le bus I2C
  Serial.begin(115200);
  Wire.setSDA(4);
  Wire.setSCL(5);
  Wire.begin();

  // Initialiser le deuxième bus I2C pour l'écran SH1106
  Wire1.setSDA(26);
  Wire1.setSCL(27);
  Wire1.begin();


  // Initialiser l'ADS1015
  if (!ads.begin(0x48,&Wire1)) {
    Serial.println("Erreur de connexion à l'ADS1015");
    while (1);
  }

  

  // Initialiser l'écran SH1106
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr); // Exemple de police
  u8g2.setCursor(30, 30);
  u8g2.print("Jaimo MIDI");
  u8g2.sendBuffer();

  // Modifier le nom de l'interface MIDI avant de la démarrer
  TinyUSBDevice.setProductDescriptor("jaimo midi");

  // Initialiser l'interface MIDI USB
  usb_midi.begin();
}

void loop() {

  unsigned long currentMillis = millis();

  // Lire et traiter les messages MIDI reçus
  if (usb_midi.available()) {
    if (!screenOn) {
      u8g2.setPowerSave(0);  // Rallume l'écran si un message MIDI est reçu
      screenOn = true;
    }
    uint8_t packet[4];
    String controle ="";
    usb_midi.readPacket(packet);

    uint8_t type = packet[0] ;  // Type de message MIDI
    uint8_t channel = packet[1] & 0x0F; // Canal MIDI (si nécessaire)
    uint8_t ccNumber = packet[2];     // Numéro du Control Change
    uint8_t value = packet[3];        // Valeur du Control Change
    
    Serial.print(type);
    // Vérifier si c'est un message Control Change (0xB0)
    if (type == 0x0B) {
      Serial.print("Control Change reçu - Numéro : ");
      Serial.print(ccNumber);
      Serial.print(", Valeur : ");
      Serial.println(value);

      // Afficher la valeur sur l'écran
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.setCursor(0, 10);
      //u8g2.print("control");
      
      
      


      // Traiter la valeur reçue selon le numéro de Control Change
      for (int i = 0; i < 4; i++) {
        if (ccNumber == ccNumbers[i]) {
          // Par exemple, on pourrait mapper la valeur reçue à une sortie PWM, un LED, etc.
          // Exemple simple : affichage de la valeur
          Serial.print("Valeur assignée au potentiomètre ");
          Serial.print(i);
          Serial.print(": ");
          float digitvalue;
          switch (i) {
            case 0:
              digitvalue = fmap(value, 0, 127, -60, 12);
              controle = "Volume";
              break;
            case 1:
              digitvalue = fmap(value, 0, 127, -12, 12);
              controle = "Bass";
              break;
            case 2:
              digitvalue = fmap(value, 0, 127, -12, 12);
              controle = "Medium";
              break;
            case 3:
              digitvalue = fmap(value, 0, 127, -12, 12);
              controle = "Treble";
              break;

          }
          u8g2.print(controle);
          u8g2.setCursor(20, 50);
          u8g2.setFont(u8g2_font_ncenB08_tr);
          u8g2.setFont(u8g2_font_ncenB24_tr);
          u8g2.print(digitvalue);
          
          u8g2.sendBuffer(); // Envoyer le buffer à l'écran

          Serial.println(digitvalue);
        }
      }

    }
    previousMillis = currentMillis;  // Réinitialiser le compteur de temps après une activité 
  }

  if (screenOn && (currentMillis - previousMillis >= interval)) {
    u8g2.setPowerSave(1);
    screenOn = false;
  }
  
  // Vous pouvez aussi continuer à lire et envoyer des messages comme avant
  for (int i = 0; i < 4; i++) {
    int16_t faderValue = ads.readADC_SingleEnded(i);
    int midiValue = map(faderValue, 0, 1095, 0, 127);

    if (abs(midiValue - lastValues[i]) > threshold) {
      uint8_t midiData[3] = {
        static_cast<uint8_t>(0xB0 | 0),  // Control Change message on channel 1
        static_cast<uint8_t>(ccNumbers[i]), // Control Change number for the current fader
        static_cast<uint8_t>(midiValue)  // Control Change value
      };

      usb_midi.write(midiData, 3);
      lastValues[i] = midiValue;
    }
  }
}

float fmap(float x, float a, float b, float c, float d)
{
      float f=x/(b-a)*(d-c)+c;
      return f;
}