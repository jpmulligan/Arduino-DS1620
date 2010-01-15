/*
    Arduino DS1620 Library - Example Sketch
    Copyright (C) 2009 John P. Mulligan
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
    Documentation and latest version: http://wiki.thinkhole.org/ds1620

    Basic Pin Setup
    
    ARDUINO       DS1620
    -------       --------------
    +5 V          Pin 8 VDD
    GND           Pin 4 GND
    Pin 5         Pin 1 DQ
    Pin 4         Pin 2 CLK/CONV
    Pin 3         Pin 3 RST
    
     -------------
    | Arduino  13 |
    |          12 |
    |          11 |
    |          10 |
    |           9 |
    |           8 |
    |           7 |               ------------------
    |           6 |              |      DS1620      |
    |           5 |--------------| DQ           VDD |------ +5 V
    |           4 |--------------| CLK/CONV  T HIGH |
    |           3 |--------------| RST        T LOW |
    |           2 |     GROUND---| GND        T COM |
    |           1 |               ------------------
     -------------

    *  No external parts are required for basic temperature reading
    
*/

#include "DS1620.h"

// Define pins for 3-wire serial communication
int dq = 5;
int clk = 4;
int rst = 3;

// Call DS1620 constructor using pin variables
DS1620 d = DS1620(dq, clk, rst);

void setup()
{  
  // Start serial communications, 9600 baud
  Serial.begin(9600);

  // Write TH and TL setpoints to DS1620 EEPROM  
  // Settings are retained even with no power
  d.write_th(30);
  d.write_tl(15);
  
  // Write config to DS1620 configuration/status register
  // Decimal 10 = Binary 00001010
  // Enables CPU-Mode and disables 1-Shot Mode.
  // See Datasheet for details
  d.write_config(10);
  
  // Start continuous temperature conversion
  // Readings can be read about once per second this way
  d.start_conv();

  // Print TH and TL to serial monitor
  Serial.print("TH = "); Serial.println(d.read_th());
  Serial.print("TL = "); Serial.println(d.read_tl());
}

void loop()
{
  //  Read and print temperature (degrees C) to serial monitor
  Serial.println(d.read_temp());
  
  // Wait 1 second (1000 ms) before reading next temperature
  delay(1000);
}
