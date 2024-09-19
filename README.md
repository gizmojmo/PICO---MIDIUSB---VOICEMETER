MIDI Controller for Volume, Bass, Mids, and Treble
This code was created for my personal needs. I use Voicemeeter Banana and wanted to control the volume and tone of the "aux 1" channel using potentiometers.

I opted to use 4 potentiometers and an OLED display to get feedback on the level of each potentiometer as I adjust them.

Hardware used:
1 Raspberry Pi Pico
1 SH1106 OLED display
1 ADS1015 module
4 (mono) 50 K potentiometers

Wiring diagram:

![Cover](https://github.com/gizmojmo/PICO---MIDIUSB---VOICEMETER/blob/main/pictures/Pico-breadboard-midi-usb_bb.jpg)

Each potentiometer should be connected to the ADS1015 through its center pin as follows:


| Potentiometer | ads1015 |
|---------------|---------|
| p1            | A0      |
| p2            | A1      |
| p3            | A2      |
| p4            | A3      |

All potentiometers should be connected to 3V on the right pin and to GND on the left pin.

SH1106 display wiring:

| SH1106 | RASPBERRY PICO |
|--------|----------------|
| VCC    | 3V             |
| GND    | GND            |
| SCL    | GPIO 5         |
| SDA    | GPIO 4         |

ADS1015 wiring:

| ADS1015 | RASPBERRY PICO | Potentiomètre Broche centrale |
|---------|----------------|-------------------------------|
| VDD     | 3v             |                               |
| GND     | GND            |                               |
| SCL     | GPIO 27        |                               |
| SDA     | GPIO 26        |                               |
| ADDR    | -              |                               |
| ALRT    | -              |                               |
| A0      |                | p1                            |
| A1      |                | p2                            |
| A2      |                | p3                            |
| A3      |                | p4                            |

Libraries used:
Adafruit TinyUSB 3.3.3
Adafruit ADS1X15 2.5.0
U8g2 2.35.21
MIDI Interface Configuration in Voicemeeter (Banana):

![Cover](https://github.com/gizmojmo/PICO---MIDIUSB---VOICEMETER/blob/main/pictures/voicemeter_config.jpg)


Go to Menu/MIDI Mapping.
In the "M.I.D.I. Input Device" field, select the MIDI interface.
Do the same in the "M.I.D.I. Output Device (feedback)" field, otherwise no feedback will appear on the screen.
Then select the command to control, click "Learn," and move the potentiometer to assign. A value should appear (e.g., "#1 Ctrl Ch. 7"). Then turn off the "Learn" button.
Finally, turn on the "F" button by clicking it multiple times until you get the value "FF."
Repeat step 4 for each potentiometer you want to configure.
Save the configuration with the "Save" button, then close the "M.I.D.I. Mapping" window.
As I mentioned at the beginning of this document, this interface meets a specific need, but you can add control buttons, for example.

For those using PlatFormIO, the platform.ini file is provided.

WARNING: If you need to reprogram the Raspberry Pi Pico, remember to use the BOOTSEL button. In the program, I have intentionally disabled Serial port management (using Serial.end in void setup) to prevent any accidental reprogramming, as the interface is meant to stay connected to your computer.