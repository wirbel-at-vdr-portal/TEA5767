/*******************************************************************************
 * TEA5767HN single-chip I2C FM stereo radio arduino library
 * Copyright (C) 2022  Winfried Koehler
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 * The latest version of this library can always be found at it's github page,
 * https://github.com/wirbel-at-vdr-portal/TEA5767
 ******************************************************************************/
#include <Arduino.h>
#include <Wire.h>
#include <TEA5767.h>

TEA5767::TEA5767() :
  PLLREF(0), XTAL(1), // 32.768kHz crystal
  MUTE(0),            // not muted
  SM(0),              // no search mode
  SUD(1),             // search direction: up
  SSL(2),             // search stop level: mid
  HLSI(1),            // high side LO injection
  MS(0),              // stereo
  MR(0),ML(0),        // mute right, mute left: off
  SWP1(0),SWP2(0),    // software port1, port2: off
  STBY(0),            // standby off
  BL(0),              // band limit US/Europe: 88..108MHz
  SMUTE(1),           // soft mute on
  HCC(0),             // high freq cutoff: off
  SNC(1),             // stereo noise cancelling on
  SI(1),              // port1 as tuning indicator
  DTC(0),             // deemphasis 50µs
  PLL(10770),         // 88MHz
  lastRead(0),        // not yet read
  b1(0),b2(0),b3(0),b4(0),b5(0),
  r1(0),r2(0),r3(0),r4(0),r5(0)
{
  Set(true);
}

void TEA5767::SetPllClock(int choice) {
  /* 0:13MHz ext clk
   * 1:32.768kHz ext clk
   * 2:6.5MHz
   */
  if (choice >= 0 && choice <= 2) {
     PLLREF = (choice & 2) >> 1;
     XTAL   = (choice & 1);
     Set();
     }
}

int TEA5767::PllClock(void) {
  switch((PLLREF << 1) | XTAL) {
     case 0:  return 13000000;
     case 1:  return 32768;
     case 2:  return 6500000;
     default: return 32768;
     }
}

void TEA5767::Mute(bool on) {
  MUTE = on ? 1 : 0;
  Set();
}

void TEA5767::SearchDirection(bool up) {
  SUD = up ? 1 : 0;
  Set();
}

void TEA5767::SearchStopLevel(int level) {
  if (level >= 1 && level <= 3)
     SSL = level;
  Set();
}

void TEA5767::HighSideInjection(bool on) {
  HLSI = on ? 1 : 0;
  Set();
}

void TEA5767::Mono(bool on) {
  MS = on ? 1 : 0;
  Set();
}

void TEA5767::MuteRight(bool on) {
  MR = on ? 1 : 0;
  Set();
}

void TEA5767::MuteLeft(bool on) {
  ML = on ? 1 : 0;
  Set();
}

void TEA5767::Port1(bool on) {
  SWP1 = on ? 1 : 0;
  Set();
}

void TEA5767::Port2(bool on) {
  SWP2 = on ? 1 : 0;
  Set();
}

void TEA5767::Standby(bool on) {
  STBY = on ? 1 : 0;
  Set();
}

void TEA5767::BandLimits(bool Japan) {
  BL = Japan ? 1 : 0;
  Set();
}

void TEA5767::SoftMute(bool on) {
  SMUTE = on ? 1 : 0;
  Set();
}

void TEA5767::HighCutControl(bool on) {
  HCC = on ? 1 : 0;
  Set();
}

void TEA5767::StereoNoiseCancelling(bool on) {
  SNC = on ? 1 : 0;
  Set();
}

void TEA5767::SearchIndicator(bool on) {
  SI = on ? 1 : 0;
  Set();
}

void TEA5767::Deemphasis(bool US) {
  DTC = US ? 1 : 0;
  Set();
}

void TEA5767::Frequency(float MHz) {
  if (HLSI)
     PLL = 0.5 + ((4000000.0*MHz + 900000.0) / ((float) PllClock()));
  else
     PLL = 0.5 + ((4000000.0*MHz - 900000.0) / ((float) PllClock()));
  Set();
}

bool TEA5767::GetReadyFlag(void) {
  Get();
  return (r1 & 0x80) > 0;
}

bool TEA5767::GetBandlimitFlag(void) {
  Get();
  return (r1 & 0x40) > 0;
}

float TEA5767::GetFrequency(void) {
  Get();
  uint16_t pll = (r1 & 0x3F) << 8 | r2;
  if (HLSI)
     return ((pll * PllClock()) - 900000) / 4000000;
  else
     return ((pll * PllClock()) + 900000) / 4000000;
}

bool TEA5767::GetStereoFlag(void) {
  Get();
  return (r3 & 0x80) > 0;
}

