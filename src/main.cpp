#include <Adafruit_TinyUSB.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <MIDI.h>
#include <U8g2lib.h>

// Create the USB MIDI object with the TinyUSB library
Adafruit_USBD_MIDI usb_midi;

// Parameters for faders and filtering
const int ccNumbers[4] = {7, 10, 74, 71};  // Control Change numbers for the 4 potentiometers
int lastValues[4] = {0, 0, 0, 0};          // Store the last values sent
const int threshold = 2;                   // Threshold for detecting significant change
const unsigned long screenTimeout = 5000;  // Duration before screen turns off (5 seconds)

Adafruit_ADS1015 ads;  // ADS1015
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE); 

unsigned long previousMillis = 0;          // Time of the last change
bool screenOn = false;                     // Screen state

// Function declarations
void processMidiInput();
void updateDisplay(const String &control, float value);
void handleFaderInput();
float fmap(float x, float in_min, float in_max, float out_min, float out_max);
void resetScreenTimeout();

void setup() {
  // Initialize the I2C bus for the SH1106 screen
  /*
  Serial.begin(115200);
  */
  Serial.end(); // Explicitly disable the USB serial port (CDC)
  Wire.setSDA(4);
  Wire.setSCL(5);
  Wire.begin();

  // Initialize the second I2C bus for the ADS1015
  Wire1.setSDA(26);
  Wire1.setSCL(27);
  Wire1.begin();

  // Initialize the ADS1015
  if (!ads.begin(0x48, &Wire1)) {
    Serial.println("Error connecting to ADS1015");
    while (1);
  }

  // Initialize the SH1106 screen
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr); // Example font
  u8g2.clearBuffer();
  u8g2.setCursor(30, 30);
  u8g2.print("Jaimo MIDI");
  u8g2.sendBuffer();  // Send the buffer to the screen

  // Change the name of the MIDI interface
  TinyUSBDevice.setProductDescriptor("Jaimo MIDI");

  // Initialize the USB MIDI interface
  usb_midi.begin();
}

void loop() {
  //unsigned long currentMillis = millis();

  // Process incoming MIDI messages
  processMidiInput();
  unsigned long currentMillis = millis();
  
  // Manage the screen (turn off after inactivity)
  if (screenOn && (currentMillis - previousMillis >= screenTimeout)) {
    u8g2.setPowerSave(1);  // Turn off the screen
    screenOn = false;
  }
  
  // Process the faders
  handleFaderInput();
}

// Function to process MIDI input
void processMidiInput() {
  if (usb_midi.available()) {
    Serial.print("ScreenOn: ");
    Serial.println(screenOn);
    if (!screenOn) {
      u8g2.setPowerSave(0);
      Serial.println("activating screen");
      screenOn = true;
    }

    uint8_t packet[4];
    usb_midi.readPacket(packet);

    // Display the contents of the received MIDI packet for debugging
    Serial.print("MIDI Packet received: ");
    for (int i = 0; i < 4; i++) {
      Serial.print(packet[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    uint8_t type = packet[1] & 0xF0;  // MIDI message type
    uint8_t channel = packet[1] & 0x0F;  // MIDI channel
    uint8_t ccNumber = packet[2];  // Control Change number
    uint8_t value = packet[3];  // Control Change value

    Serial.print("Type: 0x");
    Serial.print(type, HEX);
    Serial.print(", Channel: ");
    Serial.print(channel);
    Serial.print(", CC Number: ");
    Serial.print(ccNumber);
    Serial.print(", Value: ");
    Serial.println(value);

    // Check if it's a Control Change message (0xB0)
    if (type == 0xB0) {
      Serial.println("Control Change received");

      for (int i = 0; i < 4; i++) {
        if (ccNumber == ccNumbers[i]) {
          String control;
          float digitValue;

          switch (i) {
            case 0: control = "Volume"; digitValue = fmap(value, 0, 127, -60, 12); break;
            case 1: control = "Bass"; digitValue = fmap(value, 0, 127, -12, 12); break;
            case 2: control = "Mid"; digitValue = fmap(value, 0, 127, -12, 12); break;
            case 3: control = "Treble"; digitValue = fmap(value, 0, 127, -12, 12); break;
          }

          // Update the screen with the new values
          resetScreenTimeout();
          updateDisplay(control, digitValue);

          Serial.print("Control: ");
          Serial.print(control);
          Serial.print(", Mapped Value: ");
          Serial.println(digitValue);
        }
      }

      resetScreenTimeout();
    } else {
      Serial.println("Unhandled MIDI message");
    }
  }
}

// Function to update the display
void updateDisplay(const String &control, float value) {
  u8g2.clearBuffer();  // Clear the screen before displaying new data
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(0, 10);
  u8g2.print(control);

  u8g2.setCursor(20, 50);
  u8g2.setFont(u8g2_font_ncenB24_tr);
  u8g2.print(value);
  u8g2.sendBuffer();  // Send the buffer to the screen
}

// Function to process fader input
void handleFaderInput() {
  for (int i = 0; i < 4; i++) {
    int16_t faderValue = ads.readADC_SingleEnded(i);
    int midiValue = map(faderValue, 0, 1095, 0, 127);

    if (abs(midiValue - lastValues[i]) > threshold) {
      uint8_t midiData[3] = {
        static_cast<uint8_t>(0xB0 | 0),  // Control Change on channel 1
        static_cast<uint8_t>(ccNumbers[i]),  // Control Change number
        static_cast<uint8_t>(midiValue)  // Control Change value
      };

      usb_midi.write(midiData, 3);
      lastValues[i] = midiValue;
    }
  }
}

// Map a floating point value to a new range
float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Reset the screen timeout delay
void resetScreenTimeout() {
  previousMillis = millis();
}
