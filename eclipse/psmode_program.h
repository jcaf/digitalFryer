/*
 * psmode_program.h
 *
 *  Created on: Aug 25, 2021
 *      Author: jcaf
 */

#ifndef PSMODE_PROGRAM_H_
#define PSMODE_PROGRAM_H_

struct _Tcoccion
{
	int16_t TC;
	int16_t min;
	int16_t max;
};
extern struct _Tcoccion tmprture_coccion;
extern struct _Tcoccion EEMEM TMPRTURE_COCCION;


int8_t psmode_program(void);
extern struct _pgrmmode pgrmode;

#endif /* PSMODE_PROGRAM_H_ */
