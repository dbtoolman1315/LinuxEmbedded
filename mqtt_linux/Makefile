
#CFLAGS = -I include/test/  -I include/common/ -I include/mqttclient  -I include/ -I include/common/log -I include/mqtt  -I ./include/platform/linux  -I ./include/network 
#LDFLAGS = -L . -lmqttclient -lpthread
#KERN_DIR = /home/ubuntu/桌面/embedeed_linux/100ask_imx6ull-sdk/Linux-4.9.88
all:
	#arm-buildroot-linux-gnueabihf-gcc -o mqtt_test mqtt_test.c $(CFLAGS) $(LDFLAGS)
	make -C $(KERN_DIR) M=`pwd` modules
clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	#rm mqtt_test

obj-m	+= i2c_driver.o
obj-m   += gpio_key_drv.o


arm-buildroot-linux-gnueabihf-gcc -o mqtt_test mqtt_test.c -I include/test/  -I include/common/ -I include/mqttclient  -I include/ -I include/common/log -I include/mqtt  -I ./include/platform/linux  -I ./include/network -L . -lmqttclient -lpthread