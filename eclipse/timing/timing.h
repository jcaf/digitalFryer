/*
 * timing.h
 *
 *  Created on: Dec 10, 2020
 *      Author: jcaf
 */

#ifndef TIMING_TIMING_H_
#define TIMING_TIMING_H_


//#define TIME_HOURS_EXIST

//Limits
//#define TIME_LIM_TOP
//#define TIME_LIM_BOTTOM

struct _t
{
	#ifdef TIME_HOURS_EXIST
	int8_t hour;
	#endif
	int8_t min;
	int8_t sec;

	#ifdef TIME_LIM_TOP
	#ifdef TIME_HOURS_EXIST
		int8_t hour_limtop;
	#endif
	int8_t min_limtop;
	int8_t sec_limtop;
	#endif

	#ifdef TIME_LIM_BOTTOM
	#ifdef TIME_HOURS_EXIST
	int8_t hour_limbottom;
	#endif
	int8_t min_limbottom;
	int8_t sec_limbottom;
	#endif

};

int8_t time_dec(struct _t *pt);
int8_t time_inc(struct _t *pt);

#endif /* TIMING_TIMING_H_ */
