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
  
  Constructor takes three arguments which are the Arduino pins connected to
  pins 1 (DQ), 2 (CLK/CONV) and 3 (RST) on the DS1620.
  
*/
#ifndef DS1620_h
#define DS1620_h

#include <inttypes.h>

class DS1620
{
  public:
    DS1620(int DQ, int CLK, int RST);
    int read_temp();
    void write_th(int);
    void write_tl(int);
    int read_th();
    int read_tl();
    int read_counter();
    int read_slope();
    void start_conv();
    void stop_conv();
    int write_config(int);
    int read_config();
    
  private:
    int receive_data();
    void rst_start();
    void rst_stop();
    void send_command(int);
    int _DQ;
    int _CLK;
    int _RST;
    int _T_HIGH;
    int _T_LOW;
    int _T_COM;
    
};

#endif
