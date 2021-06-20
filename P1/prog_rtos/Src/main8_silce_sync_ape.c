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
#include <stdlib.h>
#include <time.h>

//To activate time slicing, copy in freeRTOSConfig.h:
//#define configUSE_TIME_SLICING                  1

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
osThreadId LEDThread1Handle, LEDThread2Handle,
			LEDThread3Handle, LEDThread4Handle,
			LEDThread5Handle, LEDThread6Handle, 
			LEDThread7Handle, LEDThread8Handle,
			PrintThreadHandle, SieveThreadHandle;
			
osSemaphoreId semaphore;                      				    // Semaphore ID
osSemaphoreDef(semaphore);                      			    // Semaphore definition

uint32_t TIME_VECTOR [8] = {0,0,0,0,0,0,0,0};					//Time vector for the execution of 8 periodic tasks
uint8_t * num[8];												//Pointer vector used as a arguments of thread functions
uint32_t TIME;													//Reference time
uint8_t sync_value;												//Synchronization variable
uint32_t primes[101];											//Vector for calculation of prime numbers in compute intensive task

/* Private function prototypes -----------------------------------------------*/
static void LED_Thread(void const *argument);					
static void Print_result(void const *argument);
static void Sieve_Thread(void const *argument);					//Compute intensive task
void SystemClock_Config(void);
uint32_t max_time(uint32_t, uint32_t);
void ActiveWait(uint32_t x);									//Active wait for x ms
uint8_t RandomInjection(uint8_t percentage);					//Returns 1 with probability "percentage", otherwise 0

