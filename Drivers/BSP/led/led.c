#include "led.h" 

/**
  * @brief  set the led status.
  * @param  index: index 1~8 corresponds to 1ed1~1ed8
  * @param  isOn: 1-On, 0-Off
  * @retval None
  */
void set_led_status(int index,uint8_t isOn)
{
	if(index)
	{		
			if(isOn)
			{
				HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
			}
			else
			{
				HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
			}
	}
}
