//
// Created by tiago on 11/26/2019.
//
#include "GrooveLCD.h"

void setRGB(unsigned char r, unsigned char g, unsigned char b) {
  setReg(REG_RED, r);
  setReg(REG_GREEN, g);
  setReg(REG_BLUE, b);
}
void setReg(unsigned char addr, unsigned char dta) {
  static const i2c_e i2c_bus = I2C__2;
  i2c__write_single(i2c_bus, RGB_ADDRESS, addr, dta);
}

void display() {
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void clear() {
  command(LCD_CLEARDISPLAY); // clear display, set cursor position to zero
  delay__us(2000);           // this command takes a long time!
}

void rgb_lcd_begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  _displayfunction = 0;
  _displaycontrol = 0;
  _displaymode = 0;
  _initialized = 0;

  if (lines > 1) {
    _displayfunction |= LCD_2LINE;
  }
  _numlines = lines;
  _currline = 0;

  // for some 1 line displays you can select a 10 pixel high font
  /*if ((dotsize != 0) && (lines == 1)) {
    _displayfunction |= LCD_5x10DOTS;
  }*/
  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
  // according to datasheet, we need at least 40ms after power rises above 2.7V
  // before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
  delay__us(50000);
  // this is according to the hitachi HD44780 datasheet
  // page 45 figure 23

  // Send function set command sequence
  // printf("Value: %x, %x\n", LCD_FUNCTIONSET, _displayfunction);
  command(LCD_FUNCTIONSET | _displayfunction);
  delay__us(4500); // wait more than 4.1ms

  // second try
  command(LCD_FUNCTIONSET | _displayfunction);
  delay__us(150);

  // third go
  command(LCD_FUNCTIONSET | _displayfunction);

  // finally, set # lines, font size, etc.
  command(LCD_FUNCTIONSET | _displayfunction);

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
  display();

  // clear it off
  clear();

  // Initialize to default text direction (for romance languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  command(LCD_ENTRYMODESET | _displaymode);

  // backlight init
  setReg(REG_MODE1, 0);
  // set LEDs controllable by both PWM and GRPPWM registers
  setReg(REG_OUTPUT, 0xFF);
  // set MODE2 values
  // 0010 0000 -> 0x20  (DMBLNK to 1, ie blinky mode)
  setReg(REG_MODE2, 0x20);

  setRGB(0, 0, 0);
  // setColorWhite();
}

//-------------------------------------------------------------
void command(uint8_t value) {
  static const i2c_e i2c_bus = I2C__2;
  const uint8_t temp = value;
  // printf("sending value: 0x%x\n", temp);
  // i2c__write_slave_data(i2c_bus, LCD_ADDRESS, 0x80, temp, 1);
  i2c__write_single(i2c_bus, LCD_ADDRESS, 0x80, temp);
}

void i2c_send_byteS(unsigned char *dta, unsigned char len) {
  static const i2c_e i2c_bus = I2C__2;
  const uint8_t dummy_register = 0;
  i2c__write_slave_data(i2c_bus, LCD_ADDRESS, dummy_register, &dta, len);
}

void i2c_send_byte(unsigned char dta) {

  static const i2c_e i2c_bus = I2C__2;
  const uint8_t dummy_register = 0;
  i2c__write_slave_data(i2c_bus, LCD_ADDRESS, dummy_register, &dta, 1);
}

void sendchar(char value) {
  static const i2c_e i2c_bus = I2C__2;
  const uint8_t temp = (uint8_t)value;
  // printf("sending char: 0x%x\n", temp);
  // i2c__write_slave_data(i2c_bus, LCD_ADDRESS, 0x80, temp, 1);
  i2c__write_single(i2c_bus, LCD_ADDRESS, 0x40, temp);
}