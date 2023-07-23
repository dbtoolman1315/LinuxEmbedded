#ifndef GPIO_DRV_H__
#define GPIO_DRV_H__

struct key_operations
{
	int count;
	int (*init) (int which);
	int (*opr) (int which);
};

void register_key_operations(struct key_operations *opr);
void unregister_key_operations(struct key_operations *opr);

#endif
