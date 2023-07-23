

/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2019-12-11 21:53:07
 * @LastEditTime : 2022-06-12 17:48:06
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include <string.h>
#include "mqtt_config.h"
#include "mqtt_log.h"
#include "mqttclient.h"

// #define TEST_USEING_TLS  
// extern const char *test_ca_get();

static struct subscribe_des
{
	char name[10];
	int fd;
	void (*handler )(void* , message_data_t* );
};

typedef struct mqtt
{
	char ip[20];
	char port[10];
	int subscribe_num;
	int cnt;
	struct subscribe_des *st_subscribe_des;
	mqtt_client_t *client;
}ST_MQTT_DES;

static print_help()
{
	printf("-P --port    端口号\n");
    printf("-i --ip      ip地址\n");
    printf("-n --num     订阅数量\n");
    printf("-h --help    帮助\n");
}
static void topic1_handler(void* client, message_data_t* msg)
{
    (void) client;
    MQTT_LOG_I("-----------------------------------------------------------------------------------");
    MQTT_LOG_I("%s:%d %s()...\ntopic: %s\nmessage:%s", __FILE__, __LINE__, __FUNCTION__, msg->topic_name, (char*)msg->message->payload);
    MQTT_LOG_I("-----------------------------------------------------------------------------------");
}

void *mqtt_publish_thread(void *arg)
{
    ST_MQTT_DES *mqtt_des = (ST_MQTT_DES *)arg;
	int cnt = 0;
	int val;
	char i2c_buf[6];
	char str[6];
    char buf[100] = { 0 };
    mqtt_message_t msg;
    memset(&msg, 0, sizeof(msg));
    sprintf(buf, "welcome to mqttclient, this is a publish ...");

    sleep(2);

    mqtt_list_subscribe_topic(mqtt_des->client);

    msg.payload = (void *) buf;
    
    while(1) 
	{
		/*i2c*/
		read(mqtt_des->st_subscribe_des[cnt++].fd,i2c_buf,sizeof(i2c_buf) );
        msg.qos = 0;
		if(i2c_buf)
		{
			for(int len = 0; len < 6; len++)
			{
				sprintf(&str[len], "%d", i2c_buf[len]);
				printf("%02x:",buf[len]);
				fflush(NULL);
			}
			msg.payload = (void *)str;
			mqtt_publish(mqtt_des->client, "i2c", &msg);
		}

		/*key*/
		read(mqtt_des->st_subscribe_des[cnt++].fd,&val,4);
		{
			sprintf(str, "%d", val);
			msg.payload = (void *)str;
			printf("key:0x%x",val);
			mqtt_publish(mqtt_des->client, "key", &msg);
		}
		
        cnt = 0;
        sleep(5);
    }
}

static int mqtt_init(ST_MQTT_DES *pst_mqtt_des)
{
	int res;
    pthread_t thread1;
    int qosn[] = {QOS0,QOS1,QOS2};
    printf("\nwelcome to mqttclient test...\n");

    mqtt_log_init();

    pst_mqtt_des->client = mqtt_lease();

    mqtt_set_port(pst_mqtt_des->client, pst_mqtt_des->port);

    mqtt_set_host(pst_mqtt_des->client, pst_mqtt_des->ip); /* iot.100ask.net */
    mqtt_set_client_id(pst_mqtt_des->client, random_string(10));
    mqtt_set_user_name(pst_mqtt_des->client, random_string(10));
    mqtt_set_password(pst_mqtt_des->client, random_string(10));
    mqtt_set_clean_session(pst_mqtt_des->client, 1);

    mqtt_connect(pst_mqtt_des->client);

	for(int i = 0; i < pst_mqtt_des->subscribe_num; i++)
	{
		mqtt_subscribe(pst_mqtt_des->client, pst_mqtt_des->st_subscribe_des[i].name , qosn[i], topic1_handler);
	}
    
	res = pthread_create(&thread1, NULL, mqtt_publish_thread, pst_mqtt_des);
    return res;
}	

static int i2c_init(ST_MQTT_DES *pst_mqtt_des)
{
	int fd;
	int len;
	

	/* 2. 打开文件 */
	fd = open("/dev/ap3216_i2c_dev", O_RDWR);
	if (fd == -1)
	{
		return -1;
	}
	pst_mqtt_des->st_subscribe_des[pst_mqtt_des->cnt++].fd = fd;
	return 0;
	//len = read(fd, buf, 6);	
}

static int key_init(ST_MQTT_DES *pst_mqtt_des)
{
	int fd;
	fd = open("/dev/100ask_gpio_key", O_RDWR);
	if (fd == -1)
	{
	
		return -1;
	}
	pst_mqtt_des->st_subscribe_des[pst_mqtt_des->cnt++].fd = fd;
	return 0;
}
int main(int argc, char **argv)
{
	int res,index,c;
	ST_MQTT_DES st_mqtt_des;
	char name[10];
	struct option opt1[] = {
                    {"port",1,NULL,'p'},
                    {"ip,",1,NULL,'i'},
                    {"num",1,NULL,'n'},
                    {"help",1,NULL,'h'},
                    {NULL,0,NULL,0}
	};
	if(argc < 4)
	{
		printf("usage ...\n");
		return 0;
	}
	while(1)
	{
		c = getopt_long(argc,argv,"p:i:n:h",opt1,&index);
		if(c < 0)
		{
			printf("getopt_long ok\n");
			break;
		}

		switch (c)
		{
			case 'p':
				strcpy(st_mqtt_des.port, optarg);
				break;
			case 'i':
				strcpy(st_mqtt_des.ip, optarg);
				break;
			case 'n':
				st_mqtt_des.subscribe_num = atoi(optarg);
				st_mqtt_des.st_subscribe_des =  malloc(sizeof(struct subscribe_des) * st_mqtt_des.subscribe_num);
				if(st_mqtt_des.st_subscribe_des == NULL)
				{
					printf("malloc st_subscribe_des failed\n");
					return 0;
				}
				for(int i = 0; i < st_mqtt_des.subscribe_num; i++)
				{	
					printf("please input sub name:");
					fgets(name,sizeof(name),stdin);
					name[strcspn(name, "\n")] = '\0';
					strncpy(st_mqtt_des.st_subscribe_des[i].name, name, sizeof(name));

				}
				break;
			case 'h':
				print_help();
				return 0;
			default:
				return 0;
		}
	}
	st_mqtt_des.cnt = 0;
	res = i2c_init(&st_mqtt_des);
	if(res != 0)
	{
		printf("i2c_init failed\n");
		free(st_mqtt_des.st_subscribe_des);
		return 0;
	}

	res = key_init(&st_mqtt_des);
	if(res != 0)
	{
		printf("key_init failed\n");
		free(st_mqtt_des.st_subscribe_des);
		return 0;
	}
	
    res = mqtt_init(&st_mqtt_des);
    if(res != 0)
    {
		printf("publish_thread failed\n");
		free(st_mqtt_des.st_subscribe_des);
		return 0;
    }
    
	
    while (1) {
        sleep(100);
    }

	free(st_mqtt_des.st_subscribe_des);
}


