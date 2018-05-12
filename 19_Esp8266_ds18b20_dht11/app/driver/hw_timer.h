/*
 * hw_timer.h
 *
 *  Created on: 2018年3月13日
 *      Author: Administrator
 */

#ifndef _HW_TIMER_H_
#define _HW_TIMER_H_

#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"

#define US_TO_RTC_TIMER_TICKS(t)          \
    ((t) ?                                   \
     (((t) > 0x35A) ?                   \
      (((t)>>2) * ((APB_CLK_FREQ>>4)/250000) + ((t)&0x3) * ((APB_CLK_FREQ>>4)/1000000))  :    \
      (((t) *(APB_CLK_FREQ>>4)) / 1000000)) :    \
     0)

#define FRC1_ENABLE_TIMER  BIT7
#define FRC1_AUTO_LOAD  BIT6

//TIMER PREDIVED MODE
typedef enum {
	DIVDED_BY_1 = 0,		//timer clock
	DIVDED_BY_16 = 4,	//divided by 16
	DIVDED_BY_256 = 8,	//divided by 256
} TIMER_PREDIVED_MODE;

typedef enum {			//timer interrupt mode
	TM_LEVEL_INT = 1,	// level interrupt
	TM_EDGE_INT = 0,	//edge interrupt
} TIMER_INT_MODE;

typedef enum {
	FRC1_SOURCE = 0, NMI_SOURCE = 1,
} FRC1_TIMER_SOURCE_TYPE;

#define	REG_READ(_r)	(*(volatile	uint32	*)(_r))

#define	WDEV_NOW()		REG_READ(0x3ff20c00)

extern void hw_timer_init(FRC1_TIMER_SOURCE_TYPE source_type, u8 req);
extern void hw_timer_arm(u32 val);

#endif /* _HW_TIMER_H_ */