/* Prototype for semihosting -------------------------------------------------*/
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
	 
  printf("*************freeRTOS Task Timing**************\n\n");	 
  
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

  /* Initialize LEDs */
  BSP_LED_Init(LED3);
  BSP_LED_Init(LED4);
  BSP_LED_Init(LED5);
  BSP_LED_Init(LED6);
  BSP_LED_Init(LED7);
  BSP_LED_Init(LED8);
  BSP_LED_Init(LED9);
  BSP_LED_Init(LED10);

  //Thread Compute intensive
  osThreadDef(sieve_task, Sieve_Thread, osPriorityNormal , 0, configMINIMAL_STACK_SIZE);
  //Periodic Threads with same priority
  osThreadDef(LED10, LED_Thread, osPriorityNormal , 0, configMINIMAL_STACK_SIZE); 
  osThreadDef(LED9, LED_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
  osThreadDef(LED3, LED_Thread, osPriorityNormal , 0, configMINIMAL_STACK_SIZE);
  osThreadDef(LED4, LED_Thread, osPriorityNormal , 0, configMINIMAL_STACK_SIZE);
  osThreadDef(LED5, LED_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
  osThreadDef(LED6, LED_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
  osThreadDef(LED7, LED_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
  osThreadDef(LED8, LED_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
  //Print Thread
  osThreadDef(print_task, Print_result, osPriorityNormal, 0, configMINIMAL_STACK_SIZE); 

  //enumeration used as input argument of osThreadCreate functions
  uint8_t k = 0;
  for(k=0;k<8;k++){
	  num[k] = (uint8_t *) pvPortMalloc(sizeof(uint8_t));
	  *num[k] = k+1;
  }
  //Synchronization variable initialization
  sync_value = 0;

  //Thread Compute intensive
  SieveThreadHandle = osThreadCreate(osThread(sieve_task), NULL);  
  //Periodic Threads
  LEDThread1Handle = osThreadCreate(osThread(LED10), (void*) num[0]);
  LEDThread2Handle = osThreadCreate(osThread(LED9),  (void*) num[1]);
  LEDThread3Handle = osThreadCreate(osThread(LED3),  (void*) num[2]);
  LEDThread4Handle = osThreadCreate(osThread(LED4),  (void*) num[3]);
  LEDThread5Handle = osThreadCreate(osThread(LED5),  (void*) num[4]);
  LEDThread6Handle = osThreadCreate(osThread(LED6),  (void*) num[5]);
  LEDThread7Handle = osThreadCreate(osThread(LED7),  (void*) num[6]);
  LEDThread8Handle = osThreadCreate(osThread(LED8),  (void*) num[7]);
  //Print Thread
  PrintThreadHandle = osThreadCreate(osThread(print_task), NULL);
 
  //Reference time
  TIME = osKernelSysTick();
  
  //Creation of the semaphore structure
  semaphore = osSemaphoreCreate(osSemaphore(semaphore), 1);
 
  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */
  for (;;);

}

static void Sieve_Thread(void const *argument)
{
  //The thread is immediately suspended and is resumed by the print_task asynchronously
  osThreadSuspend(NULL);
  
 
  uint32_t number = 100;
  uint32_t i,j;
  
  //Compute intensive task
  for(;;){
	//populating array with naturals numbers
	for(i = 2; i<=number; i++)
		primes[i] = i;

	i = 2;
	while ((i*i) <= number)
	{
		if (primes[i] != 0)
		{
			for(j=2; j<number; j++)
			{
				if (primes[i]*j > number)
					break;
				else
					// Instead of deleteing , making elements 0
					primes[primes[i]*j]=0;
			}
		}
		i++;
	}
	
	ActiveWait(1000);
	
	osThreadSuspend(NULL);
  }
}

static void LED_Thread(void const *argument)
{
  uint8_t led = *((int*) argument);
  uint8_t i = 0;
  for(;;){
	  
	  for (i=0; i<10 ;i++)
	  {
		//A different led lights up depending on the thread
		switch(led){
			case 1: BSP_LED_Toggle(LED10); break;
			case 2: BSP_LED_Toggle(LED9); break;
			case 3: BSP_LED_Toggle(LED3); break;
			case 4: BSP_LED_Toggle(LED4); break;
			case 5: BSP_LED_Toggle(LED5); break;
			case 6: BSP_LED_Toggle(LED6); break;
			case 7: BSP_LED_Toggle(LED7); break;
			case 8: BSP_LED_Toggle(LED8); break;
			
			default: BSP_LED_Toggle(LED4); break;
		}
		ActiveWait(20);
	  }
	
	//The execution time is saved
	TIME_VECTOR[led-1] = osKernelSysTick() - TIME; 
	
	//Critical Section 
	osSemaphoreWait(semaphore, osWaitForever);  // Wait indefinitely for a free semaphore
    sync_value++;
    osSemaphoreRelease(semaphore);              // Return a token back to a semaphore.

	osThreadSuspend(NULL);
  }
}


static void Print_result(void const *argument){	
	uint8_t i = 0;
	uint8_t k = 0;
	uint8_t j = 0;
	uint8_t num_ex = 200;							//Number of times periodic threads run
	uint32_t AVG_VECTOR [8] = {0,0,0,0,0,0,0,0}; 	//Average Time Vector
	uint32_t WCET_VECTOR [8] = {0,0,0,0,0,0,0,0}; 	//WCET Time Vector
	uint8_t sync_flag = 0;
	
	osPriority Vett_priority[8]={
		osThreadGetPriority(LEDThread1Handle),
		osThreadGetPriority(LEDThread2Handle),
		osThreadGetPriority(LEDThread3Handle),
		osThreadGetPriority(LEDThread4Handle),
		osThreadGetPriority(LEDThread5Handle),
		osThreadGetPriority(LEDThread6Handle),
		osThreadGetPriority(LEDThread7Handle),
		osThreadGetPriority(LEDThread8Handle)
	};
	
	
	for(;;){
		osSemaphoreWait(semaphore, osWaitForever);  // Wait indefinitely for a free semaphore
		// OK, the interface is free now, use it.
		if(sync_value == 8){						//All the LED tasks have finished their execution
			sync_value = 0;							
			sync_flag = 1;
	    }
		osSemaphoreRelease(semaphore);              // Return a token back to a semaphore.
		
		
		if(sync_flag == 1){
			sync_flag=0;
			//Calculation of averages and WCETs
			for(i=0;i<8;i++){
				AVG_VECTOR[i] += TIME_VECTOR[i];
				WCET_VECTOR[i] = max_time(WCET_VECTOR[i],TIME_VECTOR[i]);
			}
			
			k++;
			//After num_ex execution
			if(k == num_ex){
				//Memory is freed
				for(j=0;j<8;j++){
					vPortFree(num[j]);
				}	
				
				//Print of results
				for(i=0;i<8;i++){
					printf("Thread: %d, priority:%d average time: %ld WCET: %ld",i+1,Vett_priority[i], AVG_VECTOR[i]/num_ex,WCET_VECTOR[i]);
					printf("\n");
				}
				
				//The thread is terminated before to resume the others 
				osThreadSuspend(NULL);
			}
					
			//Reference time is updated
			TIME = osKernelSysTick();
			
			//The compute intensive task is resumed with 50% probability
			if(RandomInjection(50)==1){
				osThreadResume(SieveThreadHandle);
			}
			osThreadResume(LEDThread1Handle);
			osThreadResume(LEDThread2Handle);
			osThreadResume(LEDThread3Handle);
			osThreadResume(LEDThread4Handle);
			osThreadResume(LEDThread5Handle);
			osThreadResume(LEDThread6Handle);
			osThreadResume(LEDThread7Handle);
			osThreadResume(LEDThread8Handle);
			
		}
		else{
			osThreadYield();
		}
		
	}
}

uint8_t RandomInjection(uint8_t percentage){
    srand(time(NULL));
	if(rand()%100+1 < percentage ){
		return 1;
	}
    else{
		return 0;
	}
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

uint32_t max_time(uint32_t a, uint32_t b){
	if(a < b){
		return b;
	}
	return a;
}

void ActiveWait(uint32_t x){
	int count = osKernelSysTick()+x;
	while (count > osKernelSysTick());
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
