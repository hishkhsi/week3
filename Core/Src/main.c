/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <oled.h>
#include<string.h>
#include<math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define WAVE_SINE     0U          
#define WAVE_SAW      1U
#define WAVE_SQUARE   2U

#define N_SINE        100U        
#define N_SAW         50U         
#define N_SQUARE      150U        
#define PI_F          3.14159265f

#define RX_BUF_SIZE   32U

#define LINE_H        16U   
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint8_t rx_buff[RX_BUF_SIZE];
static uint8_t tx_frame[8];
static const uint8_t JF_TAIL[4]={0x00,0x00,0x80,0x7F};
static volatile uint8_t wave_idx=WAVE_SINE;
static volatile uint8_t sample_idx=0U;
static volatile uint8_t wave_dirty = 1U;
static uint8_t tx_buff[32];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void OLED_ShowWaveInfo(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(30U);
  OLED_Init();
  OLED_DisPlay_On();
  OLED_NewFrame();
  OLED_ShowWaveInfo();
  OLED_ShowFrame();
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1,rx_buff,sizeof(rx_buff));
  HAL_TIM_Base_Start_IT(&htim4);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // static uint8_t show=0;
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // for(uint8_t i=0;i<64;i++)
    // {
    //   OLED_NewFrame();
    //   OLED_PrintString(i, i, "hishkhsi", &font16x16, OLED_COLOR_NORMAL);
    //   OLED_ShowFrame();
    //   HAL_Delay(100);
    // }


  if(wave_dirty!=0U)
  {
    wave_dirty=0U;
    OLED_NewFrame();
    OLED_ShowWaveInfo();
    OLED_ShowFrame();
  }


  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  if(huart!=&huart1)return;


   for (uint16_t i = 0; i < Size; i++)
  {
    uint8_t ch = rx_buff[i];
    if (ch == '\r' || ch == '\n') continue;
    if (ch >= '1' && ch <= '3') 
    {
      wave_idx = (uint8_t)(ch - '1');
      sample_idx = 0U;
      wave_dirty = 1U;
    }
  }

  HAL_UARTEx_ReceiveToIdle_DMA(&huart1,rx_buff,sizeof(rx_buff));
  // if (Size>sizeof(tx_buff)) Size=sizeof(tx_buff);
  // memcpy(tx_buff,rx_buff,Size);
  // HAL_UART_Transmit_DMA(&huart1,tx_buff,Size);
  
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  float y;
  uint16_t n;

  if (htim!=&htim4) return; 

  switch (wave_idx)
  {
    case WAVE_SINE:
      n = N_SINE;
      y = 2.0f * sinf(2.0f * PI_F * (float)sample_idx / (float)n);
      break;
    case WAVE_SAW:
      n = N_SAW;
      y = -1.0f + 2.0f * (float)sample_idx / (float)n;
      break;
    case WAVE_SQUARE:
      n = N_SQUARE;
      y = (sample_idx < (n / 2U)) ? 0.5f : -0.5f;
      break;
    default:
      n = N_SINE;
      y = 0.0f;
      break;
  }
  sample_idx++;                            
  if (sample_idx >= n) sample_idx = 0U;    

  memcpy(tx_frame, &y, sizeof(y));                
  memcpy(tx_frame + 4, JF_TAIL, sizeof(JF_TAIL)); 

  if (huart1.gState == HAL_UART_STATE_READY)      
  {
    HAL_UART_Transmit_DMA(&huart1, tx_frame, sizeof(tx_frame));
  }
}

static void OLED_ShowWaveInfo(void)
{
  switch (wave_idx)
  {
    case WAVE_SINE:
      OLED_PrintString(0, LINE_H * 0U, "MODE: SINE",   &font16x16, OLED_COLOR_NORMAL);
      OLED_PrintString(0, LINE_H * 1U, "成家齐hishkhsi", &font16x16, OLED_COLOR_NORMAL);
      OLED_PrintString(0, LINE_H * 2U, "RoboMaster",  &font16x16, OLED_COLOR_NORMAL);
      OLED_PrintString(0, LINE_H * 3U, "第三周培训",  &font16x16, OLED_COLOR_NORMAL);
      break;
    case WAVE_SAW:
      OLED_PrintString(0, LINE_H * 0U, "MODE: SAWTOOTH",    &font16x16, OLED_COLOR_NORMAL);
      OLED_PrintString(0, LINE_H * 1U, "成家齐hishkhsi", &font16x16, OLED_COLOR_NORMAL);
      OLED_PrintString(0, LINE_H * 2U, "RoboMaster",  &font16x16, OLED_COLOR_NORMAL);
      OLED_PrintString(0, LINE_H * 3U, "第三周培训",  &font16x16, OLED_COLOR_NORMAL);
      break;
    case WAVE_SQUARE:
      OLED_PrintString(0, LINE_H * 0U, "MODE: SQUARE", &font16x16, OLED_COLOR_NORMAL);
      OLED_PrintString(0, LINE_H * 1U, "成家齐hishkhsi", &font16x16, OLED_COLOR_NORMAL);
      OLED_PrintString(0, LINE_H * 2U, "RoboMaster",  &font16x16, OLED_COLOR_NORMAL);
      OLED_PrintString(0, LINE_H * 3U, "第三周培训",  &font16x16, OLED_COLOR_NORMAL);
      break;
    default:
      break;
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
