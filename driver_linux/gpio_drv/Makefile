
KERN_DIR = 

all:
	make -C $(KERN_DIR) M=`pwd` modules
	$(CROSS_COMPILE)gcc -o gpio_test gpio_test.c

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f gpio_test

obj-m += gpio_dev.o
obj-m += key_board.o