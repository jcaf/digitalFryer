#include "../system.h"
#include "../main.h"

#include "PID.h"
//#include "../usart/usart.h"
struct _PWMSoft
{
    struct _dc
    {
        uint8_t TOP;
        uint8_t TOP_buff;
        uint8_t counter;
    }dc;
    uint16_t period;

}PWMSoft;

struct _PID
{
    //int16_t initial_error;
    int16_t setpoint;
    //
//    int16_t K_proportional;// = 40;
//    int16_t K_integral;// = 10;
//    int16_t K_derivative;// = 0;
    float K_proportional;// = 40;
    float K_integral;// = 10;
    float K_derivative;// = 0;
    //
    int16_t error_integral;
    int16_t previous_error;
    int16_t error_derivative;
};

struct _PID PID;

/*****************************************************
*****************************************************/
void PID_set_setpoint(int16_t sp)
{
    PID.setpoint = sp;
}
int16_t PID_get_setpoint(void)
{
    return PID.setpoint;
}
void PID_reset_errors(void)
{
    //PID.initial_error = PID.setpoint - temper_measured;
    PID.error_integral = 0;
    PID.previous_error = 0;
}

void PID_init(void)//cambiar de nombre
{
    PID_reset_errors();

    PID.K_proportional = 1.2;
    PID.K_integral = 1.45;
    PID.K_derivative = 1;//
}
/*****************************************************
*****************************************************/
#define PID_CONTROL_DEBUG
int16_t PID_control(int16_t pv)
{
    int16_t error;
    int16_t PID_output;
    int8_t _pid_integral_windup;

    error = PID.setpoint - pv;

    PID.error_integral = PID.error_integral + error;

    if (error > 25)
    {
        _pid_integral_windup =  90;//70;//ok con etas ktes...
    }
    else if (error > 7)//15)
    {
        if (pv > 900)
        {
            _pid_integral_windup =  80;//35;
        }
        else if (pv > 450)
        {
            _pid_integral_windup =  70;//45;//35;
        }
        else if (pv > 200)
        {
            _pid_integral_windup =  60;//45;//35;
        }
        else if (pv > 100 )
        {
            _pid_integral_windup =  35;//45;//35;
        }
        else if (pv > 50 )
        {
            _pid_integral_windup =  25;//45;//35;
        }
        else
        {
            _pid_integral_windup =  20;//45;//35;
        }
    }
    else if (error >3 )
    {
        if (pv > 900)
        {
            _pid_integral_windup =  56;//20;
        }
        else if (pv > 450)
        {
            _pid_integral_windup =  35;//45;//35;
        }
        else if (pv > 200)
        {
            _pid_integral_windup =  25;//45;//35;
        }
        else if (pv > 100 )
        {
            _pid_integral_windup =  10;//45;//35;
        }
        else if (pv > 50 )
        {
            _pid_integral_windup =  5;//45;//35;
        }
        else
        {
            _pid_integral_windup =  2;//45;//35;
        }
    }
    else//3 2 1 0
    {
        if (pv > 900)
        {
            _pid_integral_windup =  45;//20;
        }
        else if (pv > 450)
        {
            _pid_integral_windup =  35;//45;//35;
        }
        else if (pv > 200)
        {
            _pid_integral_windup =  25;//45;//35;
        }
        else if (pv > 100 )
        {
            _pid_integral_windup =  10;//45;//35;
        }
        else if (pv > 50 )
        {
            _pid_integral_windup =  5;//45;//35;
        }
        else
        {
            _pid_integral_windup =  2;//45;//35;
        }

    }

    if (PID.error_integral > _pid_integral_windup)//integral windup
        {PID.error_integral = _pid_integral_windup;}
    else if(PID.error_integral < 0)//-_pid_integral_windup)
        {PID.error_integral = 0;}//-_pid_integral_windup;}//-10;

    PID.error_derivative = error - PID.previous_error;
    PID.previous_error = error;

    //
    PID_output = (PID.K_proportional*error)+(PID.K_integral*PID.error_integral)+(PID.K_derivative*PID.error_derivative);


    if(PID_output > PID_CONTROL_OUTPUT_MAX)
        {PID_output = PID_CONTROL_OUTPUT_MAX;}
    else if(PID_output < 0)
        {PID_output = 0;}

    //
    return PID_output;
}

/*****************************************************
PID_control_output call from ISR
*****************************************************/
#define PWM_BASETIME_COUNTER_MAX 1//xdirectamente cada 20ms
#define PWM_PERIOD_COUNTER_MAX PID_CONTROL_OUTPUT_MAX //100


/*
//void pwm_dutycycle_kstop(int PID)
void pwm_dutycycle_kstop(float PID)
{
    dutycyle_k_upper = ((float)(PID/PID_MAX)) * PWM_PERIOD_COUNTER_MAX;
}
*/
//void pwm_dutycycle_kstop(float PID_porcentage)
//{
//    dutycyle_ = (PID_porcentage) * PWM_PERIOD_COUNTER_MAX;
//}

/*
Cada nuevo periodo se carga un nuevo duty cyle
*/
void PWMSoft_control()//cada 1 segundo
{
    PWMSoft.dc.counter++;

    if (PWMSoft.dc.counter >= PWMSoft.dc.TOP)
    {
        if (PWMSoft.dc.TOP < PWMSoft.period)
            {
        	//PinTo0();
            }
    }

    if (PWMSoft.dc.counter >= PWMSoft.period)//max 255
    {
        PWMSoft.dc.counter = 0;
        PWMSoft.dc.TOP = PWMSoft.dc.TOP_buff;
        //
        if (PWMSoft.dc.TOP > 0)
        {
            //PinTo1();
        }

    }
}
