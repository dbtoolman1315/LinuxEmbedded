#ifndef LED_DRV_H__
#define LED_DRV_H__

#include "led_opr.h"

void register_led_opr(struct led_opr *p);
void led_class_create_device(int minor);
void led_class_destory_device(int minor);


#endif
