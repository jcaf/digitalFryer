#ifndef PID_H_
#define PID_H_


struct PIDtiming
{
	int8_t sm0;
	uint16_t counter_ticks_ms;
	uint16_t kmax_ticks_ms;
	int8_t kton_ms;
	int8_t ktoff_ms;
};

struct DutyCycle
{
	uint8_t counter_ms;
	uint8_t ktop_ms;
	uint8_t ktop_uploaded_ms; /* for next period*/
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

void pid_job(void);

#endif // PID_H_
