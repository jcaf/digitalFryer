/*
10UT*PID_FS_UT
10
pid->pwm.timing.kmax_ticks_ms = 100/SYSTICKS;//100 ms
 fs= 1000/100; -> counterx*10

pid->pwm.timing.kmax_ticks_ms = 10/SYSTICKS;//100 ms
 fs= 100/10; -> counterx*10

-------------------------*/

#include "../system.h"
#include "../main.h"
#include "pid.h"

int16_t pid_algorithm(struct PID* pid, int16_t error);
void pid_pwm_control(struct PID* pid);

struct PID pid;

/* cada PWM_ACCESS_TIME_MS se lleva el timing del periodo del PWM */
#define PWM_ACCESS_TIME_MS 100

/******************************************
 *
 ******************************************/
void pid_set(void)
{
	pid.pwm.timing.kmax_ticks_ms = PWM_ACCESS_TIME_MS/SYSTICK_MS;//100 ms

	/* tengo que convertir a unidades de tiempo */
	pid.algo.scaler_time_ms = 1*(1000/PWM_ACCESS_TIME_MS);

	/* aqui es donde realmente se fija el valor */
	/* todo depende practiamente del valor asignado asignado a KP*/
	pid.algo.pid_out_max_ms = 10 * pid.algo.scaler_time_ms;
	pid.algo.pid_out_min_ms = 0 * pid.algo.scaler_time_ms;

	pid.pwm.timing.kton_ms = 6000 / PWM_ACCESS_TIME_MS;
	pid.pwm.timing.ktoff_ms = 2000/ PWM_ACCESS_TIME_MS;

	pid.pwm.io.port = &PORTWxCHISPERO_ONOFF;
	pid.pwm.io.pin = PINxCHISPERO_ONOFF;
}

/*****************************************
 *
 *****************************************/
void pid_job(void)
{
	/* 1. error es target-specific */
	int8_t pv = 25;

	int8_t error =  pid.algo.sp - pv;

	/* adjust windup for integral error */
	if (error > 10) 
	{
		pid.algo.kei_windup_max =10;
	}
	else 
	{
	}
	
	/* 2. get and convert to time the output of pid_algorithm (adimensional) */
	pid.pwm.dc.ktop_ms = pid_algorithm(&pid, error) * pid.algo.scaler_time_ms;
	//
	if (pid.pwm.dc.ktop_ms > pid.algo.pid_out_max_ms)
		pid.pwm.dc.ktop_ms = pid.algo.pid_out_max_ms;
	else if (pid.pwm.dc.ktop_ms < pid.algo.pid_out_min_ms)
		pid.pwm.dc.ktop_ms = pid.algo.pid_out_min_ms;
	//
	/* 3. */

	if (mainflag.sysTickMs)
	{
		if (++pid.pwm.timing.counter_ticks_ms >= pid.pwm.timing.kmax_ticks_ms)
		{
			pid.pwm.timing.counter_ticks_ms = 0x00;

			pid_pwm_control(&pid);
		}
	}
}
/**********************************************
pid_algorithm es adimensional

Note: previamente kei_windup_max/min has to be setted
**********************************************/
int16_t pid_algorithm(struct PID* pid, int16_t error)
{
	pid->algo.ei += error;
	//
	if (pid->algo.ei > pid->algo.kei_windup_max)
		pid->algo.ei = pid->algo.kei_windup_max;
	else if (pid->algo.ei < pid->algo.kei_windup_min)
		pid->algo.ei = pid->algo.kei_windup_min;
	//
	pid->algo.ed = error - pid->algo.eprevio;
	pid->algo.eprevio = error;
	//
	int16_t pid_out = (pid->algo.kp * error) + (pid->algo.ki * pid->algo.ei) + (pid->algo.kd * pid->algo.ed);
	//return (pid_out * pid->algo.scaler_time_ms);
	return pid_out;
}
/******************************************
 *
 ******************************************/
void pid_pwm_control(struct PID* pid)
{
	pid->pwm.dc.counter_ms++;

	/* inhibit with -1 */
	if (pid->pwm.timing.sm0 >= 0)
	{
		if (pid->pwm.dc.ktop_uploaded_ms > 0)
		{
			if (pid->pwm.timing.sm0 == 0)
			{
				if (pid->pwm.dc.counter_ms >= pid->pwm.timing.kton_ms )
				{
					pid->pwm.dc.counter_ms = 0;
					pid->pwm.timing.sm0++;
				}
			}
			else if (pid->pwm.timing.sm0 == 1)
			{
				if (pid->pwm.dc.counter_ms >= pid->pwm.dc.ktop_uploaded_ms)
				{
					if (pid->pwm.dc.ktop_uploaded_ms < pid->algo.pid_out_max_ms)
					{
						PinTo0(*pid->pwm.io.port, pid->pwm.io.pin);
					}
				}
			}
		}
	}
	//
	/* period */
	if (pid->pwm.dc.counter_ms >= (pid->pwm.timing.kton_ms + pid->algo.pid_out_max_ms + pid->pwm.timing.ktoff_ms) )
	{
		pid->pwm.dc.counter_ms = 0;
		//
		pid->pwm.timing.sm0 = 0;
		pid->pwm.dc.ktop_uploaded_ms = pid->pwm.dc.ktop_ms;	/* update, transfer*/
		//
		if (pid->pwm.dc.ktop_uploaded_ms > 0)
		{
			PinTo1(*pid->pwm.io.port, pid->pwm.io.pin);
		}
	}
}

