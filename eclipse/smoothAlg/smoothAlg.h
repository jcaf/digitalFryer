/*
 * smoothAlg.h
 *
 *  Created on: Aug 25, 2021
 *      Author: jcaf
 */

#ifndef SMOOTHALG_SMOOTHALG_H_
#define SMOOTHALG_SMOOTHALG_H_

struct _smoothAlg
{
	int8_t sm0;		//x jobs
	uint16_t counter0;
	float average;
	int16_t Pos;	//# de elementos > que la media
	int16_t Neg;	//# de elementos > que la media
	float TD;		//Total Deviation
	int SMOOTHALG_MAXSIZE;
};

int8_t smoothAlg_nonblock(struct _smoothAlg *smooth, int16_t *buffer, int SMOOTHALG_MAXSIZE, float *Answer);

#endif /* SMOOTHALG_SMOOTHALG_H_ */
