/// \file LiquidCrystal_PCF8574.h
/// \brief LiquidCrystal library with PCF8574 I2C adapter.
///
/// \author Matthias Hertel, http://www.mathertel.de
///
/// \copyright Copyright (c) 2019 by Matthias Hertel.\n
///
/// The library work is licensed under a BSD style license.\n
/// See http://www.mathertel.de/License.aspx
///
/// \details
/// This library can drive a Liquid Cristal display based on the Hitachi HD44780 chip that is connected
/// through a PCF8574 I2C adapter. It uses the original Wire library for communication.
/// The API if common to many LCD libraries and documented in https://www.arduino.cc/en/Reference/LiquidCrystal.
/// and partially functions from https://playground.arduino.cc/Code/LCDAPI/.

///
/// ChangeLog:
/// --------
/// * 19.10.2013 created.
/// * 05.06.2019 rewrite from scratch.
/// * 26.06.2020 BM:
/// *   Speed-up by about a factor of three by using optimized I2C requests
/// *   New constructors allow flexible pin assignments.
/// *   New constructor for known display types.
/// *   Replace int parameters by uint8_t where applicable
/// *   Add variant createCharPgm() which retrieves data from PROGMEM
/// *   clear() and home() wait for the display's busy signal (if rw is available)

#ifndef LiquidCrystal_PCF8574_h
#define LiquidCrystal_PCF8574_h

#include "Arduino.h"
#include "Print.h"
#include <stddef.h>
#include <stdint.h>

enum LiquidCrystal_PCF8574_type {
    LiquidCrystal_PCF8574_Default, LiquidCrystal_PCF8574_JOY_IT
};


class LiquidCrystal_PCF8574 : public Print
{
public:
  LiquidCrystal_PCF8574(uint8_t i2cAddr);
  // note:
  // There is no sda and scl parameter for i2c in any api.
  // The Wire library has standard settings that can be overwritten by using Wire.begin(int sda, int scl) before calling LiquidCrystal_PCF8574::begin();

  // Choose pin assignments from a list of known modules
  LiquidCrystal_PCF8574(uint8_t i2cAddr, enum LiquidCrystal_PCF8574_type type);

  // constructors, which allows to redefine bit assignments in case your adapter is wired differently
  LiquidCrystal_PCF8574(uint8_t i2cAddr, uint8_t rs, uint8_t enable,
    uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t backlight=255);
  LiquidCrystal_PCF8574(uint8_t i2cAddr, uint8_t rs, uint8_t rw, uint8_t enable,
    uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t backlight=255);

  // Functions from reference:

  void begin(uint8_t cols, uint8_t rows);

  void clear();
  void home();
  void setCursor(uint8_t col, uint8_t row);
  void cursor();
  void noCursor();
  void blink();
  void noBlink();
  void display();
  void noDisplay();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void autoscroll();
  void noAutoscroll();
  void leftToRight();
  void rightToLeft();
  void createChar(uint8_t, byte[]);

#ifdef __AVR__
  // own additions
  void createCharPgm(uint8_t, const byte *);
  inline void createChar(uint8_t n, const byte *data) {
    createCharPgm(n, data);
  };
#endif

  // plus functions from LCDAPI:
  void setBacklight(uint8_t brightness);
  inline void command(uint8_t value) { _send(value); }

  // support of Print class
  virtual size_t write(uint8_t ch);
  virtual size_t write(const uint8_t *buffer, size_t size);

  // helper functions
  int waitBusy();

private:
  // instance variables
  uint8_t _i2cAddr; ///< Wire Address of the LCD
  uint8_t _backlight; ///< the backlight intensity
  uint8_t _lines; ///< number of lines of the display
  uint8_t _entrymode; ///<flags from entrymode
  uint8_t _displaycontrol; ///<flags from displaycontrol

  // variables on how the PCF8574 is connected to the LCD
  uint8_t _rs_mask;
  uint8_t _rw_mask;
  uint8_t _enable_mask;
  uint8_t _backlight_mask;
  // these are used for 4-bit data to the display.
  uint8_t _data_mask[4];

  // state of the RS line
  bool _rs_state;

  // low level functions
  void _send(uint8_t value, bool isData = false);
  void _sendNibble(uint8_t halfByte, bool isData = false);
  void _write2Wire(uint8_t byte);

  void init(uint8_t i2cAddr, uint8_t rs, uint8_t rw, uint8_t enable,
    uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t backlight=255);
};

#endif
