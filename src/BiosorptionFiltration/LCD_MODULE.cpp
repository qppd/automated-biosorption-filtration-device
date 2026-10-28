#include "LCD_MODULE.h"
#include <string.h>

// LCD object definition (declared extern in LCD_MODULE.h)
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);

// Last text written to each row. lcd_write_row() compares against this so a
// value that has not changed is not sent over I2C again.
static char row_cache[LCD_ROWS][LCD_COLS + 1];

void lcd_init() {
  // LiquidCrystal_I2C::init() calls Wire.begin() internally, so the pins must
  // be selected beforehand. Calling Wire.begin() here would be overwritten
  // with the core defaults (21/22) and PINS_CONFIG.h would be ignored.
  Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  memset(row_cache, 0, sizeof row_cache);
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
  memset(row_cache, 0, sizeof row_cache);
}

void lcd_write_row(uint8_t row, const char* text) {
  if (row >= LCD_ROWS) return;

  // Clip to the display width first, otherwise a longer string would compare
  // unequal on every call and be rewritten continuously.
  char clipped[LCD_COLS + 1];
  strncpy(clipped, text, LCD_COLS);
  clipped[LCD_COLS] = '\0';

  if (strcmp(row_cache[row], clipped) == 0) return;

  strcpy(row_cache[row], clipped);
  setLCDText(clipped, 0, row);  // pads the rest of the row with spaces
}
