#ifndef LED_OPR_H__
#define LED_OPR_H__
struct led_opr 
{
	int (*init)(int which);
	int (*opr) (int which, char state);
};


#endif
