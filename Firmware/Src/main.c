#include "main.h"
#include "stm32f1xx_hal.h"
#include "board.h"

ADC_HandleTypeDef hadc1;
I2C_HandleTypeDef hi2c2;
TIM_HandleTypeDef htim4;
UART_HandleTypeDef huart2;

void SystemClock_Config(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
 
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C2_Init(void);
static void MX_USART2_UART_Init(void);

   
void micronet_handler()
{
	if(micronet_request.Cmd == 1){
			float humidity = hts221_getHumidity();
			float temperature = hts221_getTemperature();		 
		  micronet_response.Data[0] = (unsigned char)(humidity);
		  micronet_response.Data[1] = (unsigned char)(temperature+127);
		  micronet_response.DataLength = 2;
		  micronet_send_response(micronet_response);
	}
	if(micronet_request.Cmd == 2){
		  LsmDataTypeDef data = lsm_read_acc(); 
		  data.X +=16000; data.Y +=16000; data.Z +=16000;
		  micronet_response.Data[0] = ((uint16_t)data.X) >> 8;
		  micronet_response.Data[1] = ((uint16_t)data.X) &  0xFF;
		  micronet_response.Data[2] = ((uint16_t)data.Y) >> 8;
		  micronet_response.Data[3] = ((uint16_t)data.Y) &  0xFF;
		  micronet_response.Data[4] = ((uint16_t)data.Z) >> 8;
		  micronet_response.Data[5] = ((uint16_t)data.Z) &  0xFF;
		
		  data = lsm_read_mag(); data.X +=16000; data.Y +=16000; data.Z +=16000;

		  micronet_response.Data[6] = ((uint16_t)data.X) >> 8;
		  micronet_response.Data[7] = ((uint16_t)data.X) &  0xFF;
		  micronet_response.Data[8] = ((uint16_t)data.Y) >> 8;
		  micronet_response.Data[9] = ((uint16_t)data.Y) &  0xFF;
		  micronet_response.Data[10] = ((uint16_t)data.Z) >> 8;
		  micronet_response.Data[11] = ((uint16_t)data.Z) &  0xFF;
		  micronet_response.DataLength = 12;
		  micronet_send_response(micronet_response);
	}
	if(micronet_request.Cmd == 3){
			beep(micronet_request.Data[0]*256+micronet_request.Data[1]);
		  micronet_response.DataLength = 0;
		  micronet_send_response(micronet_response);
	}
	if(micronet_request.Cmd == 4){
			board_led(micronet_request.Data[0]);
		  micronet_response.DataLength = 0;
		  micronet_send_response(micronet_response);
	}
	if(micronet_request.Cmd == 5){
			uint16_t voltage = get_board_voltage();
			micronet_response.Data[0] = voltage >> 8;
		  micronet_response.Data[1] = voltage & 0xFF;
		  micronet_response.DataLength = 2;
		  micronet_send_response(micronet_response);
	}
	if(micronet_request.Cmd == 6){
			fet_set_state(micronet_request.Data[0], micronet_request.Data[1]);
		  micronet_response.DataLength = 0;
		  micronet_send_response(micronet_response);
	}
	if(micronet_request.Cmd == 7){
			raspberry_power(0);
			beep(150);
		  raspberry_power(1);
			fet_set_state(micronet_request.Data[0], micronet_request.Data[1]);
		  micronet_response.DataLength = 0;
		  micronet_send_response(micronet_response);
	}
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
	HAL_Delay(3000);
	MX_I2C2_Init();
	MX_ADC1_Init();
  MX_GPIO_Init();
  MX_USART2_UART_Init();
	micronet_set_descriptor("RPower 2.0");
	board_init();
	raspberry_power(1);
  while (1)
  {	
  }
}




void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV2;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){_Error_Handler(__FILE__, __LINE__);}
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK){_Error_Handler(__FILE__, __LINE__);}
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK){_Error_Handler(__FILE__, __LINE__);}
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}


static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK){_Error_Handler(__FILE__, __LINE__);}
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK){_Error_Handler(__FILE__, __LINE__);}
}

static void MX_I2C2_Init(void)
{
	__HAL_RCC_I2C2_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
	
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK){_Error_Handler(__FILE__, __LINE__);}
}



static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }else{
			__HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
	}
}


static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7 |GPIO_PIN_15, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|LED_Pin|GPIO_PIN_3 |GPIO_PIN_4|GPIO_PIN_5, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7 |GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|LED_Pin|GPIO_PIN_3 |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void _Error_Handler(char *file, int line)
{
  while(1)
  {
  }
}
