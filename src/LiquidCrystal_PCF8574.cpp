/// \file LiquidCrystal_PCF8574.cpp
/// \brief LiquidCrystal library with PCF8574 I2C adapter.
///
/// \author Matthias Hertel, http://www.mathertel.de
/// \copyright Copyright (c) 2019 by Matthias Hertel.
///
/// ChangeLog see: LiquidCrystal_PCF8574.h

#include "LiquidCrystal_PCF8574.h"

#include <Wire.h>

LiquidCrystal_PCF8574::LiquidCrystal_PCF8574(uint8_t i2cAddr)
{
  // default pin assignment
  init(i2cAddr, 0, 1, 2, 4, 5, 6, 7, 3);
} // LiquidCrystal_PCF8574

LiquidCrystal_PCF8574::LiquidCrystal_PCF8574(uint8_t i2cAddr, enum LiquidCrystal_PCF8574_type type)
{
  switch (type) {
  case LiquidCrystal_PCF8574_JOY_IT:
    // https://joy-it.net/en/products/RB-LCD-20x4
    init(i2cAddr, 4, 5, 7, 0, 1, 2, 3, 255);
    break;
  case LiquidCrystal_PCF8574_Default:
  default:
    init(i2cAddr, 0, 1, 2, 4, 5, 6, 7, 3);
    break;
  };
} // LiquidCrystal_PCF8574

// constructors, which allows to redefine bit assignments in case your adapter is wired differently
LiquidCrystal_PCF8574::LiquidCrystal_PCF8574(uint8_t i2cAddr, uint8_t rs, uint8_t enable,
    uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t backlight)
{
  init(i2cAddr, rs, 255, enable, d4, d5, d6, d7, backlight);
} // LiquidCrystal_PCF8574

LiquidCrystal_PCF8574::LiquidCrystal_PCF8574(uint8_t i2cAddr, uint8_t rs, uint8_t rw, uint8_t enable,
    uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t backlight)
{
  init(i2cAddr, rs, rw, enable, d4, d5, d6, d7, backlight);
} // LiquidCrystal_PCF8574


void LiquidCrystal_PCF8574::init(uint8_t i2cAddr, uint8_t rs, uint8_t rw, uint8_t enable,
    uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t backlight)
{
  _i2cAddr = i2cAddr;
  _backlight = 0;

  _entrymode = 0x02; // like Initializing by Internal Reset Circuit
  _displaycontrol = 0x04;

  _rs_mask = 0x01 << rs;
  if (rw != 255)
    _rw_mask = 0x01 << rw;
  else
    _rw_mask = 0;
  _enable_mask = 0x01 << enable;
  _data_mask[0] = 0x01 << d4;
  _data_mask[1] = 0x01 << d5;
  _data_mask[2] = 0x01 << d6;
  _data_mask[3] = 0x01 << d7;

  if (backlight != 255)
    _backlight_mask = 0x01 << backlight;
  else
    _backlight_mask = 0;
} // init()


void LiquidCrystal_PCF8574::begin(uint8_t cols, uint8_t lines)
{
  // _cols = cols ignored !
  _lines = lines;

  uint8_t functionFlags = 0;

  if (lines > 1) {
    functionFlags |= 0x08;
  }

  // initializing the display
  Wire.begin();
  _write2Wire(0x00);
  delayMicroseconds(50000);

  // after reset the mode is this
  _displaycontrol = 0x04;
  _entrymode = 0x02;

  // sequence to reset. see "Initializing by Instruction" in datatsheet
  _sendNibble(0x03);
  delayMicroseconds(4500);
  _sendNibble(0x03);
  delayMicroseconds(200);
  _sendNibble(0x03);
  delayMicroseconds(200);
  _sendNibble(0x02);   // finally, set to 4-bit interface

  // Instruction: Function set = 0x20
  _send(0x20 | functionFlags);

  display();
  clear();
  leftToRight();
} // begin()


void LiquidCrystal_PCF8574::clear()
{
  // Instruction: Clear display = 0x01
  _send(0x01);
  delayMicroseconds(1600); // this command takes 1.5ms!
} // clear()


