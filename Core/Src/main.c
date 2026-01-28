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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ili9341.h"
#include "img.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi5;
DMA_HandleTypeDef hdma_spi5_tx;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint8_t game_state = 0;
  // 0 = welcome screen
  // 1 = playing
  // 2 = end screen
uint8_t winner = 0;
uint16_t ropeX = 120 - 5;  // Center of screen (240/2) minus half rope width
uint16_t ropeWidth = 10;
uint16_t ropeHeight = 320;
uint16_t markerWidth = 20;  // Marker wider than rope
uint16_t markerX = 120 - 10; // Center marker (240/2 - markerWidth/2)
int rope_pos = 160;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI5_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Load Cell 1 - PB8/PB9
#define DT_PIN_1 GPIO_PIN_8
#define DT_PORT_1 GPIOB
#define SCK_PIN_1 GPIO_PIN_9
#define SCK_PORT_1 GPIOB

// Load Cell 2 - PA6/PA7
#define DT_PIN_2 GPIO_PIN_6
#define DT_PORT_2 GPIOA
#define SCK_PIN_2 GPIO_PIN_7
#define SCK_PORT_2 GPIOA

// Load Cell 1 calibration
int32_t tare1 = -145000;           // HX711 raw at weight 0
float knownOriginal1 = 200000;     // Known weight in mg
float knownHX711_1 = -167100;       // HX711 value when known weight applied

// Load Cell 2 calibration
int32_t tare2 = -639000;
float knownOriginal2 = 200000;
float knownHX711_2 = -661000;

void microDelay(uint16_t delay)
{
  __HAL_TIM_SET_COUNTER(&htim2, 0);
  while (__HAL_TIM_GET_COUNTER(&htim2) < delay);
}

int32_t getHX711(GPIO_TypeDef* dtPort, uint16_t dtPin, GPIO_TypeDef* sckPort, uint16_t sckPin)
{
  uint32_t data = 0;
  uint32_t startTime = HAL_GetTick();
  while(HAL_GPIO_ReadPin(dtPort, dtPin) == GPIO_PIN_SET)
  {
    if(HAL_GetTick() - startTime > 50)
      return 0;
  }
  for(int8_t len=0; len<24 ; len++)
  {
    HAL_GPIO_WritePin(sckPort, sckPin, GPIO_PIN_SET);
    microDelay(1);
    data = data << 1;
    HAL_GPIO_WritePin(sckPort, sckPin, GPIO_PIN_RESET);
    microDelay(1);
    if(HAL_GPIO_ReadPin(dtPort, dtPin) == GPIO_PIN_SET)
      data ++;
  }
  if(data & 0x800000)
    data |= 0xFF000000;
  HAL_GPIO_WritePin(sckPort, sckPin, GPIO_PIN_SET);
  microDelay(1);
  HAL_GPIO_WritePin(sckPort, sckPin, GPIO_PIN_RESET);
  microDelay(1);
  return data;
}


int weigh(GPIO_TypeDef* dtPort, uint16_t dtPin, GPIO_TypeDef* sckPort, uint16_t sckPin,
          int32_t tare, float knownOriginal, float knownHX711)
{
  int32_t  total = 0;
  int32_t  samples = 10; // Faster response
  int32_t  validSamples = 0;
  int milligram;
  float coefficient;
  for(uint16_t i=0 ; i<samples ; i++)
  {
      int32_t reading = getHX711(dtPort, dtPin, sckPort, sckPin);
      if(reading != 0) // Skip timeout reads
      {
        total += reading;
        validSamples++;
      }
  }
  if(validSamples == 0)
    return 0;
  int32_t average = (int32_t)(total / validSamples);
  coefficient = abs(knownOriginal / abs(knownHX711 - tare));
  milligram = (int)(abs(average - tare) * coefficient);
  return milligram;
}


void draw_welcome_screen() {
  ILI9341_FillScreen(ILI9341_BLACK);
  ILI9341_DrawImage(0, 0, &kiet_start_image);
  ILI9341_DrawImage(160, 220, &minh_start_image);
  ILI9341_DrawText(38, 130, "KEO CO CUC CANG", &Font_11x18, ILI9341_WHITE, ILI9341_RED);
  ILI9341_DrawText(25, 178, "Press PA0 to start", &Font_11x18, ILI9341_WHITE, ILI9341_RED);
  ILI9341_FillRect(0, 100, 240, 2, ILI9341_YELLOW);
  ILI9341_FillRect(0, 220, 240, 2, ILI9341_YELLOW);
}

void draw_end_screen(){
	ILI9341_FillScreen(ILI9341_BLACK);
	if (winner == 1){ // MInh, draw kiet_lose
		ILI9341_DrawText(40, 0, "KIET LOSES", &Font_16x26, ILI9341_WHITE, ILI9341_RED);
    ILI9341_DrawImage(0, 40, &kiet_lose_image);
	}
	else{
		ILI9341_DrawText(40, 0, "MINH LOSES", &Font_16x26, ILI9341_WHITE, ILI9341_RED);
		ILI9341_DrawImage(0, 40, &minh_lose_image);
	}
}
void draw_rope(int16_t pos);

void draw_game_screen(){
	ILI9341_FillScreen(ILI9341_BLACK);
	ILI9341_DrawImage(0, 0, &kiet_start_image);
	ILI9341_DrawImage(160, 220, &minh_start_image);
	ILI9341_FillRect(0, 100, 240, 2, ILI9341_YELLOW);
	ILI9341_FillRect(0, 220, 240, 2, ILI9341_YELLOW);
	draw_rope(160);
}

