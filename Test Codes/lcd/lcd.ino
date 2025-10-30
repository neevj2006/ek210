#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Set the LCD I2C address (common addresses are 0x27 or 0x3F)
// You may need to find the correct address for your specific module.
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the address, 16 columns, 2 rows

void setup() {
  lcd.init();                      // Initialize the LCD
  lcd.backlight();                 // Turn on the backlight
}

void loop() {
  // You can add more code here to update the display
  // For example, displaying sensor readings or changing messages.
  lcd.setCursor(0, 1); // Set cursor to the second line (row 1)
  lcd.print("Arduino I2C");
  delay(2000);
  lcd.clear();
}