void LiquidCrystal_PCF8574::home()
{
  // Instruction: Return home = 0x02
  _send(0x02);
  delayMicroseconds(1600); // this command takes 1.5ms!
} // home()


/// Set the cursor to a new position.
void LiquidCrystal_PCF8574::setCursor(uint8_t col, uint8_t row)
{
  uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
  // Instruction: Set DDRAM address = 0x80
  _send(0x80 | (row_offsets[row] + col));
} // setCursor()


// Turn the display on/off (quickly)
void LiquidCrystal_PCF8574::noDisplay()
{
  // Instruction: Display on/off control = 0x08
  _displaycontrol &= ~0x04; // display
  _send(0x08 | _displaycontrol);
} // noDisplay()


void LiquidCrystal_PCF8574::display()
{
  // Instruction: Display on/off control = 0x08
  _displaycontrol |= 0x04; // display
  _send(0x08 | _displaycontrol);
} // display()


// Turns the underline cursor on/off
void LiquidCrystal_PCF8574::cursor()
{
  // Instruction: Display on/off control = 0x08
  _displaycontrol |= 0x02; // cursor
  _send(0x08 | _displaycontrol);
} // cursor()


void LiquidCrystal_PCF8574::noCursor()
{
  // Instruction: Display on/off control = 0x08
  _displaycontrol &= ~0x02; // cursor
  _send(0x08 | _displaycontrol);
} // noCursor()


// Turn on and off the blinking cursor
void LiquidCrystal_PCF8574::blink()
{
  // Instruction: Display on/off control = 0x08
  _displaycontrol |= 0x01; // blink
  _send(0x08 | _displaycontrol);
} // blink()


void LiquidCrystal_PCF8574::noBlink()
{
  // Instruction: Display on/off control = 0x08
  _displaycontrol &= ~0x01; // blink
  _send(0x08 | _displaycontrol);
} // noBlink()


// These commands scroll the display without changing the RAM
void LiquidCrystal_PCF8574::scrollDisplayLeft(void)
{
  // Instruction: Cursor or display shift = 0x10
  // shift: 0x08, left: 0x00
  _send(0x10 | 0x08 | 0x00);
} // scrollDisplayLeft()


void LiquidCrystal_PCF8574::scrollDisplayRight(void)
{
  // Instruction: Cursor or display shift = 0x10
  // shift: 0x08, right: 0x04
  _send(0x10 | 0x08 | 0x04);
} // scrollDisplayRight()


// == controlling the entrymode

// This is for text that flows Left to Right
void LiquidCrystal_PCF8574::leftToRight(void)
{
  // Instruction: Entry mode set, set increment/decrement =0x02
  _entrymode |= 0x02;
  _send(0x04 | _entrymode);
} // leftToRight()


// This is for text that flows Right to Left
void LiquidCrystal_PCF8574::rightToLeft(void)
{
  // Instruction: Entry mode set, clear increment/decrement =0x02
  _entrymode &= ~0x02;
  _send(0x04 | _entrymode);
} // rightToLeft()


// This will 'right justify' text from the cursor
void LiquidCrystal_PCF8574::autoscroll(void)
{
  // Instruction: Entry mode set, set shift S=0x01
  _entrymode |= 0x01;
  _send(0x04 | _entrymode);
} // autoscroll()


// This will 'left justify' text from the cursor
void LiquidCrystal_PCF8574::noAutoscroll(void)
{
  // Instruction: Entry mode set, clear shift S=0x01
  _entrymode &= ~0x01;
  _send(0x04 | _entrymode);
} // noAutoscroll()


/// Setting the brightness of the background display light.
/// The backlight can be switched on and off.
/// The current brightness is stored in the private _backlight variable to have it available for further data transfers.
void LiquidCrystal_PCF8574::setBacklight(uint8_t brightness)
{
  _backlight = brightness;
  // send no data but set the background-pin right;
  _write2Wire(0x00);
} // setBacklight()


// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LiquidCrystal_PCF8574::createChar(uint8_t location, byte charmap[])
{
  location &= 0x7; // we only have 8 locations 0-7
  // Set CGRAM address
  _send(0x40 | (location << 3));
  for (int i = 0; i < 8; i++) {
    write(charmap[i]);
  }
} // createChar()


