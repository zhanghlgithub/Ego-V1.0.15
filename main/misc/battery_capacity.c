#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <time.h>


#include "mozart_config.h"
#include "power_interface.h"
#include "tips_interface.h"
#include "modules/mozart_module_local_music.h"
#include "lifting_appliance.h"
#include "mozart_musicplayer.h"
#include "ingenicplayer.h"

#include "battery_capacity.h"
#ifdef SUPPORT_BT
#include "modules/mozart_module_bt.h"
#endif

#define BATTERY_CAPACITY_FILENAME "/sys/devices/platform/jz-i2c.0/i2c-0/0-0062/power_supply/fg-bat/capacity"
#define POWER_ON "/sys/class/gpio/gpio98/value"	//读取电池电量状态
char g_power_on = -1;
int capacity=100;                                                          //电池的电量
bool power_led_config_state=false;                           //����ģʽ��

//关机？？
void power_off()                                                 
{
	int i=20;		       
	printf("\n++++++++++++++++++++++++guanji+++++++++++++++++++++++\n");
	if(mozart_musicplayer_get_status(mozart_musicplayer_handler) == PLAYER_PLAYING)
	{
		mozart_musicplayer_play_pause_pause(mozart_musicplayer_handler);
	}
	//语音提示：正在关机请稍后
	mozart_play_key("power_off");	
				
	system("echo 1 > /sys/class/leds/led-start/brightness"); 
	system("echo 1 > /sys/class/leds/led-wifi2/brightness"); 
	system("echo 1 > /sys/class/leds/led-bt2/brightness"); 
	lifting_appliance_led(6);	//关闭六联指示灯新添加于18.6.11号
	mozart_stop_mode(mozart_mozart_mode);

	/********************新添加于2018.8.10号*********************************/
//	mozart_alarm_stop_alarm();		//新添加于2018.8.10号
	/**********************************************************************/

	mozart_mozart_mode=0;
	lifting_appliance_go_high=0;
	Pack_write(Power_off,NULL,0);
	sleep(2);//添加于2018.7.30号
	while(i)
	{
		printf("\n %d     %d",lifting_appliance_high,lifting_appliance_go_high);
		if(lifting_appliance_high==0||lifting_appliance_high==1)
        	mozart_power_off();
		i--;
		sleep(1);
	}
	mozart_power_off();
}

