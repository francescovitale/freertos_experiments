/**
  ******************************************************************************
  * @file    FreeRTOS/FreeRTOS_ThreadCreation/Src/main.c
  * @author  MCD Application Team
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>

//Time Slicing activated 
#define configUSE_TIME_SLICING                  1

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
osThreadId Thread1Handle, Thread2Handle,
			Thread3Handle, PrintThreadHandle;

uint32_t START_VECTOR [3] = {0,0,0};				   //Start time vector 
uint32_t END_VECTOR [3] = {0,0,0};					   //End time vector 
uint32_t CRITICAL_SECTION_ELAPSED_TIME [3] = {0,0,0};  //Elapsed time in critical section  

/* Shared Variable -----------------------------------------------------------*/
osMutexDef(MutexIsr);                                  // Mutex name definition
osMutexId mutex_id;									   // Mutex ID definition

/* Private function prototypes -----------------------------------------------*/
static void Thread1(void const *argument);
static void Thread2(void const *argument);
static void Thread3(void const *argument);
static void Print_result(void const *argument);
void SystemClock_Config(void);
void ActiveWait(uint32_t x);						   //Active wait for x ms

/* Mutex function prototypes -----------------------------------------------*/
void CreateMutex(void);
void WaitMutex(void);
void ReleaseMutex(void);

