#include "../system.h"
#include "../main.h"
#include "pid.h"

int16_t pid_get_output(struct PID* pid, int16_t error);
void pid_pwm_control(struct PID* pid);


void pid_pwm_set_pin(struct PID *pid, int8_t pin)
{
	if (pin == 1)
	{
		PinTo1(*pid->pwm.io.port, pid->pwm.io.pin);
	}
	else
	{
		PinTo0(*pid->pwm.io.port, pid->pwm.io.pin);
	}

}
void pid_pwm_stablish_levelpin(struct PID *pid)
{
	pid->pwm.dc.ktop_uploaded_ms = pid->pwm.dc.ktop_ms;
	if (pid->pwm.dc.ktop_uploaded_ms > 0)
	{
		PinTo1(*pid->pwm.io.port, pid->pwm.io.pin);
	}
	else
	{
		PinTo0(*pid->pwm.io.port, pid->pwm.io.pin);
	}
}

void pid_find_ktop_ms(struct PID *pid, int16_t error)
{
	/* 2. get and convert to time the output of pid_algorithm (adimensional) */
	/* Obtain ktop_ms*/
	pid->pwm.dc.ktop_ms = pid_get_output(pid, error) * pid->algo.scaler_time_ms;
	//
	if (pid->pwm.dc.ktop_ms > pid->algo.pid_out_max_ms)
		{pid->pwm.dc.ktop_ms = pid->algo.pid_out_max_ms;}
	else if (pid->pwm.dc.ktop_ms < pid->algo.pid_out_min_ms)
		{pid->pwm.dc.ktop_ms = pid->algo.pid_out_min_ms;}
}
void pid_job(struct PID *pid, int16_t error)
{
	pid_find_ktop_ms(pid, error);
	//
	/* 3. */
	if (mainflag.sysTickMs)
	{
		/* se accede PWM_ACCESS_TIME_MS/SYSTICK_MS */
		if (++pid->pwm.timing.counter_ticks_ms >= pid->pwm.timing.kmax_ticks_ms)
		{
			pid->pwm.timing.counter_ticks_ms = 0x00;

			/* aqui tengo cada 100 ms*/
			pid_pwm_control(pid);
		}
	}
}
/**********************************************
pid_algorithm es adimensional
Note: previamente kei_windup_max/min has to be setted

Buscar a traves de Kp, que el la salida del PID
este entre el rango de "10...0", para cualquier proceso
en donde >=10 es 100%, y 0=0%
**********************************************/
int16_t pid_get_output(struct PID* pid, int16_t error)
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
				if (pid->pwm.dc.counter_ms >= pid->pwm.timing.k_systemdelay_ton_ms )
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
	if (pid->pwm.dc.counter_ms >= (pid->pwm.timing.k_systemdelay_ton_ms + pid->algo.pid_out_max_ms + pid->pwm.timing.k_systemdelay_toff_ms) )
	{
		pid->pwm.dc.counter_ms = 0;
		//
		pid->pwm.timing.sm0 = 0;
		//
		pid_pwm_stablish_levelpin(pid);
//		pid->pwm.dc.ktop_uploaded_ms = pid->pwm.dc.ktop_ms;	/* update, transfer*/
//		if (pid->pwm.dc.ktop_uploaded_ms > 0)
//		{
//			PinTo1(*pid->pwm.io.port, pid->pwm.io.pin);
//		}
		//
	}
}

