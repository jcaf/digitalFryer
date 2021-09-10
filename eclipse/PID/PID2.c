#include "../system.h"
#include "../main.h"

#include "PID.h"
#define PID_OUTPUT_MIN 0
#define PID_OUTPUT_MAX 10

struct TimeBasePID
{
	int8_t sm0;
	uint16_t counter0;
	int8_t factor_escala;
	//
	/* estas variables debo de separlos*/
	int8_t toff_adicional_seg;
	int8_t idx;
	int8_t kdiv_1s[TB_NUM_OPTIONS];
	uint16_t kmax_ms[TB_NUM_OPTIONS];
};

struct DutyCycle
{
	uint8_t ktop;
	uint8_t ktop_buff;
	uint8_t counter;
} dc;

struct Ctrl
{
	int16_t sp;			/* set point */
	float kp;
	float ki;
	float kd;
	//
	int16_t ei; 		/* error integral */
	int16_t eprevio;	/* error previo */
	int16_t ed;			/* error derivativo	*/
	//
	int8_t kei_windup_max;	/* max windup for error integral */
	int8_t kei_windup_min;	/* min windup for error integral */
};

struct PID
{
	struct Ctrl ctrl;
	struct PWM
	{
		struct DutyCycle dc;
		struct TimeBasePID tb;
	}pwm;

}pid;

/*
 *
 */
pid_control
set_pid
{

}

int16_t pid_control(int16_t pv)
{
	int16_t error;
	int16_t pid_out;

	error = pid.ctrl.sp - pv;
	//
	pid.ctrl.ei += error;
	//kei_windup =  10;
	//
	if (pid.ctrl.ei > pid.ctrl.kei_windup_max)
		pid.ctrl.ei = pid.ctrl.kei_windup_max;
	else if (pid.ctrl.ei < pid.ctrl.kei_windup_min)
		pid.ctrl.ei = pid.ctrl.kei_windup_min;
	//
	pid.ctrl.ed = error - pid.ctrl.eprevio;
	pid.ctrl.eprevio = error;
	//
	pid_out = (pid.ctrl.kp * error) + (pid.ctrl.ki * pid.ctrl.ei) + (pid.ctrl.kd * pid.ctrl.ed);
	//
	if (pid_out > PID_OUTPUT_MAX)
		pid_out = PID_OUTPUT_MAX;
	else if (pid_out < PID_OUTPUT_MIN)
		pid_out = PID_OUTPUT_MIN;
	//
	return pid_out * pid.pwm.tb.factor_escala;

	/*
	 * Puede ser reescalado (0..10) * F
	 * en realidad PID_output va a representar TIEMPO expresado en segundos
	 * factor_escala_pid puede ser una float con 1 solo decimal
	 */
}

/*
 * Esta función va a ser temporizada desde main() y no desde ISR()
 */
void PWMSoft_control(void)
{
	if (mainflag.sysTickMs)
	{
		if (++pid.pwm.tb.counter0 >= pid.pwm.tb.kmax_ms[pid.pwm.tb.idx])
		{
			pid.pwm.tb.counter0 = 0x00;

			/* Begin time base control */
			
			pid.pwm.dc.counter++;

			if (pid.pwm.tb.sm0 >= 0)//inhibit with -1
			{
				/* adicional = 0*/
				pid.pwm.tb.toff_adicional_seg = 0;

				if (pid.pwm.dc.ktop > 0)
				{
					if (pid.pwm.tb.sm0 == 0)
					{
						if (pid.pwm.dc.counter >= (KFRYER_ON_SEG * pid.pwm.tb.kdiv_1s[pid.pwm.tb.idx]) ) //afectado por DKIV_1S
						{
							pid.pwm.dc.counter = 0;
							pid.pwm.tb.sm0++;
						}
					}
					else if (pid.pwm.tb.sm0 == 1)
					{
						/* PWMSoft.dc.TOP es afectado por el factor de escala en la multiplicacion */
						if (pid.pwm.dc.counter >= (pid.pwm.dc.ktop * pid.pwm.tb.kdiv_1s[pid.pwm.tb.idx]) )
						{
							if (pid.pwm.dc.ktop < (PID_OUTPUT_MAX*pid.pwm.tb.factor_escala))
							{
								//PinTo0();
							}

							/* este es un caso especial*/
							/* aqui entonces el TOP seria mejor que expusiera el factor de escala */
							if ( ( (PID_OUTPUT_MAX * pid.pwm.tb.factor_escala) - pid.pwm.dc.ktop  ) <= KFRYER_OFF_SEG )
							{
								/* adicionar un tiempo adicional*/
								pid.pwm.tb.toff_adicional_seg = KFRYER_OFF_SEG + 1;

								pid.pwm.tb.sm0 = -1;
							}
							
						}
					}
				}
			}

			/* period */
			if (pid.pwm.dc.counter >= (KFRYER_ON_SEG + (PID_OUTPUT_MAX*pid.pwm.tb.factor_escala) + pid.pwm.tb.toff_adicional_seg )) //Fin de periodo
			{
				pid.pwm.dc.counter = 0;
				//
				pid.pwm.tb.sm0 = 0;
				pid.pwm.dc.ktop = pid.pwm.dc.ktop_buff;	/* update, transfer*/
				//
				if (pid.pwm.dc.ktop > 0)
				{
					//PinTo1();
				}

			}
		}
	}
}
