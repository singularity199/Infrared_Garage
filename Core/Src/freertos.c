/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "event_groups.h"
#include "i2c.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SLOT_1_OCCUPIED  (1 << 0)
#define SLOT_2_OCCUPIED  (1 << 1)
#define SLOT_3_OCCUPIED  (1 << 2)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
EventGroupHandle_t ParkingStatusGroup;
/* USER CODE END Variables */
/* Definitions for Infrared_Detect */
osThreadId_t Infrared_DetectHandle;
const osThreadAttr_t Infrared_Detect_attributes = {
  .name = "Infrared_Detect",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LED_Show_Task */
osThreadId_t LED_Show_TaskHandle;
const osThreadAttr_t LED_Show_Task_attributes = {
  .name = "LED_Show_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for LCD_Show_Task */
osThreadId_t LCD_Show_TaskHandle;
const osThreadAttr_t LCD_Show_Task_attributes = {
  .name = "LCD_Show_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void Start_Infrared_Detect_Task(void *argument);
void Start_LED_Show_Task(void *argument);
void Start_LCD_Show_Task(void *argument);

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

  /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Infrared_Detect */
  Infrared_DetectHandle = osThreadNew(Start_Infrared_Detect_Task, NULL, &Infrared_Detect_attributes);

  /* creation of LED_Show_Task */
  LED_Show_TaskHandle = osThreadNew(Start_LED_Show_Task, NULL, &LED_Show_Task_attributes);

  /* creation of LCD_Show_Task */
  LCD_Show_TaskHandle = osThreadNew(Start_LCD_Show_Task, NULL, &LCD_Show_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_Start_Infrared_Detect_Task */
/**
  * @brief  Function implementing the Infrared_Detect thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_Start_Infrared_Detect_Task */
void Start_Infrared_Detect_Task(void *argument)
{
  /* USER CODE BEGIN Start_Infrared_Detect_Task */
    uint8_t last_status = 0x00;
    /* Infinite loop */
    for (;;) {
        uint8_t current_status = 0x00;

        // 使用 LL 库读取 PA0, PA1, PA2 的状态
        // 假设红外模块检测到障碍物(有车)输出低电平(0)，无障碍物输出高电平(1)
        if (LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0) == 0) current_status |= SLOT_1_OCCUPIED;
        if (LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_1) == 0) current_status |= SLOT_2_OCCUPIED;
        if (LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_2) == 0) current_status |= SLOT_3_OCCUPIED;

        // 简易防抖：延时 20ms 后再次确认状态
        vTaskDelay(pdMS_TO_TICKS(20));

        uint8_t confirm_status = 0x00;
        if (LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0) == 0) confirm_status |= SLOT_1_OCCUPIED;
        if (LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_1) == 0) confirm_status |= SLOT_2_OCCUPIED;
        if (LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_2) == 0) confirm_status |= SLOT_3_OCCUPIED;

        // 如果两次读取状态一致，且与上次记录的状态不同，说明车位状态发生了实质性改变
        if ((current_status == confirm_status) && (current_status != last_status)) {
            // 更新 EventGroup
            // 先清除相关的 3 个 bit，再设置新的状态 bit
            xEventGroupClearBits(ParkingStatusGroup, SLOT_1_OCCUPIED | SLOT_2_OCCUPIED | SLOT_3_OCCUPIED);
            xEventGroupSetBits(ParkingStatusGroup, current_status);

            last_status = current_status; // 更新记录
        }

        // 轮询周期：50ms 扫描一次即可，既保证了实时性又释放了 CPU
        vTaskDelay(pdMS_TO_TICKS(30));


    }
  /* USER CODE END Start_Infrared_Detect_Task */
}

/* USER CODE BEGIN Header_Start_LED_Show_Task */
/**
* @brief Function implementing the LED_Show_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_LED_Show_Task */
void Start_LED_Show_Task(void *argument)
{
  /* USER CODE BEGIN Start_LED_Show_Task */
    EventBits_t status_bits;
    /* Infinite loop */
    for (;;) {
        // 阻塞等待事件标志组的任意一位发生变化
        // 参数说明：句柄,
        //         等待的位,
        //         退出时是否清除(否),
        //         是否等待所有位(否),
        //         阻塞时间(死等)
        status_bits = xEventGroupWaitBits(ParkingStatusGroup,
                                          SLOT_1_OCCUPIED | SLOT_2_OCCUPIED | SLOT_3_OCCUPIED,
                                          pdFALSE,
                                          pdFALSE,
                                          portMAX_DELAY);

        // 根据读取到的状态，使用 LL 库高速翻转 LED
        // 假设高电平点亮红灯(有车)，低电平熄灭
        if (status_bits & SLOT_1_OCCUPIED) LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_0);
        else LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_0);

        if (status_bits & SLOT_2_OCCUPIED) LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_1);
        else LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_1);

        if (status_bits & SLOT_3_OCCUPIED) LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_2);
        else LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_2);

    }
  /* USER CODE END Start_LED_Show_Task */
}

/* USER CODE BEGIN Header_Start_LCD_Show_Task */
/**
* @brief Function implementing the LCD_Show_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_LCD_Show_Task */
void Start_LCD_Show_Task(void *argument)
{
  /* USER CODE BEGIN Start_LCD_Show_Task */
    /* Infinite loop */
    for (;;) {
        // 读取事件标志组的状态，但不阻塞等待，每 100ms 刷新一次显示
        EventBits_t status_bits = xEventGroupGetBits(ParkingStatusGroup);
        //每次刷新前先清屏，防止残影
        OLED_Clear();
        OLED_ShowChar(1,16,'SLOT1:empty');
        osDelay(100);
    }
  /* USER CODE END Start_LCD_Show_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