int TEA5767::GetIfCounter(void) {
  /* Because the IF counter is not automatically updated, each time
   * the IF counter value is read, a previous write action must be
   * performed. The following actions are needed:
   *   1. write to I2C
   *   2. wait >= 27ms for result
   *   3. read the result
   */
  Set(true);    // write to I2C
  delay(28);    // >= 27msec
  lastRead = 0; // enshure we are able to read
  Get();        // get result && update lastRead
  return r3 & 0x7F;
}

float TEA5767::GetIfFrequency(void) {
  float resolution = XTAL == 1 ? 4062.5 : 4096.0;
  return GetIfCounter() * resolution;
}

int TEA5767::GetAdcLevel(void) {
  Get();
  return r4 >> 4;
}

int TEA5767::Get_dBuV(void) {
  float dBuV = 8.0 + 52.0 * (GetAdcLevel() / 15.0);
  return 0.5 + dBuV;
}

void TEA5767::Set(bool force) {
  uint8_t B1,B2,B3,B4,B5;

  /*
   * 7 MUTE          1=muted,0=not muted
   * 6 SM            1=search mode, 0=no search mode
   * 5..0 PLL[13:8]  setting of synthesizer programmable counter for search or preset
   */
  B1 = (MUTE << 7) | (SM << 6) | (PLL >> 8);

  /*
   * 7..0 PLL[7:0]
   */
  B2 = (PLL & 0xFF);

  /*
   * 7    SUD        Search Up/Down: SUD = 1 up; SUD = 0 down
   * 6..5 SSL[1:0]   Search Stop Level:  0:forbidden, 1:5, 2:7, 3:10
   * 4    HLSI       High/Low Side Injection, 1=High Side LO Injection, 0=Low Side LO injection
   * 3    MS         Mono = 1, Stereo=0
   * 2    MR         Mute Right; MR=1 right audio channel muted and forced mono; if MR = 0 then the right audio channel is not muted
   * 1    ML         Mute Left:  ML=1 left  audio channel muted and forced mono; if ML = 0 then the left audio channel is not muted
   * 0    SWP1       1:Port1 high, 0:port1 low
   */
  B3 = (SUD << 7) | (SSL << 5) | (HLSI << 4) | (MS << 3) | (MR << 2) | (ML << 1) | SWP1;

  /*
   * 7    SWP2       wie SWP1 für port2
   * 6    STBY       1=standby, 0=on
   * 5    BL         Band Limit: 1=Japan, 0=US/Europe
   * 4    XTAL       zusammen mit PLLREF: clock 13Mhz, 32.768kHz, 6.5Mhz
   * 3    SMUTE      Soft Mute, 1=on 0=off                The softmute function suppresses the interstation noise and prevents excessive noise from being heard when the signal level drops to a low level.
   * 2    HCC        High Cut Control: 1 ON; 0 OFF        Cut high frequencies from the audio signal when a weak signal is received.
   * 1    SNC        Stereo Noise Cancelling: 1 ON; 0 OFF The stereo noise cancelling (SNC) function gradually turns the stereo decoder from ‘full stereo’ to mono under weak signal conditions.
   * 0    SI         Search Indicator: 1=pin SWPORT1 is output for the ready flag; if SI = 0 then pin SWPORT1 is software programmable port 1
   */
  B4 = (SWP2 << 7) | (STBY << 6) | (BL << 5) | (XTAL << 4) | (SMUTE << 3) | (HCC << 2) | (SNC << 1) | SI;

  /*
   * 7    PLLREF     1: 6.5Mhz 6.5 MHz reference frequency for the PLL is enabled; if PLLREF = 0 then the 6.5 MHz reference frequency for the PLL is disabled
   * 6    DTC        de-emphasis time constant; 1:75µs 0:50µs
   * 5:0  not used
   */
  B5 = (PLLREF << 7) | (DTC << 6);

  if (force || (B1 != b1) || (B2 != b2) || (B3 != b3) || (B4 != b4) || (B5 != b5)) {
     Wire.beginTransmission(Address); 
     Wire.write(B1);
     Wire.write(B2);
     Wire.write(B3);
     Wire.write(B4);
     Wire.write(B5);
     Wire.endTransmission();

     b1 = B1;
     b2 = B2;
     b3 = B3;
     b4 = B4;
     b5 = B5;
     }
}

void TEA5767::Get(void) {
  if (millis() < (lastRead + 500))
     return;

  lastRead = millis();
  Wire.beginTransmission(GyroAddress); 
  Wire.requestFrom(Address, 5);
  if (Wire.available() >= 5) {
     r4 = 0;

     r1 = Wire.read();
     r2 = Wire.read();
     r3 = Wire.read();
     r4 = Wire.read();
     r5 = Wire.read();
     }
  Wire.endTransmission();
}