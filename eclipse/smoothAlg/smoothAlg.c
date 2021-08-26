/*
 * smoothAlg.c
 *
 *  Created on: Aug 25, 2021
 *      Author: jcaf
 */
#include <stdint.h>
#include "smoothAlg.h"

int8_t smoothAlg_nonblock(struct _smoothAlg *smooth, int16_t *buffer, int SMOOTHALG_MAXSIZE, float *Answer)
{
//	static float average=0;
//	static int16_t Pos;	//# de elementos > que la media
//	static int16_t Neg;	//# de elementos > que la media
//	static float TD;	//Total Deviation
//

	//1- Calculate media
	if (smooth->sm0 == 0)
	{
		smooth->average = 0;
		smooth->counter0 = 0x0;
		smooth->sm0++;
	}
	if (smooth->sm0 == 1)
	{
		smooth->average +=buffer[smooth->counter0];

		if (++smooth->counter0 >= SMOOTHALG_MAXSIZE)
		{
			smooth->counter0 = 0x00;//bug fixed

			smooth->average /= SMOOTHALG_MAXSIZE;
			//
			smooth->Pos = 0;
			smooth->Neg = 0;
			smooth->TD = 0;
			smooth->sm0++;
		}
	}
	//2 - Find Pos and Neg + |Dtotal|
	else if (smooth->sm0 == 2)
	{
		if (buffer[smooth->counter0] > smooth->average)
		{
			smooth->Pos++;
			smooth->TD += ( ((float)(buffer[smooth->counter0]))-smooth->average);//Find |Dtotal|
		}
		if (buffer[smooth->counter0] < smooth->average)
		{
			smooth->Neg++;
		}
		//
		if (++smooth->counter0 >= SMOOTHALG_MAXSIZE)
		{
			smooth->counter0 = 0;
			smooth->sm0 = 0;
			//bug
			if (smooth->TD<0)
			{
				smooth->TD *= -1;//convirtiendo a positivo
			}
			//
			*Answer = smooth->average + ( ( (smooth->Pos-smooth->Neg) * smooth->TD )/ ( SMOOTHALG_MAXSIZE*SMOOTHALG_MAXSIZE) );
			return 1;
			//
		}
	}
	return 0;
}

