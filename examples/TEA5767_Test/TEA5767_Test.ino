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
#include <Wire.h>
#include <EEPROM.h>
#include "TEA5767.h"


/*******************************************************************************
 * Connections:
 *
 * I2C -> TEA5767
 *   Uno/Nano A4 (SDA), A5 (SCL)
 *   Mega2560 20 (SDA), 21 (SCL)
 *
 * 10k 10 Turns Potentiometer for tuning,
 * ie. Vishay 534-10K
 *   1   +5V
 *   2   A2
 *   3   GND
 ******************************************************************************/


TEA5767* tuner;
float freq;
int val,oldval;
int handmode;
int repcount = 0;

void setup() {
  // Wire needs to be in intialized before use.
  Wire.begin();
  Serial.begin(9600);
  while (!Serial) ;
  Serial.println("TEA5767 tuner test");

  EEPROM.get(0, freq);
  if ((freq < 88) || (freq > 108))
     freq = 88;

  tuner = new TEA5767;
  tuner->Deemphasis(false);
  tuner->HighCutControl(false);
  tuner->Frequency(freq);

  handmode = 0;
  oldval = val = map(analogRead(A2), 0, 1023, 10*88, 10*108); 
}


void loop() {
  // put your main code here, to run repeatedly:

  val = map(analogRead(A2), 0, 1023, 10*88, 10*108);
  if (abs(val - oldval) > 2) {
     // > 0.2MHz
     if (handmode == 0)
        Serial.println("handmode=on");
     handmode = 30;
     }
  else if (handmode > 0) {
     handmode--;
     if (handmode == 0) {
        Serial.println("handmode=off");
        EEPROM.put(0, freq);
        }
     }

  if (handmode && (freq * 10.0 != val)) {
     freq = val/10.0;
     oldval = val;
     tuner->Frequency(freq);
     Serial.print("freq = ");
     Serial.print(freq);
     Serial.print(", ADC level = ");
     Serial.print(tuner->GetAdcLevel());
     Serial.print(", stereo = ");
     Serial.println(tuner->GetStereoFlag());
     }

  repcount++;
  if (repcount == 10) {
     repcount = 0;
     Serial.print("ADC level = ");
     Serial.print(tuner->GetAdcLevel());
     Serial.print(", stereo = ");
     Serial.println(tuner->GetStereoFlag());
     }
  delay(1000);
}
