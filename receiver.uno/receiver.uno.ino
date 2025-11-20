// Arduino B

#include <IRremote.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define PIN_RECV 12
#define PIN_SEND 11

LiquidCrystal_I2C lcd(0x27, 16, 2);

char mapHEXToKey(uint8_t hex);

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(PIN_RECV);
  IrSender.begin(PIN_SEND);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Arduino B Ready");
  Serial.println("Arduino B Ready");
}

void loop() {

  if (IrReceiver.decode()) {

    uint8_t proto = IrReceiver.decodedIRData.protocol;

    if (proto == SAMSUNG) {
      Serial.println("Samsung detected → Sending NEC handshake");

      uint8_t address = IrReceiver.decodedIRData.address;
      uint8_t message = IrReceiver.decodedIRData.command;
      if (address == 0x0708 && message != 0x55) {
        Serial.print("Codeword Hex: ");
        Serial.println(message);
        char translatedMessage = mapHEXToKey(message);
        Serial.print("Codeword: ");
        Serial.println(translatedMessage);

        // Display on LCD
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Code Received:");
        lcd.setCursor(0,1);
        lcd.print(translatedMessage);
      }


      // --- Send two NEC signals ---
      IrSender.sendNEC(0x0102, 0x34, 0);
      delay(300);
      IrSender.sendNEC(0x0102, 0x34, 0);

      Serial.println("NEC handshake sent");
    }

    IrReceiver.resume();
  }
}

char mapHEXToKey(uint8_t hex) {
  switch (hex) {
    case 0x00: return '0';
    case 0x01: return '1';
    case 0x02: return '2';
    case 0x03: return '3';
    case 0x04: return '4';
    case 0x05: return '5';
    case 0x06: return '6';
    case 0x07: return '7';
    case 0x08: return '8';
    case 0x09: return '9';
    case 0x0A: return '*';
    case 0x0B: return '#';
    case 0x0C: return 'A';
    case 0x0D: return 'B';
    case 0x0E: return 'C';
    case 0x0F: return 'D';
    default:
      return '?';  // invalid hex
  }
}

