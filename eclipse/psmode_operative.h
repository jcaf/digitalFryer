/*
 * psmode_operative.h
 *
 *  Created on: Aug 26, 2021
 *      Author: jcaf
 */

#ifndef PSMODE_OPERATIVE_H_
#define PSMODE_OPERATIVE_H_

void psmode_operative(void);
void psmode_operative_init(void);

extern struct _basket basket_temp[BASKET_MAXSIZE];

extern char lcdanBuff[LCDAN_ROW][LCDAN_COL];

#endif /* PSMODE_OPERATIVE_H_ */