void power_led_set(int i)
{
	switch(i)
  	{
    	case 1:
			if(!power_led_config_state)
		 	{
		    	system("echo timer > /sys/class/leds/led-pwr/trigger");	
                system("echo 0 > /sys/class/leds/led-pwr/delay_on");	
                system("echo 0 > /sys/class/leds/led-pwr/delay_off");
		        system("echo 0 > /sys/class/leds/led-pwr/brightness");
			
      		    system("echo timer > /sys/class/leds/led-start/trigger");	
                system("echo 0 > /sys/class/leds/led-start/delay_on");	
                system("echo 0 > /sys/class/leds/led-start/delay_off");                                            
		        system("echo 0 > /sys/class/leds/led-start/brightness");
		 	 }
		   break;
        case  2:
			if(!power_led_config_state)
		 	{
		    	system("echo timer > /sys/class/leds/led-pwr/trigger");
			 	system("echo 0 > /sys/class/leds/led-pwr/delay_on");	
                system("echo 0 > /sys/class/leds/led-pwr/delay_off");
		        system("echo 0 > /sys/class/leds/led-pwr/brightness");
				
                system("echo 1 > /sys/class/leds/led-start/brightness");
		       	system("echo timer > /sys/class/leds/led-start/trigger");	
                system("echo 500 > /sys/class/leds/led-start/delay_on");	
                system("echo 500 > /sys/class/leds/led-start/delay_off");
  	        }
		  break;	

		case 3:
			power_led_config_state=true;
		 	system("echo 1 > /sys/class/leds/led-pwr/brightness");
            system("echo timer > /sys/class/leds/led-pwr/trigger");	
            system("echo 500 > /sys/class/leds/led-pwr/delay_on");	
            system("echo 500 > /sys/class/leds/led-pwr/delay_off");
	  
            system("echo 1 > /sys/class/leds/led-start/brightness");
            system("echo timer > /sys/class/leds/led-start/trigger");	
            system("echo 500 > /sys/class/leds/led-start/delay_on");	
            system("echo 500 > /sys/class/leds/led-start/delay_off");	
		   	break;			 

		case 4:
	   		power_led_config_state=true;
	   	    system("echo 1 > /sys/class/leds/led-pwr/brightness");
	   	    system("echo timer > /sys/class/leds/led-pwr/trigger");
			system("echo 500 > /sys/class/leds/led-pwr/delay_on");	
            system("echo 500 > /sys/class/leds/led-pwr/delay_off");

				
		    system("echo timer > /sys/class/leds/led-start/trigger");	
            system("echo 0 > /sys/class/leds/led-start/delay_on");	
            system("echo 0 > /sys/class/leds/led-start/delay_off");                     
			system("echo 0 > /sys/class/leds/led-start/brightness");
	   	   	break;

	   	case 5:
	   		power_led_config_state=false;
            if(capacity>10||capacity<0)
            {
		    	system("echo timer > /sys/class/leds/led-pwr/trigger");	
                system("echo 0 > /sys/class/leds/led-pwr/delay_on");	
                system("echo 0 > /sys/class/leds/led-pwr/delay_off");
		        system("echo 0 > /sys/class/leds/led-pwr/brightness");
			
      		    system("echo timer > /sys/class/leds/led-start/trigger");	
                system("echo 0 > /sys/class/leds/led-start/delay_on");	
                system("echo 0 > /sys/class/leds/led-start/delay_off");                                            
		        system("echo 0 > /sys/class/leds/led-start/brightness");
            }
		    else
		    {
		    	system("echo timer > /sys/class/leds/led-pwr/trigger");
			 	system("echo 0 > /sys/class/leds/led-pwr/delay_on");	
                system("echo 0 > /sys/class/leds/led-pwr/delay_off");
		    	system("echo 0> /sys/class/leds/led-pwr/brightness");
				
                system("echo 1 > /sys/class/leds/led-start/brightness");
		       	system("echo timer > /sys/class/leds/led-start/trigger");	
                system("echo 500 > /sys/class/leds/led-start/delay_on");	
                system("echo 500 > /sys/class/leds/led-start/delay_off");
		    }
		  break;
  	}
  	return;
}

int get_battery_capacity()                            //获取电池电量
{
	FILE *fp = NULL;
  	char *string=NULL;
  
  	if ((fp = fopen(BATTERY_CAPACITY_FILENAME, "r")) == NULL) {
		//printf("fopen error\n");
		return -1;
	}
	
	if ((string = calloc(5, sizeof(char))) != NULL) {
		//printf("\n%s\n",string);
		fread(string, sizeof(char), 4, fp);
		return atoi(string);	
      
	}
	return -1;
}

int power_mode=-1;                          //0: 0%-5%  1:6%-10%  2:11%-100%
int power_last_mode=-1;

