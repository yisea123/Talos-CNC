/* 
* c_spindle.h
*
* Created: 3/6/2019 12:26:09 PM
* Author: jeff_d
*/

/*
  spindle_control.h - spindle control methods
  Part of Grbl

  Copyright (c) 2012-2016 Sungeun K. Jeon for Gnea Research LLC
  Copyright (c) 2009-2011 Simen Svale Skogsrud

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __C_SPINDLE_H__
#define __C_SPINDLE_H__

#include <stdint.h>
#include "..\helpers.h"

#define SPINDLE_NO_SYNC false
#define SPINDLE_FORCE_SYNC true

#define SPINDLE_STATE_DISABLE  0  // Must be zero.
#define SPINDLE_STATE_CW       bit(0)
#define SPINDLE_STATE_CCW      bit(1)


class c_spindle
{
//variables
public:
protected:
private:

//functions
public:
static void spindle_init();

// Returns current spindle output state. Overrides may alter it from programmed states.
static uint8_t spindle_get_state();

// Called by g-code parser when setting spindle state and requires a buffer sync.
// Immediately sets spindle running state with direction and spindle rpm via PWM, if enabled.
// Called by spindle_sync() after sync and parking motion/spindle stop override during restore.

// Called by g-code parser when setting spindle state and requires a buffer sync.
static void spindle_sync(uint8_t state, float rpm);

// Sets spindle running state with direction, enable, and spindle PWM.
static void spindle_set_state(uint8_t state, float rpm);

// Sets spindle PWM quickly for stepper ISR. Also called by spindle_set_state().
// NOTE: Mega2560 PWM register is 16-bit.
static void spindle_set_speed(uint16_t pwm_value);

// Computes Mega2560-specific PWM register value for the given RPM for quick updating.
static uint16_t spindle_compute_pwm_value(float rpm);

// Stop and start spindle routines. Called by all spindle routines and stepper ISR.
static void spindle_stop();
protected:
private:

}; //c_spindle

#endif //__C_SPINDLE_H__
