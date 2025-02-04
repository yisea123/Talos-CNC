/*
* c_spindle.cpp
*
* Created: 3/6/2019 12:26:09 PM
* Author: jeff_d
*/




/*
  spindle_control.c - spindle control methods
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

#include "c_spindle.h"
#include "c_system.h"
#include "c_settings.h"
#include "c_gcode_plus.h"
#include <math.h>
#include "c_protocol.h"
//#include "..\Common\Hardware_Abstraction_Layer\AVR_2560\c_grbl_avr_2560_spindle.h"
#include "hardware_def.h"

static float pwm_gradient; // Precalulated value to speed up rpm to PWM conversions.


void c_spindle::spindle_init()
{
	Hardware_Abstraction_Layer::Grbl::Spindle::initialize();

	//// Configure variable spindle PWM and enable pin, if required.
	//SPINDLE_PWM_DDR |= (1<<SPINDLE_PWM_BIT); // Configure as PWM output pin.
	//SPINDLE_TCCRA_REGISTER = SPINDLE_TCCRA_INIT_MASK; // Configure PWM output compare timer
	//SPINDLE_TCCRB_REGISTER = SPINDLE_TCCRB_INIT_MASK;
	//SPINDLE_OCRA_REGISTER = SPINDLE_OCRA_TOP_VALUE; // Set the top value for 16-bit fast PWM mode
	//SPINDLE_ENABLE_DDR |= (1<<SPINDLE_ENABLE_BIT); // Configure as output pin.
	//SPINDLE_DIRECTION_DDR |= (1<<SPINDLE_DIRECTION_BIT); // Configure as output pin.
  //
	//pwm_gradient = SPINDLE_PWM_RANGE/(c_settings::settings.rpm_max-c_settings::settings.rpm_min);
	//spindle_stop();
}


uint8_t c_spindle::spindle_get_state()
{
#ifdef INVERT_SPINDLE_ENABLE_PIN
	if (bit_isfalse(SPINDLE_ENABLE_PORT, (1 << SPINDLE_ENABLE_BIT)) && (SPINDLE_TCCRA_REGISTER & (1 << SPINDLE_COMB_BIT))) {
#else
	if (bit_istrue(SPINDLE_ENABLE_PORT, (1 << SPINDLE_ENABLE_BIT)) && (SPINDLE_TCCRA_REGISTER & (1 << SPINDLE_COMB_BIT))) {
#endif
		if (SPINDLE_DIRECTION_PORT & (1 << SPINDLE_DIRECTION_BIT)) { return(SPINDLE_STATE_CCW); }
		else { return(SPINDLE_STATE_CW); }
	}
	return(SPINDLE_STATE_DISABLE);
}


// Disables the spindle and sets PWM output to zero when PWM variable spindle speed is enabled.
// Called by various main program and ISR routines. Keep routine small, fast, and efficient.
// Called by spindle_init(), spindle_set_speed(), spindle_set_state(), and mc_reset().
void c_spindle::spindle_stop()
{
	Hardware_Abstraction_Layer::Grbl::Spindle::stop();
	//SPINDLE_TCCRA_REGISTER &= ~(1<<SPINDLE_COMB_BIT); // Disable PWM. Output voltage is zero.
	//#ifdef INVERT_SPINDLE_ENABLE_PIN
	//  SPINDLE_ENABLE_PORT |= (1<<SPINDLE_ENABLE_BIT);  // Set pin to high
	//#else
	//  SPINDLE_ENABLE_PORT &= ~(1<<SPINDLE_ENABLE_BIT); // Set pin to low
	//#endif
}


// Sets spindle speed PWM output and enable pin, if configured. Called by spindle_set_state()
// and stepper ISR. Keep routine small and efficient.
void c_spindle::spindle_set_speed(uint16_t pwm_value)
{
	Hardware_Abstraction_Layer::Grbl::Spindle::set_speed(pwm_value);
	//SPINDLE_OCR_REGISTER = pwm_value; // Set PWM output level.
	//if (pwm_value == SPINDLE_PWM_OFF_VALUE) {
	//  SPINDLE_TCCRA_REGISTER &= ~(1<<SPINDLE_COMB_BIT); // Disable PWM. Output voltage is zero.
	//} else {
	//  SPINDLE_TCCRA_REGISTER |= (1<<SPINDLE_COMB_BIT); // Ensure PWM output is enabled.
	//}
}


// Called by spindle_set_state() and step segment generator. Keep routine small and efficient.
uint16_t c_spindle::spindle_compute_pwm_value(float rpm) // Mega2560 PWM register is 16-bit.
{
	uint16_t pwm_value;
	rpm *= (0.010*c_system::sys.spindle_speed_ovr); // Scale by spindle speed override value.
	// Calculate PWM register value based on rpm max/min settings and programmed rpm.
	if ((c_settings::settings.rpm_min >= c_settings::settings.rpm_max) || (rpm >= c_settings::settings.rpm_max)) {
		// No PWM range possible. Set simple on/off spindle control pin state.
		c_system::sys.spindle_speed = c_settings::settings.rpm_max;
		pwm_value = SPINDLE_PWM_MAX_VALUE;
	}
	else if (rpm <= c_settings::settings.rpm_min) {
		if (rpm == 0.0) { // S0 disables spindle
			c_system::sys.spindle_speed = 0.0;
			pwm_value = SPINDLE_PWM_OFF_VALUE;
		}
		else { // Set minimum PWM output
			c_system::sys.spindle_speed = c_settings::settings.rpm_min;
			pwm_value = SPINDLE_PWM_MIN_VALUE;
		}
	}
	else {
		// Compute intermediate PWM value with linear spindle speed model.
		// NOTE: A nonlinear model could be installed here, if required, but keep it VERY light-weight.
		c_system::sys.spindle_speed = rpm;
		pwm_value = floor((rpm - c_settings::settings.rpm_min)*pwm_gradient) + SPINDLE_PWM_MIN_VALUE;
	}
	return(pwm_value);
}


// Immediately sets spindle running state with direction and spindle rpm via PWM, if enabled.
// Called by g-code parser spindle_sync(), parking retract and restore, g-code program end,
// sleep, and spindle stop override.
void c_spindle::spindle_set_state(uint8_t state, float rpm)
{
	if (c_system::sys.abort) { return; } // Block during abort.
	if (state == SPINDLE_DISABLE) { // Halt or set spindle direction and rpm.

		c_system::sys.spindle_speed = 0.0;
		spindle_stop();

	}
	else {

		/*if (state == SPINDLE_ENABLE_CW) {
		  SPINDLE_DIRECTION_PORT &= ~(1<<SPINDLE_DIRECTION_BIT);
		} else {
		  SPINDLE_DIRECTION_PORT |= (1<<SPINDLE_DIRECTION_BIT);
		}*/
		Hardware_Abstraction_Layer::Grbl::Spindle::set_direction(state);

		// NOTE: Assumes all calls to this function is when Grbl is not moving or must remain off.
		if (c_settings::settings.flags & BITFLAG_LASER_MODE) {
			if (state == SPINDLE_ENABLE_CCW) { rpm = 0.0; } // TODO: May need to be rpm_min*(100/MAX_SPINDLE_SPEED_OVERRIDE);
		}
		spindle_set_speed(spindle_compute_pwm_value(rpm));

		//#ifdef INVERT_SPINDLE_ENABLE_PIN
		//	SPINDLE_ENABLE_PORT &= ~(1<<SPINDLE_ENABLE_BIT);
		//#else
		//	SPINDLE_ENABLE_PORT |= (1<<SPINDLE_ENABLE_BIT);
		//#endif   
		Hardware_Abstraction_Layer::Grbl::Spindle::enable();
	}

	c_system::sys.report_ovr_counter = 0; // Set to report change immediately
}


// G-code parser entry-point for setting spindle state. Forces a planner buffer sync and bails 
// if an abort or check-mode is active.
void c_spindle::spindle_sync(uint8_t state, float rpm)
{
	if (c_system::sys.state == STATE_CHECK_MODE) { return; }
	c_protocol::protocol_buffer_synchronize(); // Empty planner buffer to ensure spindle is set when programmed.
	spindle_set_state(state, rpm);
}