void draw_rope(int16_t pos) {
  static int16_t lastPos = -1;
  
  if(lastPos == -1) {
    // First time - draw full rope
    ILI9341_FillRect(ropeX, 0, ropeWidth, pos - 5, ILI9341_RED);
    ILI9341_FillRect(markerX, pos - 5, markerWidth, 10, ILI9341_GREEN);
    ILI9341_FillRect(ropeX, pos + 5, ropeWidth, ropeHeight - (pos + 5), ILI9341_BLUE);
    lastPos = pos;
    return;
  }
  
  if(pos == lastPos) return;
  
  if(pos > lastPos) {
    // Moving down by 1 pixel
    ILI9341_FillRect(ropeX, lastPos - 5, ropeWidth, 1, ILI9341_RED);      // Add 1px red
    ILI9341_FillRect(ropeX, lastPos + 5, ropeWidth, 1, ILI9341_BLACK);    // Erase 1px blue

    // Marker top edge: center 10px rope color, sides 5px black
    ILI9341_FillRect(markerX, lastPos - 5, 5, 1, ILI9341_BLACK);          // Left side black
    ILI9341_FillRect(ropeX, lastPos - 5, ropeWidth, 1, ILI9341_RED);      // Center rope red
    ILI9341_FillRect(markerX + 15, lastPos - 5, 5, 1, ILI9341_BLACK);     // Right side black
    // Marker bottom edge: full green
    ILI9341_FillRect(markerX, pos + 4, markerWidth, 1, ILI9341_GREEN);

  } else {
    // Moving up by 1 pixel
    ILI9341_FillRect(ropeX, pos - 5, ropeWidth, 1, ILI9341_BLACK);        // Erase 1px red
    ILI9341_FillRect(ropeX, pos + 5, ropeWidth, 1, ILI9341_BLUE);         // Add 1px blue
    // Marker bottom edge: center 10px rope color, sides 5px black
    ILI9341_FillRect(markerX, lastPos + 4, 5, 1, ILI9341_BLACK);          // Left side black
    ILI9341_FillRect(ropeX, lastPos + 4, ropeWidth, 1, ILI9341_BLUE);     // Center rope blue
    ILI9341_FillRect(markerX + 15, lastPos + 4, 5, 1, ILI9341_BLACK);     // Right side black
    // Marker top edge: full green
    ILI9341_FillRect(markerX, pos - 5, markerWidth, 1, ILI9341_GREEN);
  }
  
  lastPos = pos;
}

void main_game_logic(int weight_MInh, int weight_Kiet){

  int rope_change = abs(weight_MInh - weight_Kiet) / 1000 * 2;
  if (rope_change == 0)
	  return;
  int rope_dir = 0;
  if (weight_MInh > weight_Kiet)
	  rope_dir = 1;
  else
	  rope_dir = -1;

  for(int i = 1; i <= rope_change; i++){
	  rope_pos += rope_dir;
	  if(rope_pos == 95 || rope_pos == 225)
		  break;
	  draw_rope(rope_pos);
  }
  if (rope_pos == 95) {
	  game_state = 2;
	  winner = 2;
	  draw_end_screen();
  }
  else if (rope_pos == 225) {
	  game_state = 2;
	  winner = 1;
	  draw_end_screen();
  }
}
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
  MX_SPI5_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim2);

  // Initialize HX711 modules
  HAL_GPIO_WritePin(SCK_PORT_1, SCK_PIN_1, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SCK_PORT_2, SCK_PIN_2, GPIO_PIN_RESET);

  ILI9341_Init();
  ILI9341_SetRotation(0);
  
  // Show welcome screen
  draw_welcome_screen();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint8_t debug = 0;
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  int weight_MInh, weight_Kiet;
	  weight_MInh = weigh(DT_PORT_1, DT_PIN_1, SCK_PORT_1, SCK_PIN_1, tare1, knownOriginal1, knownHX711_1) / 1000;
	  weight_Kiet = weigh(DT_PORT_2, DT_PIN_2, SCK_PORT_2, SCK_PIN_2, tare2, knownOriginal2, knownHX711_2) / 1000;
	  if (debug){
		  char buf[200];
		  sprintf(buf, "MInh Weight:%d gram | Kiet Weight:%d gram\r\n", weight_MInh, weight_Kiet);
		  HAL_UART_Transmit(&huart1, (const uint8_t*)buf, strlen(buf), 100);
	  }
	  if (game_state == 0){
		  HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_13);
		  HAL_Delay(500);
	  }
	  else if (game_state == 2){
		  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
		  HAL_Delay(500);
	  }
	  else
		  main_game_logic(weight_MInh, weight_Kiet);
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI5_Init(void)
{

  /* USER CODE BEGIN SPI5_Init 0 */

  /* USER CODE END SPI5_Init 0 */

  /* USER CODE BEGIN SPI5_Init 1 */

  /* USER CODE END SPI5_Init 1 */
  /* SPI5 parameter configuration*/
  hspi5.Instance = SPI5;
  hspi5.Init.Mode = SPI_MODE_MASTER;
  hspi5.Init.Direction = SPI_DIRECTION_2LINES;
  hspi5.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi5.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi5.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi5.Init.NSS = SPI_NSS_SOFT;
  hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi5.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi5.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI5_Init 2 */

  /* USER CODE END SPI5_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PD12 PD13 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PG13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : PB7 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */


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