void *battery_capacity_thread(void *arg)       //���������߳�
{
	char buf_value[3];
	int fd_value = -1;
	while(1)
	{
		/***********************************test***********************************************/
		fd_value = open(POWER_ON, O_RDONLY);
		read(fd_value,buf_value,3);
		g_power_on = buf_value[0];
		//printf("buf_value:%d,g_power_on:%d................\n",buf_value[0],g_power_on);
		/*************************************************************************************/
		
		capacity = get_battery_capacity();
		if(capacity!=-1)
		{
			printf("Now battery supply and battery_capacity is %d!!\n", capacity);
			if(capacity>10)
			{
				power_mode=2;
				if(power_mode!=power_last_mode)
				{
					power_led_set(1);
					power_last_mode=2;
				}
			}
			else if(capacity>5)
			{
				power_mode=1;
				if(power_mode!=power_last_mode)
				{
                	power_led_set(2);				
					power_last_mode=1;
				}
			}
			else
			{
            	power_mode=0;
				if(power_mode!=power_last_mode)
				{
					if(mozart_musicplayer_get_status(mozart_musicplayer_handler) == PLAYER_PLAYING)
					{
						mozart_musicplayer_play_pause_pause(mozart_musicplayer_handler);
					}
                	//mozart_play_key("charge_prompt"); //电池电量不足，请充电
                    power_led_set(2);
					power_last_mode=0;
				}
			}
			if(capacity < 5 && capacity > 3)
			{
				//电池电量低请充电
				if(g_power_on == 49)		//判断是否外接电源
				{	
					if(mozart_musicplayer_get_status(mozart_musicplayer_handler) == PLAYER_PLAYING)
					{
						mozart_musicplayer_play_pause_pause(mozart_musicplayer_handler);
					}
					mozart_play_key("charge_prompt");
				}
			}

		  	if(capacity < 3)
			{
				//电池电量低，即将关机
				if(g_power_on == 49)	//判断是否在外接电源
				{
					if(mozart_musicplayer_get_status(mozart_musicplayer_handler) == PLAYER_PLAYING)
					{
						mozart_musicplayer_play_pause_pause(mozart_musicplayer_handler);
					}
					mozart_play_key("low_power_off");	
		       	 	power_off();
				}
				
			}
			else 
			{
				if(reply_time>=60)
			  	{
                	power_off();
			  	}
				//新添加了逻辑：当g_power_on == 49 表示当前没有给base充电，此时认为进入省电模式//新添加于2018.7.30号
                if((!get_cuband_state()) && (mozart_mozart_mode == FREE) && (mozart_player_getstatus(mozart_musicplayer_handler->player_handler)!=PLAYER_PLAYING) && (g_power_on == 49))
               	{
               		 reply_time++;
                }
				else
				{
					reply_time=0;
				}
			}
		}
		close(fd_value);
		sleep(10);
	}
	return NULL;
}

void *manger_tower_battery_thread(void *arg)
{
	int state = 0;			//1：正在充电，0：断开充电
	int state_tower = 1;	// 1:电量大于20, 0：电量小于20
	/*************************************************************************/
	//sleep(10);
	/************************************************************************/
	
//	struct info *info;
	if((capacity > 40) || (g_power_on == 48)){			//新添加于2018.7.30号
		lifting_appliance_control(4);
		state = 1;
	}
	while(1)
	{	
		if((capacity > 40) || (g_power_on == 48))	//逻辑：base电量大于40给tower充电，或者当前正在充电//新添加于2018.7.30号
		{
			
			if(g_tower_power > 98 && (state == 1)){
				lifting_appliance_control(5);		//断电
				state = 0;
				printf("断电。。。。%d。。。\n",info->Tower_if_power);	
				//sleep(10);
			}
			if(g_tower_power < 10 && (state == 0)){
				lifting_appliance_control(4);			//充电
				state = 1;
				//printf("升到1档。。。。。。。\n");	
				//sleep(10);
			}
		}
		if((capacity < 40) && (g_power_on == 49))		//电量小于40且没外接电源，不给tower充电
		{
			lifting_appliance_control(5);
			state = 0;
		}
		if(g_tower_power < 20)
		{
		//	printf("tower 电量小于20\n");
			lifting_appliance_go_high = 0;
		}
		
		//新添加于2018.7.25号(安全性措施)
		if(g_tower_power < 20 && state_tower == 1 && get_tower_state())	//加上判断是否当前正在连接tower
		{
			state_tower = 0;
			lifting_appliance_go_high = 0;
			//添加语言提示“意念魔方电量低”
			if(mozart_musicplayer_get_status(mozart_musicplayer_handler) == PLAYER_PLAYING)
			{
				mozart_musicplayer_play_pause_pause(mozart_musicplayer_handler);
			}
			//printf("语音提示电量意念魔方电量低\n");
			mozart_play_key("tower_low_battery");	
		}
		if(((g_tower_power > 20) && state_tower == 0) || ((0==get_tower_state()) && state_tower == 0))
		{
			state_tower = 1;
		}
		sleep(1);
	}
}

void register_battery_capacity(void)
{
	pthread_t recv_event;
	pthread_t tower_event;
	if(0 != pthread_create(&recv_event, NULL, battery_capacity_thread, NULL))
		printf("Can't create battery_capacity!\n");
	pthread_detach(recv_event);

	if(0 != pthread_create(&tower_event, NULL, manger_tower_battery_thread, NULL))
		printf("Can't create manger_tower_battery_thread!\n");
	pthread_detach(tower_event);
}

void unregister_battery_capacity(void)
{

}
