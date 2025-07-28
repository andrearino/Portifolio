/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file   SH1106_ESP.c
  * @brief  Library Standard ESP
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

 /* USER CODE BEGIN DEFINES */
 /*
  * Warning: ifdef warning used for IDE version changes (compatibility 
  * / migrating)
  * User code previously added there should be copied in the new user sections 
  * before the section contents can be deleted.
  */
 /* USER CODE BEGIN 0 */

 /* USER CODE END 0 */

 /* USER CODE END DEFINES */
 
 /* USER CODE BEGIN DECLARATIONS */
 
 /* Includes ------------------------------------------------------------------*/
  #include "SH1106_ESP.h" 
  #include "driver/i2c_master.h"

 /* Private typedef -----------------------------------------------------------*/
 /* typedef struct {
    variable_type variable_name_#;
    } Stuct_Name_#;
    Stuct_Name_# *Memory_Variable_Pointer = 
    (Stuct_Name_# *)malloc(n_Object * sizeof(Stuct_Name_#));

 /* Private define ------------------------------------------------------------*/
 
 /* Private variables ---------------------------------------------------------*/
 i2c_master_dev_handle_t BUS_Handle;
 ii2c_master_dev_handle_t DEV_Handle;
 uint16_t Slave_ADD = 0x7A; /* Slave ADD, A0 = 1, WR = 1 */
 uint16_t Slave_ADD_SA0 = 0xFD; /* A0 = 0 */
 uint16_t Slave_ADD_WR = 0xFE; /* WR = 0 */
 const uint8_t *write_buffer; /* buffer de Transmissão = 0 */
 uint8_t Control_Byte = 0xC0; /* Control Byte, CO = 1, DC = 1 */
 uint8_t Control_Byte_CO = 0xBF; /* CO = 0 1 operação*/
 uint8_t Control_Byte_DC = 0x7F; /* DC = 0 para comando */

 /* USER CODE END  DECLARATIONS */
 
 /* Private function prototypes -----------------------------------------------*/

 // #if _USE_DEFINITION == 1  /* Enable or Disable Function

 // #endif /* _USE_DEFINITION = 1 /* Return of Enable or Disable Function

 /* Private functions ---------------------------------------------------------*/
 
 /**
   * @brief  Brief of function
   * @param  pv_parameter_in: Description of private parameter
   * @retval RETURN_OUT: Description of output values
   */
 // #if _USE_DEFINITION_X == 1  /* Enable or Disable Function
 /* RETURN FUNCTION (
    pv_parameter_in           /* Description of input parameter 
 )
 {
   USER CODE BEGIN INIT
     CLASS_InitTypeDef CLASS_InitStruct_# = {gv_atribute};  /* Create new 
                                            object of CLASS_InitTypeDef 
                                            instance and pass global atributes
     CLASS_InitStruct_#.Function_X = pv_atribute;  /* pass private atribute to
                                            function of constructor new object  
     FUNCTION_Y(&CLASS_InitStruct_#); /* Execute FUNCTION_Y with constructor
                                         return with new object atributes 

     return pv_parameter_out;               /* RETURN_OUT = pv_parameter_out
     return OTHER_FUNCTION (pv_parameter);  /* RETURN_OUT = RETURN_OUT of 
                                               other function 
  }
  */

  /**
   * @Initialize Drive OLED
   * @param  pv_parameter_in: Application I2C Bus Handler Type for ESP
   * @retval RETURN_OUT: Void
  */
  void OLED_Init (i2c_master_bus_handle_t bus_handle)
                  /* Receive application I2C Handler Type */
  {
    BUS_Handle = bus_handle;  /* Local I2C Handler Type */
    i2c_device_config_t dev_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = Slave_ADD;
    .scl_speed_hz = 400000,
    };
    i2c_master_bus_add_device(BUS_Handle, &dev_cfg, &DEV_Handle);
  }

  /**
   * @OLED Display OFF
   * @param  pv_parameter_in: Void
   * @retval RETURN_OUT: Void
  */
 void OLED_Display_OFF () 
                  /*  */
  {
    DEV_Handle.device_address = (Slave_ADD & Slave_ADD_SA0 & Slave_ADD_WR);
    uint8_t data[2];
    *write_buffer = data;
    write_buffer[0] = (Control_Byte & Control_Byte_CO & Control_Byte_DC);
    write_buffer[1] = _Display_OFF;
    i2c_master_transmit(DEV_Handle, write_buffer, _Data_Length_1, -1);
  }

  /**
   * @OLED Display ON
   * @param  pv_parameter_in: Void
   * @retval RETURN_OUT: Void
  */
 void OLED_Display_ON () 
                  /*  */
  {
    DEV_Handle.device_address = (Slave_ADD & Slave_ADD_SA0 & Slave_ADD_WR);
    uint8_t data[2];
    *write_buffer = data;
    write_buffer[0] = (Control_Byte & Control_Byte_CO & Control_Byte_DC);
    write_buffer[1] = _Display_ON;
    i2c_master_transmit(DEV_Handle, write_buffer, _Data_Length_1, -1);
  }  

  /**
   * @OLED FULL ON
   * @param  pv_parameter_in: Void
   * @retval RETURN_OUT: Void
  */
 void OLED_FULL_ON ()
                  /*  */
  {
    DEV_Handle.device_address = (Slave_ADD & Slave_ADD_SA0 & Slave_ADD_WR);
    uint8_t data[2];
    *write_buffer = data;
    write_buffer[0] = (Control_Byte & Control_Byte_CO & Control_Byte_DC);
    write_buffer[1] = _Set_Entire_Display_ON;
    i2c_master_transmit(DEV_Handle, write_buffer, _Data_Length_1, -1);
  }    

  /**
   * @OLED CLEAR
   * @param  pv_parameter_in: Void
   * @retval RETURN_OUT: Void
  */
 void OLED_CLEAR ()
                  /*  */
  {
    DEV_Handle.device_address = (Slave_ADD & Slave_ADD_SA0 & Slave_ADD_WR);
    uint8_t data[2];
    *write_buffer = data;
    write_buffer[0] = (Control_Byte & Control_Byte_CO & Control_Byte_DC);
    write_buffer[1] = _Set_Entire_Display_ON;
    i2c_master_transmit(DEV_Handle, write_buffer, _Data_Length_1, -1);
    write_buffer[1] = _Set_Reverse_Display;
    i2c_master_transmit(DEV_Handle, write_buffer, _Data_Length_1, -1);
  } 

   /* USER CODE END INIT */
  // #endif /* _USE_DEFINITION_X = 1 /* Return of Enable or Disable Function
 