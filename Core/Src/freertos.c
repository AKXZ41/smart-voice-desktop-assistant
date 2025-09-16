/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <string.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "esp8266.h"
#include "i2c_led.h"
#include "voice.h"
#include "i2c.h"
#include "usart.h"
#include "ds1302.h"
#include "weather.h"
#include "countdown.h"
#include "display_manager.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticEventGroup_t osStaticEventGroupDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for TASK_INIT */
osThreadId_t TASK_INITHandle;
const osThreadAttr_t TASK_INIT_attributes = {
  .name = "TASK_INIT",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for TASK_SHOW */
osThreadId_t TASK_SHOWHandle;
const osThreadAttr_t TASK_SHOW_attributes = {
  .name = "TASK_SHOW",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for TASK_RUN */
osThreadId_t TASK_RUNHandle;
const osThreadAttr_t TASK_RUN_attributes = {
  .name = "TASK_RUN",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TASK_READ */
osThreadId_t TASK_READHandle;
const osThreadAttr_t TASK_READ_attributes = {
  .name = "TASK_READ",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TASK_WIFI */
osThreadId_t TASK_WIFIHandle;
const osThreadAttr_t TASK_WIFI_attributes = {
  .name = "TASK_WIFI",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Timer_ds1302_read */
osTimerId_t Timer_ds1302_readHandle;
const osTimerAttr_t Timer_ds1302_read_attributes = {
  .name = "Timer_ds1302_read"
};
/* Definitions for Binary_Sem_1 */
osSemaphoreId_t Binary_Sem_1Handle;
const osSemaphoreAttr_t Binary_Sem_1_attributes = {
  .name = "Binary_Sem_1"
};
/* Definitions for Binary_Sem_2 */
osSemaphoreId_t Binary_Sem_2Handle;
const osSemaphoreAttr_t Binary_Sem_2_attributes = {
  .name = "Binary_Sem_2"
};
/* Definitions for Event_01 */
osEventFlagsId_t Event_01Handle;
osStaticEventGroupDef_t Event_01C;
const osEventFlagsAttr_t Event_01_attributes = {
  .name = "Event_01",
  .cb_mem = &Event_01C,
  .cb_size = sizeof(Event_01C),
};
/* Definitions for Event_02 */
osEventFlagsId_t Event_02Handle;
osStaticEventGroupDef_t Event_02C;
const osEventFlagsAttr_t Event_02_attributes = {
  .name = "Event_02",
  .cb_mem = &Event_02C,
  .cb_size = sizeof(Event_02C),
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
// 同步机制使用说明：
// Binary_Sem_1: 保护ESP8266通信资源
// Binary_Sem_2: 保护天气数据缓存
// Event_01: DS1302时钟读写控制 (bit0: 读取请求, bit1: 写入请求)
// Event_02: OLED显示控制 (bit0: 时间显示, bit1: 天气显示, bit2: 刷新请求)

/* USER CODE END FunctionPrototypes */

void task_init(void *argument);
void task_show(void *argument);
void task_read(void *argument);
void task_wifi(void *argument);
void Callback_ds1302_read(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of Binary_Sem_1 */
  Binary_Sem_1Handle = osSemaphoreNew(1, 1, &Binary_Sem_1_attributes);

  /* creation of Binary_Sem_2 */
  Binary_Sem_2Handle = osSemaphoreNew(1, 1, &Binary_Sem_2_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of Timer_ds1302_read */
  Timer_ds1302_readHandle = osTimerNew(Callback_ds1302_read, osTimerPeriodic, NULL, &Timer_ds1302_read_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of TASK_INIT */
  TASK_INITHandle = osThreadNew(task_init, NULL, &TASK_INIT_attributes);

  /* creation of TASK_SHOW */
  TASK_SHOWHandle = osThreadNew(task_show, NULL, &TASK_SHOW_attributes);

  /* creation of TASK_RUN */
  TASK_RUNHandle = osThreadNew(task_show, NULL, &TASK_RUN_attributes);

  /* creation of TASK_READ */
  TASK_READHandle = osThreadNew(task_read, NULL, &TASK_READ_attributes);

  /* creation of TASK_WIFI */
  TASK_WIFIHandle = osThreadNew(task_wifi, NULL, &TASK_WIFI_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Create the event(s) */
  /* creation of Event_01 */
  Event_01Handle = osEventFlagsNew(&Event_01_attributes);

  /* creation of Event_02 */
  Event_02Handle = osEventFlagsNew(&Event_02_attributes);

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_task_init */
/**
  * @brief  Function implementing the TASK_INIT thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_task_init */
void task_init(void *argument)
{
  /* USER CODE BEGIN task_init */
  Esp8266Handle *g_esp = usart_get_esp8266_handle();
  
  // 初始化各个模块
  esp8266_init(g_esp, &huart2);
  i2c_led_init(&hi2c1);
  voice_init();
  voice_bind_uart(&huart1);  // 用于VC02通信
  ds1302_init();
  weather_init("101290101", "ea5aa92b06a04c84bf38775d5586a48f");
  countdown_init();  // 初始化倒计时模块
  display_manager_init();  // 初始化显示管理模块
  
  // 等待ESP8266启动完成
  osDelay(3000);
  
  // 发送AT命令并等待响应
  esp8266_send_at_async(g_esp, "AT");
  osDelay(2000);
  
  // 检查AT响应
  char response[128];
  if (esp8266_read_line(g_esp, response, sizeof(response), 1000)) {
    // AT命令有响应，继续
    esp8266_send_at_async(g_esp, "ATE0");
    osDelay(1000);
    
    esp8266_send_at_async(g_esp, "AT+CWMODE=1");
    osDelay(1000);
    
    // 使用PC13 LED指示连接状态
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);  // 点亮LED表示通信正常
  } else {
    // AT命令无响应，LED快速闪烁表示通信失败
    for(int i = 0; i < 10; i++) {
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
      osDelay(100);
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
      osDelay(100);
    }
  }
  
  // 尝试连接WiFi
  bool wifi_connected = weather_wifi_connect(g_esp, "RedmiK60", "1234567899", 20000);
  
  // 通过PC13 LED指示WiFi连接状态
  if (wifi_connected) {
    // 连接成功：LED常亮
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    
    // 连接成功后同步网络时间到DS1302
    weather_sync_time_from_network(g_esp);
  } else {
    // 连接失败：LED慢速闪烁
    for(int i = 0; i < 10; i++) {
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
      osDelay(300);
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
      osDelay(300);
    }
  }
  
  // 初始化完成后，进入ESP8266发送处理循环
  for(;;)
  {
    (void)esp8266_task_tx_poll(g_esp, 50);
    osDelay(10);
  }
  /* USER CODE END task_init */
}

/* USER CODE BEGIN Header_task_show */
/**
* @brief Function implementing the TASK_SHOW thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_task_show */
void task_show(void *argument)
{
  /* USER CODE BEGIN task_show */
  for(;;)
  {
    // 每秒触发时间刷新
    display_manager_force_refresh(DisplayRefresh_Time);
    
    osDelay(1000);
  }
  /* USER CODE END task_show */
}

/* USER CODE BEGIN Header_task_read */
/**
* @brief Function implementing the TASK_READ thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_task_read */
void task_read(void *argument)
{
  /* USER CODE BEGIN task_read */
  VoiceCmd cmd;
  static uint32_t test_counter = 0;
  
  for(;;)
  {
    // 每10秒测试一次语音功能（用于调试）
    test_counter++;
    if (test_counter >= 10000) {  // 10秒
      test_counter = 0;
      // 可以在这里添加测试命令
      // voice_simulate_input("你好小艾");
    }
    
    // 等待语音指令
    if (voice_get_cmd(&cmd, 100)) {
      switch (cmd) {
        case VoiceCmd_Introduce:
          voice_send_reply("你好我叫小艾 我是一个桌面助手 你可以通过语音和我进行交互 你可以说天气如何 更改时间 更换图片 有什么需要尽管给我说");
          break;
          
        case VoiceCmd_Hello:
          if (weather_is_connected()) {
            voice_send_reply("随时待命");
          } else {
            voice_send_reply("好你妈，快给我把网连上");
          }
          break;
          
        case VoiceCmd_WakeUp:
          if (weather_is_connected()) {
            voice_send_reply("我在我在");
          } else {
            voice_send_reply("别小了，没网络了");
          }
          break;
          
        case VoiceCmd_Weather: {
          // 检查网络状态
          if (weather_is_connected()) {
            // 获取天气信息并回复
            char desc[32], temp[8];
            weather_get_last(desc, sizeof(desc), temp, sizeof(temp));
            char reply[64];
            snprintf(reply, sizeof(reply), "今天天气%s摄氏度，湿度%s度", temp, desc);
            voice_send_reply(reply);
          } else {
            voice_send_reply("我也不知道了喵，没有网络连接喵，要不你出门看一眼吧");
          }
          break;
        }
        
        case VoiceCmd_Time: {
          // 检查网络状态
          if (weather_is_connected()) {
            // 获取当前时间并回复
            RtcDateTime now;
            if (ds1302_read(&now)) {
              char reply[32];
              snprintf(reply, sizeof(reply), "现在时间是%02u点%02u分", 
                      (unsigned)now.hours, (unsigned)now.minutes);
              voice_send_reply(reply);
            } else {
              voice_send_reply("时间获取失败");
            }
          } else {
            voice_send_reply("没网络了喵，请连接网络");
          }
          break;
        }
        
        case VoiceCmd_ChangeImage:
          voice_send_reply("已经更换好了照片");
          display_manager_force_refresh(DisplayRefresh_Background);
          break;
          
        case VoiceCmd_Countdown5Min:
          if (countdown_start(5, NULL)) {
            // 倒计时启动成功，语音提示已在countdown_start中发送
          } else {
            voice_send_reply("倒计时启动失败，可能已经在运行中");
          }
          break;
          
        case VoiceCmd_Countdown10Min:
          if (countdown_start(10, NULL)) {
            // 倒计时启动成功，语音提示已在countdown_start中发送
          } else {
            voice_send_reply("倒计时启动失败，可能已经在运行中");
          }
          break;
          
        case VoiceCmd_CountdownStatus: {
          CountdownState state = countdown_get_state();
          if (state == CountdownState_Running) {
            uint32_t remaining_minutes = countdown_get_remaining_minutes();
            uint32_t remaining_seconds = countdown_get_remaining_seconds() % 60;
            char reply[64];
            snprintf(reply, sizeof(reply), "倒计时还剩%d分%d秒", 
                    (int)remaining_minutes, (int)remaining_seconds);
            voice_send_reply(reply);
          } else if (state == CountdownState_Finished) {
            voice_send_reply("倒计时已结束");
          } else {
            voice_send_reply("当前没有倒计时");
          }
          break;
        }
        
        case VoiceCmd_CountdownStop:
          countdown_stop();
          break;
          
        case VoiceCmd_UpdateWeather: {
          // 更新天气信息
          if (weather_is_connected()) {
            Esp8266Handle *g_esp = usart_get_esp8266_handle();
            if (weather_fetch_now(g_esp, 5000)) {
              char desc[32], temp[8];
              weather_get_last(desc, sizeof(desc), temp, sizeof(temp));
              char weather_str[32];
              snprintf(weather_str, sizeof(weather_str), "%s %sC", desc, temp);
              display_manager_set_weather(weather_str);
              display_manager_force_refresh(DisplayRefresh_Weather);
              voice_send_reply("天气信息已更新");
            } else {
              voice_send_reply("天气信息更新失败");
            }
          } else {
            voice_send_reply("没有网络连接，无法更新天气");
          }
          break;
        }
          
        case VoiceCmd_Refresh: {
          // 刷新天气信息、时间和背景
          if (weather_is_connected()) {
            Esp8266Handle *g_esp = usart_get_esp8266_handle();
            if (weather_fetch_now(g_esp, 5000)) {
              // 同步网络时间到DS1302
              weather_sync_time_from_network(g_esp);
              // 强制刷新所有显示内容
              display_manager_force_refresh(DisplayRefresh_Time | DisplayRefresh_Weather | DisplayRefresh_Background);
              voice_send_reply("天气信息已刷新");
            } else {
              voice_send_reply("天气信息刷新失败");
            }
          } else {
            // 无网络时只能刷新背景和时间
            display_manager_force_refresh(DisplayRefresh_Time | DisplayRefresh_Background);
            voice_send_reply("只能刷新背景了喵，因为没有网络了喵");
          }
          break;
        }
          
        default:
          break;
      }
    }
    osDelay(10);
  }
  /* USER CODE END task_read */
}

/* USER CODE BEGIN Header_task_wifi */
/**
* @brief Function implementing the TASK_WIFI thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_task_wifi */
void task_wifi(void *argument)
{
  /* USER CODE BEGIN task_wifi */
  Esp8266Handle *g_esp = usart_get_esp8266_handle();
  uint32_t last_weather_fetch = 0;
  uint32_t last_background_refresh = 0;
  
  for(;;)
  {
    uint32_t current_time = HAL_GetTick();
    
    // 每小时获取天气信息
    if (current_time - last_weather_fetch >= 3600000) { // 1小时 = 3600000毫秒
      if (weather_fetch_now(g_esp, 5000)) {
        char desc[32], temp[8];
        weather_get_last(desc, sizeof desc, temp, sizeof temp);
        char weather_str[32];
        snprintf(weather_str, sizeof weather_str, "%s %sC", desc, temp);
        display_manager_set_weather(weather_str);
        display_manager_force_refresh(DisplayRefresh_Weather);
      }
      last_weather_fetch = current_time;
    }
    
    // 每小时刷新背景
    if (current_time - last_background_refresh >= 3600000) { // 1小时 = 3600000毫秒
      display_manager_force_refresh(DisplayRefresh_Background);
      last_background_refresh = current_time;
    }
    
    osDelay(60000); // 每分钟检查一次
  }
  /* USER CODE END task_wifi */
}

/* Callback_ds1302_read function */
void Callback_ds1302_read(void *argument)
{
  /* USER CODE BEGIN Callback_ds1302_read */

  /* USER CODE END Callback_ds1302_read */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

