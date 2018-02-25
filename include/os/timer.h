#ifndef __TIMER_H__
#define __TIMER_H__

#include <linux/timer.h>

struct _timer_list {
	struct timer_list t;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0)
       void (*function)(unsigned long);
       unsigned long data;
#endif
};

#endif