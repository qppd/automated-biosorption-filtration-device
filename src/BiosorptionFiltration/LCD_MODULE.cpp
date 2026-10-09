#include "LCD_MODULE.h"

// LCD object definition (declared extern in LCD_MODULE.h)
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);

void lcd_init() {
  // Explicit ESP32 I2C pins from PINS_CONFIG.h (defaults: SDA 21 / SCL 22)
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void lcd_display_message(const char* message, uint8_t row) {
  lcd.setCursor(0, row);
  lcd.print(message);
  // Clear remaining characters on the line
  for (uint8_t i = strlen(message); i < LCD_COLS; ++i) {
    lcd.print(' ');
  }
}

void lcd_clear() {
  lcd.clear();
}