// Allows us to fill the first 8 CGRAM locations
// with custom characters stored in PROGMEM
void LiquidCrystal_PCF8574::createCharPgm(uint8_t location, const byte *charmap) {
  PGM_P p = reinterpret_cast<PGM_P>(charmap);
  location &= 0x7; // we only have 8 locations 0-7
  _send(0x40 | (location << 3));
  for (int i = 0; i < 8; i++) {
    byte c = pgm_read_byte(p++);
    write(c);
  }
} // createCharPgm()


/* The write function is needed for derivation from the Print class. */
inline size_t LiquidCrystal_PCF8574::write(uint8_t ch)
{
  _send(ch, true);
  return 1; // assume success
} // write()


size_t LiquidCrystal_PCF8574::write(const uint8_t *buffer, size_t size) {
  size_t n = size;
  uint8_t out, out1;
  uint8_t c = 0;

  out = _rs_mask;  // RS==HIGH
  if (_backlight > 0)
    out |= _backlight_mask;
  out1 = out;

  while (size--) {
    byte value = *buffer++;

    out = out1;
    if (value & 0x10) out |= _data_mask[0];
    if (value & 0x20) out |= _data_mask[1];
    if (value & 0x40) out |= _data_mask[2];
    if (value & 0x80) out |= _data_mask[3];

    // pulse enable
    if (c == 0) {
      Wire.beginTransmission(_i2cAddr);
    }
    Wire.write(out | _enable_mask);
    Wire.write(out);

    out = out1;
    if (value & 0x01) out |= _data_mask[0];
    if (value & 0x02) out |= _data_mask[1];
    if (value & 0x04) out |= _data_mask[2];
    if (value & 0x08) out |= _data_mask[3];

    // pulse enable
    Wire.write(out | _enable_mask);
    Wire.write(out);
    c += 4;
    if (c >= BUFFER_LENGTH - 4) {
      // We only restart the transmission once the buffer is full.
      Wire.endTransmission();
      c = 0;
    }
  }
  if (c != 0) Wire.endTransmission();
  return n;
}


// write either command or data
void LiquidCrystal_PCF8574::_send(uint8_t value, bool isData)
{
  uint8_t out = 0, out1;

  if (_backlight > 0)
    out |= _backlight_mask;
  if (isData)
    out |= _rs_mask;

  out1 = out;
  if (value & 0x10) out |= _data_mask[0];
  if (value & 0x20) out |= _data_mask[1];
  if (value & 0x40) out |= _data_mask[2];
  if (value & 0x80) out |= _data_mask[3];

  // pulse enable
  Wire.beginTransmission(_i2cAddr);
  Wire.write(out | _enable_mask);
  Wire.write(out);

  out = out1;
  if (value & 0x01) out |= _data_mask[0];
  if (value & 0x02) out |= _data_mask[1];
  if (value & 0x04) out |= _data_mask[2];
  if (value & 0x08) out |= _data_mask[3];

  // pulse enable
  Wire.write(out | _enable_mask);
  Wire.write(out);
  Wire.endTransmission();
} // _send()


// write a nibble / halfByte with handshake
void LiquidCrystal_PCF8574::_sendNibble(uint8_t value, bool isData)
{
  // map the given values to the hardware of the I2C schema
  uint8_t out = 0;
  if (isData)
    out |= _rs_mask;
  // _rw_mask is not used here.
  if (_backlight > 0)
    out |= _backlight_mask;

  if (value & 0x01) out |= _data_mask[0];
  if (value & 0x02) out |= _data_mask[1];
  if (value & 0x04) out |= _data_mask[2];
  if (value & 0x08) out |= _data_mask[3];

  Wire.beginTransmission(_i2cAddr);
  // pulse enable
  Wire.write(out | _enable_mask);
  Wire.write(out);
  Wire.endTransmission();
} // _sendNibble


// private function to change the PCF8674 pins to the given value
void LiquidCrystal_PCF8574::_write2Wire(uint8_t byte)
{
  uint8_t out = _rs_mask;
  if (_backlight > 0)
    out |= _backlight_mask;
  Wire.beginTransmission(_i2cAddr);
  Wire.write(out);
  Wire.endTransmission();
} // write2Wire

// The End.
