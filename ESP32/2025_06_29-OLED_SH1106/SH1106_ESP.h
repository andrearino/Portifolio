/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file   SH1106_ESP.h
  * @brief  Header for SH1106_ESP applications
  ******************************************************************************
  * @modify    		  : 06/27/2025
  * @version		    : V_BRANCH#_STABLE#_IMPRUVE#
  * @designer       : André Arino
  * @contact        : andrearino@hotmail.com
  * @crieted data  	: 06/27/2025
  * @hardware       : ESP32 + OLED Drive SH1107
  * @dependences    : ESP-EDF 5.3.1 | I2C.h
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 André Arino 28869802850.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/*-----------------------------------------------------------------------------/
/ Additional user header to be used
/-----------------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */
/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef SH1106_ESP_H        // Defição
#define SH1106_ESP_H

#define _Data_Length_1 2
#define _Data_Length_2 4

#define _Display_ON = 0xAF
#define _Display_OFF = 0xAE
#define _Set_Entire_Display_ON = 0xA5
#define _Set_Reverse_Display = 0xA7

/* USER CODE END Includes */
#include "driver/i2c_master.h"

/*-----------------------------------------------------------------------------/
/ Function Configurations
/-----------------------------------------------------------------------------*/

/* USER CODE BEGIN Prototypes */
void OLED_Init (i2c_master_bus_handle_t bus_handle);
void OLED_Display_OFF ();
void OLED_Display_ON ();
void OLED_FULL_ON ();
void OLED_CLEAR ();
/* USER CODE END Prototypes */

#endif                      // Finaliza  */

