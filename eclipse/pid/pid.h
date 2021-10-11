#ifndef PID_H_
#define PID_H_

/*
 * Esta version es usando puros enteros
 * */
//#define PID_INTEGERS
#define PID_FLOATS
#ifdef PID_INTEGERS
	struct PIDtiming
	{
		int8_t sm0;
		uint16_t counter_ticks_ms;
		uint16_t kmax_ticks_ms;
		int8_t k_systemdelay_ton_ms;
		int8_t k_systemdelay_toff_ms;
	};

	struct DutyCycle
	{
		uint8_t counter_ms;
		int16_t ktop_ms;
		int16_t ktop_uploaded_ms; /* for next period*/
	};

	struct Algorithm
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
		//
		int8_t pid_out_max_ms;
		int8_t pid_out_min_ms;
		//
		int8_t scaler_time_ms;
	};
#else //PID_FLOATS
	struct PIDtiming
	{
		int8_t sm0;
		uint16_t counter_ticks_ms;
		uint16_t kmax_ticks_ms;
		int8_t k_systemdelay_ton_ms;
		int8_t k_systemdelay_toff_ms;
	};

	struct DutyCycle
	{
		uint8_t counter_ms;
		int16_t ktop_ms;
		int16_t ktop_uploaded_ms; /* for next period*/
	};

	struct Algorithm
	{
		int16_t sp;			/* set point */

		//
		int16_t ei; 		/* error integral */
		int16_t eprevio;	/* error previo */
		int16_t ed;			/* error derivativo	*/
		//
		int8_t kei_windup_max;	/* max windup for error integral */
		int8_t kei_windup_min;	/* min windup for error integral */
		//
		int8_t pid_out_max_ms;
		int8_t pid_out_min_ms;
		//
		int8_t scaler_time_ms;


		float kp;
		float ki;
		float kd;
	};

#endif

struct PID
{
	struct Algorithm algo;
	struct PWM
	{
		struct DutyCycle dc;
		struct PIDtiming timing;
		//
		struct IOport
		{
			volatile unsigned char *port;
			int8_t pin;
		}io;

	}pwm;
};
void pid_job(struct PID *pid, int16_t error);
void pid_pwm_set_pin(struct PID *pid);
void pid_set_ktop_ms(struct PID *pid, int16_t error);

#endif // PID_H_
