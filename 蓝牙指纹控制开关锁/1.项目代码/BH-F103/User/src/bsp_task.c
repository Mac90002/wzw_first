#include "bsp_task.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_led.h"
#include "bsp_key.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"
#include "bsp_SysTick.h"
#include "bsp_ili9341_lcd.h"
#include "bsp_hc05.h"
#include "bsp_usart.h"
#include "timers.h"
#include "bsp_hc05_usart.h"

extern volatile    uint16_t uart_p;
extern uint8_t     uart_buff[UART_BUFF_SIZE];
void timer2Callback(TimerHandle_t pxTimer);
uint8_t a = 0 ;

TaskHandle_t start_task_handler;
void start_task(void *pvParameters);

TaskHandle_t task1_handler;
void task1(void *pvParameters);

TaskHandle_t task2_handler;
void task2(void *pvParameters);

TaskHandle_t task3_handler;
void task3(void *pvParameters);

QueueHandle_t semaphore_handle ;
TimerHandle_t tim2_handle = 0; /*周期定时器*/

uint32_t i = 0 ;
static uint32_t timer = 0 ;

void OS_Task(void)
{
  xTaskCreate((TaskFunction_t)start_task,
              (char *)"start_task",
              (uint16_t)START_TASK_STACK_SIZE,
              (void *)NULL,
              (UBaseType_t)START_TASK_PRIO,
              (TaskHandle_t *)&start_task_handler);
  vTaskStartScheduler();
}


void start_task(void *pvParameters)
{
  taskENTER_CRITICAL(); /* 进入临界区 */
  semaphore_handle = xSemaphoreCreateBinary() ;
  if(semaphore_handle != NULL){
    printf("binary create susceed\n");
  }
  tim2_handle = xTimerCreate("tim2",1000,pdTRUE,(void *)2,timer2Callback);
  xTimerStart(tim2_handle,portMAX_DELAY);

  xTaskCreate((TaskFunction_t)task1,
              (char *)"task1",
              (uint16_t)TASK1_STACK_SIZE,
              (void *)NULL,
              (UBaseType_t)TASK1_PRIO,
              (TaskHandle_t *)&task1_handler);

  xTaskCreate((TaskFunction_t)task2,
              (char *)"task2",
              (uint16_t)TASK2_STACK_SIZE,
              (void *)NULL,
              (UBaseType_t)TASK2_PRIO,
              (TaskHandle_t *)&task2_handler);

  xTaskCreate((TaskFunction_t)task3,
              (char *)"task3",
              (uint16_t)TASK3_STACK_SIZE,
              (void *)NULL,
              (UBaseType_t)TASK3_PRIO,
              (TaskHandle_t *)&task3_handler);

  vTaskDelete(NULL);
  taskEXIT_CRITICAL(); /* 退出临界区 */
}


void task1(void *pvParameters)
{
  BaseType_t err ;
	
  while (1)
  {
    err = xSemaphoreTake(semaphore_handle,1);/*获取信号量并等待1000*/
    if(err == pdTRUE){
				err = pdFAIL ;
     if( ! IS_HC05_CONNECTED() )
     {
       HC05_Send_CMD("AT+INQ\r\n",1);//模块在查询状态，才能容易被其它设备搜索到
			 ILI9341_DispString_EN(40,80,"Please connect Bluetooth");
       printf("蓝牙尚未连接。请用手机打开蓝牙调试助手搜索连接蓝牙\r\n" );
     }
     else
     {
			 if(a == 0){
			 ILI9341_Clear(0,0,240,320);
			 }
			 
			 a = 1 ;
			 ILI9341_DispString_EN(20,40,"HC05 connection successful");
			 ILI9341_DispString_EN(20,80,"Enter your password to open the door");

       printf("蓝牙已连接。发送“RED_LED”可控制翻转LED灯\r\n" );
     }
    }
    TransData_CtrlLED_Test();
    vTaskDelay(100);
  }
}
	


void task2(void *pvParameters)
{
  while (1)
  {
		
		if(timer == 3){
          if(semaphore_handle != NULL){
          xSemaphoreGive(semaphore_handle);
          }
    }
    vTaskDelay(100);
  }
}

void task3(void *pvParameters){
			
		while(1){
			vTaskDelay(1000);
		}
}



void timer2Callback(TimerHandle_t pxTimer){
  
  //printf("timer2 running frequency : %d \r\n",++timer);
	++timer;
  if(timer == 4){
    timer = 0 ;
  }
}
