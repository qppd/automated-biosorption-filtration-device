#ifndef LCD_MODULE_H
#define LCD_MODULE_H

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include "PINS_CONFIG.h"  // I2C_SDA_PIN / I2C_SCL_PIN
#include "CONFIG.h"        // LCD_I2C_ADDR / LCD_COLS / LCD_ROWS

// The LCD object is defined in LCD_MODULE.cpp and declared extern here so
// the header can be included from any translation unit.
extern LiquidCrystal_I2C lcd;

void lcd_init();
void lcd_display_message(const char* message, uint8_t row = 0);
void lcd_clear();

// Writes a row only when its text differs from what is already displayed.
// Use this for live values (metered volume, countdown) so the display does not
// flicker and the I2C bus is not written on every loop pass.
void lcd_write_row(uint8_t row, const char* text);

// Write text at column x, row y without clearing the whole display.
// x: 0..LCD_COLS-1, y: 0..LCD_ROWS-1; out-of-range values are clamped.
inline void setLCDText(const String &text, uint8_t x, uint8_t y) {
  if (x >= LCD_COLS) x = LCD_COLS - 1;
  if (y >= LCD_ROWS) y = LCD_ROWS - 1;
  lcd.setCursor(x, y);
  lcd.print(text);
  // Pad the rest of the row with spaces to clear previous content
  for (uint8_t i = text.length() + x; i < LCD_COLS; ++i) {
    lcd.print(' ');
  }
}

// Integer overloads
inline void setLCDText(int value, uint8_t x, uint8_t y) {
  setLCDText(String(value), x, y);
}

inline void setLCDText(long value, uint8_t x, uint8_t y) {
  setLCDText(String(value), x, y);
}

// Floating-point overloads; decimals defaults to 2. The cast selects the
// decimal-places String constructor, which is otherwise ambiguous on the
// ESP32 core.
inline void setLCDText(float value, uint8_t x, uint8_t y, unsigned char decimals = 2) {
  setLCDText(String(value, (unsigned int)decimals), x, y);
}

inline void setLCDText(double value, uint8_t x, uint8_t y, unsigned char decimals = 2) {
  setLCDText(String(value, (unsigned int)decimals), x, y);
}

#endif // LCD_MODULE_H
