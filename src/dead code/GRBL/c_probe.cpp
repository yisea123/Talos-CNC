/*
* c_probe.cpp
*
* Created: 3/6/2019 10:46:10 AM
* Author: jeff_d
*/


/*
probe.c - code pertaining to probing methods
Part of Grbl

Copyright (c) 2014-2016 Sungeun K. Jeon for Gnea Research LLC

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

#include "c_probe.h"
#include <string.h>
#include "c_system.h"
#include "..\helpers.h"
#include "c_settings.h"
//#include "..\Common\Hardware_Abstraction_Layer\AVR_2560\c_grbl_avr_2560_probe.h"
#include "hardware_def.h"

//#include "grbl.h"
//#include "cpu_map.h"

// Inverts the probe pin state depending on user settings and probing cycle mode.
static uint8_t probe_invert_mask;

// Probe pin initialization routine.
void c_probe::probe_init()
{
	Hardware_Abstraction_Layer::Grbl::Probe::initialize();
	//PROBE_DDR &= ~(PROBE_MASK); // Configure as input pins
	//#ifdef DISABLE_PROBE_PIN_PULL_UP
	//PROBE_PORT &= ~(PROBE_MASK); // Normal low operation. Requires external pull-down.
	//#else
	//PROBE_PORT |= PROBE_MASK;    // Enable internal pull-up resistors. Normal high operation.
	//#endif
	//probe_configure_invert_mask(false); // Initialize invert mask.
}

// Called by probe_init() and the mc_probe() routines. Sets up the probe pin invert mask to
// appropriately set the pin logic according to setting for normal-high/normal-low operation
// and the probing cycle modes for toward-workpiece/away-from-workpiece.
//void c_probe::probe_configure_invert_mask(uint8_t is_probe_away)
//{
//probe_invert_mask = 0; // Initialize as zero.
//if (bit_isfalse(c_settings::settings.flags, BITFLAG_INVERT_PROBE_PIN))
//{
//probe_invert_mask ^= PROBE_MASK;
//}
//if (is_probe_away)
//{
//probe_invert_mask ^= PROBE_MASK;
//}
//}

// Returns the probe pin state. Triggered = true. Called by gcode parser and probe state monitor.
uint8_t c_probe::probe_get_state()
{

	return ((Hardware_Abstraction_Layer::Grbl::Probe::Pin_Values & Hardware_Abstraction_Layer::Grbl::Probe::Pin_Mask)
	 ^ Hardware_Abstraction_Layer::Grbl::Probe::Pin_Mask_Invert);
	//return ((PROBE_PIN & PROBE_MASK) ^ probe_invert_mask);
}

// Monitors probe pin state and records the system position when detected. Called by the
// stepper ISR per ISR tick.
// NOTE: This function must be extremely efficient as to not bog down the stepper ISR.
void c_probe::probe_state_monitor()
{
	if (probe_get_state())
	{
		c_system::sys_probe_state = PROBE_OFF;
		memcpy(c_system::sys_probe_position, c_system::sys_position, sizeof(c_system::sys_position));
		bit_true(c_system::sys_rt_exec_state, EXEC_MOTION_CANCEL);
	}
}
