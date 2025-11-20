// Arduino B

#include <IRremote.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define PIN_RECV 12
#define PIN_SEND 11

LiquidCrystal_I2C lcd(0x27, 16, 2);

char mapHEXToKey(int hex);

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
    
      uint8_t address = IrReceiver.decodedIRData.address;
      uint8_t message = IrReceiver.decodedIRData.command;

    if (proto == SAMSUNG) {
      Serial.println("Samsung detected → Sending NEC handshake");
      Serial.print("Message:");
      Serial.println(message);
      Serial.print("Address: ");
      Serial.println(address);


      // --- Send two NEC signals ---
      IrSender.sendNEC(0x0102, 0x34, 0);
      delay(300);
      IrSender.sendNEC(0x0102, 0x34, 0);

      Serial.println("NEC handshake sent");
    } 
    else {
      Serial.print("NEC Message:");
      Serial.println(message);
      Serial.print("Address: ");
      Serial.println(address);

      if (address==2 && message!=52) {
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
    }

    IrReceiver.resume();
  }
}

char mapHEXToKey(int hex) {
  switch (hex) {
    case 0: return '0';
    case 1: return '1';
    case 2: return '2';
    case 3: return '3';
    case 4: return '4';
    case 5: return '5';
    case 6: return '6';
    case 7: return '7';
    case 8: return '8';
    case 9: return '9';
    case 10: return '*';
    case 11: return '#';
    case 12: return 'A';
    case 13: return 'B';
    case 14: return 'C';
    case 15: return 'D';
    default:
      return '?';  // invalid hex
  }
}