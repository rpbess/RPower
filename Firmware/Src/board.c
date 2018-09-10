#include "stm32f1xx_hal.h"
#include "board.h"

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim4;
extern UART_HandleTypeDef huart2;
extern unsigned long micronet_address[3];

void micronet_init()
{
	static unsigned long *chip_id = (unsigned long *)0x1FFFF7E8;
	micronet_address[0] = chip_id[0];
	micronet_address[1] = chip_id[1];
	micronet_address[2] = chip_id[2];
	micronet_find_delay = 0xFFFF;
	static unsigned char *id = (unsigned char *)0x1FFFF7E8;
	for(int ii=0;ii<12;ii++){
		int i;
    micronet_find_delay ^= id[ii];   
    for (i = 0; i < 8; ++i){
        if (micronet_find_delay & 1) micronet_find_delay = (micronet_find_delay >> 1) ^ 0xA001;
        else micronet_find_delay = (micronet_find_delay >> 1);
    }
	}
	micronet_find_delay &= 0xFFF;	
}

void micronet_send_byte(uint8_t b)
{
		HAL_UART_Transmit(&huart2, &b,1,1000);
}

void USART2_IRQHandler(void)
{
	micronet_append_byte(USART2->DR);
  HAL_UART_IRQHandler(&huart2);
}

uint16_t get_board_voltage()
{
	  HAL_ADC_Start(&hadc1);
		return ((3.3/4096*HAL_ADC_GetValue(&hadc1))*11.0 +0.8)*1000;
}

void beep(unsigned int width)
{
	  for(int i=0;i<width;i++){
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
			for(int ii=0;ii<5000;ii++);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
			for(int ii=0;ii<5000;ii++);
		}
}

void raspberry_power(char power_state)
{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, power_state>0?GPIO_PIN_SET:GPIO_PIN_RESET);
}

void board_led(char led_state)
{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, led_state>0?GPIO_PIN_SET:GPIO_PIN_RESET);
}

void fet_set_state(char fet, char state)
{
		if(fet==1)HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, state>0?GPIO_PIN_SET:GPIO_PIN_RESET);
	  if(fet==2)HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, state>0?GPIO_PIN_SET:GPIO_PIN_RESET);
}

void board_init(void)
{
  hts221_init();	
	lsm_init();
	micronet_init();
}