/* prototype for semihosting -------------------------------------------------*/
extern void initialise_monitor_handles(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /*---------------------------Initialization---------------------------------*/
  
  //Inizialization for semihosting
  initialise_monitor_handles();
	 
  printf("*************freeRTOS Priority Inversion**************\n\n");	 
  
  /* STM32F3xx HAL library initialization:
       - Configure the Flash prefetch
       - Systick timer is configured by default as source of time base, but user 
         can eventually implement his proper time base source (a general purpose 
         timer for example or other time source), keeping in mind that Time base 
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
  */  
  HAL_Init();

  /* Configure the System clock to 72 MHz */
  SystemClock_Config();

  /* Mutex creation */
  CreateMutex();

  /* Initialize LEDs */
  BSP_LED_Init(LED3);
  BSP_LED_Init(LED4);
  BSP_LED_Init(LED5);

  //Thread Definitions 
  osThreadDef(Name_thread1, Thread1, osPriorityHigh, 0, configMINIMAL_STACK_SIZE); 
  osThreadDef(Name_thread2, Thread2, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
  osThreadDef(Name_thread3, Thread3, osPriorityBelowNormal, 0, configMINIMAL_STACK_SIZE);
  //Print Thread
  osThreadDef(print_task, Print_result, osPriorityLow, 0, configMINIMAL_STACK_SIZE); 

  //Thread Creation
  Thread1Handle = osThreadCreate(osThread(Name_thread1), NULL);
  Thread2Handle = osThreadCreate(osThread(Name_thread2), NULL);
  Thread3Handle = osThreadCreate(osThread(Name_thread3), NULL);
  //Print Thread
  PrintThreadHandle = osThreadCreate(osThread(print_task), NULL);
 
 
  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */
  for (;;);

}

/**
  * @brief  Toggle LED3 thread 1
  * @param  thread not used
  * @retval None
  */

static void Thread1(void const *argument)
{
  //The thread is immediately suspended and is resumed by thread 3
  osThreadSuspend(NULL);
  
  //Start Time is saved
  START_VECTOR[0] = osKernelSysTick(); 
  int count = 0;
  
  for(;;){
	//Wait and turn on Led 3
    ActiveWait(100);
	BSP_LED_Toggle(LED3); 
	
	
	
	/* CRITICAL SECTION */
	WaitMutex();																				//Wait for the resource
	CRITICAL_SECTION_ELAPSED_TIME[0] = osKernelSysTick();										//Time start in critical section
	
	ActiveWait(100);
	
	CRITICAL_SECTION_ELAPSED_TIME[0] = osKernelSysTick() - CRITICAL_SECTION_ELAPSED_TIME[0];	//Time end in critical section
	ReleaseMutex();																				//Release the resource
	/* CRITICAL SECTION */
	
	
	
	//Turn off led 3 and wait	
	BSP_LED_Toggle(LED3); 
    ActiveWait(100);
	
	//End Time is saved
    END_VECTOR[0] = osKernelSysTick(); 
	
	osThreadSuspend(NULL);
  }
}

static void Thread2(void const *argument)
{
  //The thread is immediately suspended and is resumed by thread 3
  osThreadSuspend(NULL);
  
  //Start Time is saved  
  START_VECTOR[1] = osKernelSysTick(); 
  int i = 0;
  int count = 0;
  
  for(;;){
	  
	//Blink Led 4 for 5 times
	for(i=0; i <5; i++){
		BSP_LED_On(LED4);
		ActiveWait(50);
		BSP_LED_Off(LED4);
		ActiveWait(50);
	}
	
	//End Time is saved
    END_VECTOR[1] = osKernelSysTick(); 
	
	osThreadSuspend(NULL);
  }
}

static void Thread3(void const *argument)
{
  //Start Time is saved  
  START_VECTOR[2] = osKernelSysTick(); 
  int count = 0;
  
  for(;;){
	//Turn on led 5, wait and then turn off led 5
	BSP_LED_Toggle(LED5); 
    ActiveWait(200);
	BSP_LED_Toggle(LED5); 
	
	
	/* CRITICAL SECTION */
	WaitMutex();																				//Wait for the resource
	CRITICAL_SECTION_ELAPSED_TIME[2] = osKernelSysTick();										//Time start in critical section
	
	ActiveWait(100);
	
	//Task 1 is resumed
	osThreadResume(Thread1Handle);
	
	ActiveWait(100);
	
	//Task 2 is resumed
	osThreadResume(Thread2Handle);
		
	ActiveWait(100);
	
	CRITICAL_SECTION_ELAPSED_TIME[2] = osKernelSysTick() - CRITICAL_SECTION_ELAPSED_TIME[2];    //Time end in critical section
	ReleaseMutex();																				//Release the resource
	/* CRITICAL SECTION */
	
	
	//Turn off led 3 and wait
	BSP_LED_Toggle(LED3); 
    ActiveWait(100);
	
	//End Time is saved
    END_VECTOR[2] = osKernelSysTick(); 
	
	osThreadSuspend(NULL);
  }
}

static void Print_result(void const *argument){	
	uint8_t i = 0;

	osPriority Vett_priority[3]={
		osThreadGetPriority(Thread1Handle),
		osThreadGetPriority(Thread2Handle),
		osThreadGetPriority(Thread3Handle)
	};
	
	for(;;){
		
		//Print Result
		for(i=0;i<3;i++){
			printf("Thread: %d, priority:%d start_time: %ld end_time: %ld elapsed_time_in_critical_section: %ld",
							i+1,Vett_priority[i], START_VECTOR[i], END_VECTOR[i], CRITICAL_SECTION_ELAPSED_TIME[i]);
			printf("\n");
		}
		
		osThreadSuspend(NULL);
		
	}
}

void CreateMutex (void){   
  mutex_id = osMutexCreate(osMutex(MutexIsr));
  if (mutex_id != NULL) {
    printf("Mutex Created!\n");// Mutex object created
  }   
}

void ReleaseMutex (void){
  osStatus status;
  
  if (mutex_id != NULL)  {
    status = osMutexRelease(mutex_id);
    if (status != osOK)  {
      printf("ERRORE nella Release");// handle failure code
    }
  }
}

void WaitMutex(void){
osStatus status;
  if (mutex_id != NULL)  {
    status  = osMutexWait(mutex_id, osWaitForever);
    if (status != osOK)  {
      printf("ERRORE nella WaitMutex");// handle failure code
    }
  }
}

void ActiveWait(uint32_t x){
	int count = osKernelSysTick()+x;
	while (count > osKernelSysTick());
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 72000000
  *            HCLK(Hz)                       = 72000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 8000000
  *            HSE PREDIV                     = 1
  *            PLLMUL                         = RCC_PLL_MUL9 (9)
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
