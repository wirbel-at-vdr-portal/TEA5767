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
#pragma once
#include <stdint.h> // uint{8,16,32}_t

class TEA5767 {
private:
  static constexpr int Address = 0x60;
  int PLLREF,XTAL;
  int MUTE;
  int SM;
  int SUD;
  int SSL;
  int HLSI;
  int MS,MR,ML;
  int SWP1,SWP2;
  int STBY;
  int BL;
  int SMUTE;
  int HCC;
  int SNC;
  int SI;
  int DTC;
  uint16_t PLL;
  unsigned long lastRead;
  uint8_t b1,b2,b3,b4,b5;
  uint8_t r1,r2,r3,r4,r5;

  void Set(bool force = false);
  void Get(void);
public:
  TEA5767();
  void SetPllClock(int choice);                    // 0:13MHz ext clk, 1:32.768kHz ext clk, 2:6.5MHz
  uint32_t PllClock(void);                         // Current PLL clock in Hz
  void Mute(bool on);                              //
  void SearchDirection(bool up);                   //
  void SearchStopLevel(int level);                 // 1:low, 2:mid, 3:high
  void Search(void);                               // search next station.
  void HighSideInjection(bool on);                 // true: high side injection, false: low side injection
  void Mono(bool on);                              //
  void MuteRight(bool on);                         //
  void MuteLeft(bool on);                          //
  void Port1(bool on);                             //
  void Port2(bool on);                             //
  void Standby(bool on);                           // low power state
  void SoftMute(bool on);                          //
  void BandLimits(bool Japan);                     // Japan: Japanese FM band (76MHz..91MHz), US/Europe FM band (88MHz..108MHz) otherwise.
  void HighCutControl(bool on);                    // damp higher audio freq on low signal
  void StereoNoiseCancelling(bool on);             // decrease stereo effect on low signal
  void SearchIndicator(bool on);                   // port1 as search indicator output.
  void Deemphasis(bool US);                        // US = false: Europe, 50µs, US = true: 75µs
  void Frequency(float MHz);                       // set tuner freq in MHz (preset, instead of search)
  uint16_t PllDivider(void);                       // current PLL value set by Frequency(), readonly.
  /* hw readback functions. */
  bool GetReadyFlag(void);                         // true: a station has been found or band limit reached, false no station has been found
  bool GetBandlimitFlag(void);                     // true if band limit has been reached
  float GetFrequency(void);                        // frequency after search or preset
  bool GetStereoFlag(void);                        // true: stereo reception
  int  GetIfCounter(void);                         // IF counter result (>30msecs!)
  float GetIfFrequency(void);                      // estimate IF frequency from IF counter result (>30msecs!)
  int  GetAdcLevel(void);                          // level ADC readout
  int  Get_dBuV(void);                             // estimate dBuV from ADC level;
};
