/*
  Arduino DS1620 Library 0.1
  Copyright (C) 2009 John P. Mulligan. All rights reserved.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.
    
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  Project URL:  http://wiki.thinkhole.org/ds1620

  Datasheet URL: http://www.maxim-ic.com/quick_view2.cfm/qv_pk/2735

  DS1620 8-Pin DIP Pin Assignment:
  
    1 - DQ         8 - VDD (2.7V - 5.5V)
    2 - CLK/CONV   7 - T HIGH
    3 - RST        6 - T LOW
    4 - GND        5 - T COM

  Serial Communications (3-Wire):

    (1) Set RST high
    (2) Send command, least significant bit first
    (3) Read or write 8-bit config or 9-bit temperature
    (5) Set RST low

*/

#include "WProgram.h"
#include "DS1620.h"

// DS1620 Commands

#define READ_TEMP  0xAA         // Read temperature register
#define WRITE_TH   0x01         // Write to the TH (High Temp) register
#define WRITE_TL   0x02         // Write to the TL (Low Temp) register
#define READ_TH    0xA1         // Read the TH (High Temp) register
#define READ_TL    0xA2         // Read the TL (Low Temp) register
#define READ_CNTR  0xA0         // Read the value of the counter byte
#define READ_SLOPE 0xA9         // Read the slope counter byte
#define START_CNV  0xEE         // Begin temperature conversion
#define STOP_CNV   0x22         // Stop temperature conversion
#define WRITE_CFG  0x0C         // Write configuration register
#define READ_CFG   0xAC         // Read the value in the config register

#define WRITE_DELAY 20          // Time to wait (ms) after a EEPROM write

DS1620::DS1620(int DQ, int CLK, int RST)
{
    pinMode(DQ, OUTPUT);
    pinMode(RST, OUTPUT);
    pinMode(CLK, OUTPUT);
    _DQ = DQ;
    _CLK = CLK;
    _RST = RST;
}
    
int DS1620::read_temp()
{
    short t, tempIn, sign;

    rst_start();
    send_command(READ_TEMP); // Next 9 clock cycles, last temp conv result
    tempIn = (short)receive_data();
    rst_stop();

    sign = (tempIn & 0x0100);
    sign = (sign >> 8);      // mask sign bit from temp
    tempIn = (tempIn & 0x00FF);

    if (sign == 1)
    {
        // temp is negative
        // According to specification: [256 (0x0100) - Temp]
        tempIn = 0x0100 - tempIn;
        tempIn -= (tempIn * 2);
    }

    t = tempIn/2;

    return(t);
} 


void DS1620::write_th(int high_temp)
{
    int bit;
    
    high_temp = high_temp * 2;
    rst_start();  
    send_command(WRITE_TH); // Next 9 clock cycles, value of the high temp limit
    for(int n=0; n<9; n++){ // Send all nine bits of temperature
        bit = (high_temp >> n) & 0x01;
        digitalWrite(_DQ, bit); // DQ HIGH or LOW based on bit
        digitalWrite(_CLK, LOW);  // CLK LOW then HIGH to make one cycle
        digitalWrite(_CLK, HIGH);  
    }
    delay(WRITE_DELAY); // Write can take up to 10ms
    rst_stop();
}

void DS1620::write_tl(int temp)
{
    int bit;
    
    temp = temp * 2;
    rst_start();  
    send_command(WRITE_TL); // Next 9 clock cycles, value of the high temp limit
    for(int n=0; n<9; n++){ // Send all nine bits of temperature
        bit = (temp >> n) & 0x01;
        digitalWrite(_DQ, bit); // DQ HIGH or LOW based on bit
        digitalWrite(_CLK, LOW);  // CLK LOW then HIGH to make one cycle
        digitalWrite(_CLK, HIGH);  
    }
    delay(WRITE_DELAY); // Write can take up to 10ms
    rst_stop();
}

int DS1620::read_th()
{
  int temp = 0;
  
  rst_start();
  send_command(READ_TH); // Next 8 clock cycles output value of config register
  temp = receive_data()/2;
  rst_stop();
  return(temp);
}

int DS1620::read_tl()
{
  int temp = 0;
  
  rst_start();
  send_command(READ_TL); // Next 8 clock cycles output value of config register
  temp = receive_data()/2;
  rst_stop();
  return(temp);
}

int DS1620::read_counter()
{
  int counter = 0;
  
  rst_start();
  send_command(READ_CNTR); // Next 9 clock cycles output value of counter
  counter = receive_data();
  rst_stop();
  return(counter);
}

int DS1620::read_slope()
{
  int slope = 0;
  
  rst_start();
  send_command(READ_SLOPE); // Next 9 clock cycles output value of counter
  slope = receive_data();
  rst_stop();
  return(slope);
}

void DS1620::start_conv()
{
  rst_start();
  send_command(START_CNV); // Begins temp conv, depends on 1-shot mode   
  rst_stop();
}

void DS1620::stop_conv()
{
  rst_start();
  send_command(STOP_CNV); // Stops temperature conversion 
  rst_stop();
}

int DS1620::write_config(int config_register)
{
  /*
      Return codes:  0 = Write successful
                     1 = Write verification failed
                     2 = Bad config register
  */
  if(config_register > 0) {
    rst_start();
    send_command(WRITE_CFG); // Next 8 clock cycles, value of config register;
    send_command(config_register);
    delay(WRITE_DELAY); // Write can take up to 10ms
    rst_stop();
    // Confirm that config was properly written
    if(read_config() == config_register) { return 0; }
    else { return 1; }
  }
  return 2;
}

int DS1620::read_config()
{
  int config_register = 0;
  
  rst_start();
  send_command(READ_CFG); // Next 8 clock cycles output value of config register
  config_register = receive_data();
  rst_stop();
  return(config_register);
}

int DS1620::receive_data()
{
  int data = 0;
  int n;
  int bit;
  
  pinMode(_DQ, INPUT); // Change Data/DQ pin mode to accept INPUT
  for(n=0; n<9; n++) { // Always receive 9 bits of data
    digitalWrite(_CLK, LOW);
    bit = digitalRead(_DQ);
    digitalWrite(_CLK, HIGH);
    data = data | bit << n;
  }  
  pinMode(_DQ, OUTPUT); // Done reading, set back to OUTPUT
  
  return(data);
}

void DS1620::rst_start()
{
  digitalWrite(_RST, LOW);
  digitalWrite(_CLK, HIGH);
  digitalWrite(_RST, HIGH); // All communications start by taking RST HIGH
}

void DS1620::rst_stop()
{
  digitalWrite(_RST, LOW); // Taking RST LOW will terminate any communication
}

void DS1620::send_command(int command)
{
 int n;
 int bit;
 
 for(n=0; n<8; n++){ // Always send 8 bits of data
   // Arithmetic bitwise shift command to the right, then AND with
   // bitmask (00000001) to return next bit, least significant (rightmost) first
   bit = (command >> n) & 0x01;
   digitalWrite(_DQ, bit); // DQ HIGH or LOW based on bit
   digitalWrite(_CLK, LOW);  // CLK LOW then HIGH to make one cycle
   digitalWrite(_CLK, HIGH);  
 }
}
