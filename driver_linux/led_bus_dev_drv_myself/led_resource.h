#ifndef LED_RESOURCE_H__
#define LED_RESOURCE_H__


#define GROUP(x) (x >> 16)
#define PIN(x) (x & 0xffff)
#define GROUP_PIN(g, p) ((g << 16) | p)

#endif
