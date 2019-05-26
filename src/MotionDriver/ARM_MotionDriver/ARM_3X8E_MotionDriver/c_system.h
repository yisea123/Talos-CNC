/*
* c_system.h
*
* Created: 5/25/2019 10:41:31 PM
* Author: Family
*/


#ifndef __C_SYSTEM_H__
#define __C_SYSTEM_H__
#include <stdint.h>
#define CONTROL_MODE_NONE 0
#define CONTROL_MODE_JOG 1

#define STEP_CONTROL_EXECUTE_HOLD         bit(1)
#define STEP_CONTROL_END_MOTION		bit(0)

namespace Motion_Core
{
	class System
	{
		//variables
		public:
		static uint32_t control_state_modes;
		static uint8_t StepControl;
		static uint32_t new_sequence;
		protected:
		private:

		//functions
		public:
		protected:
		private:

	}; //c_system
};
#endif //__C_SYSTEM_H__
