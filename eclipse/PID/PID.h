#ifndef PID_H_
#define PID_H_

void PID_init(void);
void PID_reset_errors(void);
int16_t PID_control(int16_t pv);
void PID_set_setpoint(int16_t sp);
int16_t PID_get_setpoint(void);
void PID_control_output(uint8_t pwm_dutycycle);
void PWMSoft_control(void);

/*
 * Esta es la base, pero puede ser reescalado por un factor >1
 */
#define PID_OUTPUT_MIN 0
#define PID_OUTPUT_MAX 10

//2021
/*
Para evitar cargar de ejecuciones de mult/div
mejor escojo la kte directamente
*/
//int8_t 	TB_IDX = 0;					//La opcion, solo 3: 0,1,2
//int8_t 	TB_KDIV_1S[TB_NUM_OPTIONS] = {1,2,4};
//uint16_t TB_KMAX[TB_NUM_OPTIONS]={1000,500,250};//en milisegundos



#define TB_NUM_OPTIONS 3		//TiempoBase
struct _TimeBasePID
{
		int8_t sm0;
		int8_t idx;
		int8_t factor_reescala_pid;
		int8_t kdiv_1s[TB_NUM_OPTIONS];
		int8_t toff_adicional_seg;
		//
		uint16_t counter0;
		uint16_t kmax_ms[TB_NUM_OPTIONS];
};

extern volatile struct _TimeBasePID TB;
#endif // PID_H_
