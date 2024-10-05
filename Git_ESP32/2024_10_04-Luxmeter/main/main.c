/* OLED Test */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 AARINO
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
/**
 * Standard Lib C;
 */
#include <stdio.h>
#include <string.h>
#include "math.h"

/**
 * FreeRTOs;
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

/**
 * Lib Display SSD1306 Oled;
 * Atenção: Inclua este arquivo apenas 1x;
 * Caso contrário, inclua "lib_ss1306.h"
 */

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "lib_ssd1306_fonts.h"
/**
 * IOs;
 */
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/adc.h"

/**
 * Log Errors;
 */
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_intr_alloc.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
// Select ADC control type
#define ADC_CHANNEL    ADC2_CHANNEL_3

// Select LCD control type
#define CONFIG_LCD_CONTROLLER_SSD1306 1

// For activate Debug 1 and 0 for desactivate
#define DEBUG                         1

// Include library acording with LCD type
#include "esp_lcd_panel_vendor.h"

// The pixel number in horizontal and vertical
#define LCD_H_RES                 128
#define LCD_V_RES                 64

/* Private define ------------------------------------------------------------*/
/* I2C Definitions; */
#define I2C_BUS_PORT              I2C_NUM_0
#define I2C_PIN_NUM_SDA           4
#define I2C_PIN_NUM_SCL           5
#define I2C_PIN_NUM_RST           -1
#define I2C_MODE                  I2C_MODE_MASTER // Use I2C_MODE_SLAVE for slave
#define I2C_MASTER_TX_BUF_DISABLE 0   /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0   /*!< I2C master doesn't need buffer */

/* OLED Definitions; */
#define OLED_PIXEL_CLOCK_HZ       (400 * 1000) // 400kHz
#define OLED_I2C_HW_ADDR          0x3C

/* Private macro -------------------------------------------------------------*/

/* Private Handles -----------------------------------------------------------*/
/* ADC resolution */
static const adc_bits_width_t width = ADC_WIDTH_BIT_11;
/* I2C config */
i2c_mode_t i2c_mode = I2C_MODE; 
i2c_config_t i2c_config;
/* SSD1306 OLED config */
esp_lcd_panel_io_handle_t io_handle = NULL;
esp_lcd_panel_io_i2c_config_t io_config;
esp_lcd_panel_dev_config_t panel_config;
esp_lcd_panel_handle_t panel_handle = NULL;


/* Private variables ---------------------------------------------------------*/
/* Debug contant */
static const char *TAG = "example";

/* Private open external image */
//extern void example_lvgl_demo_ui(lv_disp_t *disp);

/* Private function prototypes -----------------------------------------------*/
void app_main(void);
static void task_adc (void * pvParameter);
static void i2c_marter_init_function (void);
static void oled_init_function (void);
void clear_display(esp_lcd_panel_handle_t panel_handle, int width, int height);
void draw_text(esp_lcd_panel_handle_t panel_handle, const char *text, int x, int y); 
_Bool color_trans_done_callback(esp_lcd_panel_io_handle_t handle, esp_lcd_panel_io_event_data_t *event_data, void *user_ctx);

/* Private user code ---------------------------------------------------------*/

/**
 * @brief ADC2_CH3 initialization and Read ADC and print Terminal Task
 */
static void task_adc (void * pvParameter)
{
  /**
  * Config ADC;
  * Config I2C Struct;
  */
  // imprime na saída do console;
  if( DEBUG )
    ESP_LOGI( TAG, "ADC inicializado...");

  int read_raw;
  float vldr;
  float rldr;
  float lum;
  char  str[8];
  int h = LCD_H_RES; 
  int v = LCD_V_RES;
  esp_err_t r;

  gpio_num_t adc_gpio_num;

  r = adc2_pad_get_io_num( ADC_CHANNEL, &adc_gpio_num );
  assert( r == ESP_OK );

  printf("ADC2 canal %d e GPIO %d.\n", ADC_CHANNEL, adc_gpio_num);

  //be sure to do the init before using adc.
  printf("Inicializando ADC2_CH3...\n");
  adc2_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_12);

  vTaskDelay(2000/portTICK_PERIOD_MS);

  printf("Inicia a conversão.\n");

  esp_lcd_panel_disp_on_off(panel_handle,0);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  esp_lcd_panel_disp_on_off(panel_handle,1);

  clear_display(panel_handle, h, v);
  draw_text(panel_handle, "  Andre Arino   ", 0, 0);
  draw_text(panel_handle, "  04/10/2024    ", 0, 10);
  draw_text(panel_handle, "  Volts:        ", 0, 20);
  draw_text(panel_handle, "  Ohms:         ", 0, 30);
  draw_text(panel_handle, "  Lux:          ", 0, 40);
  /**
  * Leitura ADC e Escrita Serial;
  */
  while(1) 
  {
    r = adc2_get_raw( ADC_CHANNEL, width, &read_raw);
    if ( r == ESP_OK ) 
    {
      vldr = ((3.3/2047)*read_raw);
      rldr = (vldr/((3.3 - vldr)/10000));
      lum = pow(10,6.5-1.25*log10(rldr));
      printf("Tensão ADC: %.2f Volts | %.2f ohms | %.2f lux\n", vldr, rldr, lum );
      sprintf(str, "%.2f", vldr);
      draw_text(panel_handle, str, 60, 20);
      sprintf(str, "%.0f", rldr);
      draw_text(panel_handle, str, 60, 30);
      sprintf(str, "%.2f", lum);
      draw_text(panel_handle, str, 60, 40);
    } 
    else if ( r == ESP_ERR_INVALID_STATE ) 
    {
      printf("%s: ADC não foi inicializado ainda.\n", esp_err_to_name(r));
    } 
    else if ( r == ESP_ERR_TIMEOUT ) 
    {
      printf("%s: ADC está sendo usado pelo WiFi.\n", esp_err_to_name(r));
    } 
    else 
    {
      printf("%s\n", esp_err_to_name(r));
    }
    vTaskDelay(2000/portTICK_PERIOD_MS);
  }
}

/**
 * @brief i2c master initialization
 */
static void i2c_marter_init_function (void)
{
/**
  * Config I2C;
  * Config I2C Struct;
  */
  ESP_LOGI(TAG, "Initialize I2C bus");
  i2c_config_t i2c_config = 
  {
    .mode = i2c_mode, /* Master */
    .sda_io_num = I2C_PIN_NUM_SDA, /*!< GPIO number for I2C sda signal */
    .scl_io_num = I2C_PIN_NUM_SCL, /*!< GPIO number for I2C scl signal */
    .sda_pullup_en = 1, /*!< Internal GPIO pull mode for I2C sda signal*/
    .scl_pullup_en = 1, /*!< Internal GPIO pull mode for I2C scl signal*/
    .master.clk_speed = (400 * 1000), /*!< clock speed 400kHz */
  };

  ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0,&i2c_config));
    
  /**
  * Instal I2C Drive;
  */
  ESP_LOGI(TAG, "Install panel IO");
  ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0,i2c_config.mode,I2C_MASTER_RX_BUF_DISABLE,I2C_MASTER_TX_BUF_DISABLE,0));
}

/**
 * @brief OLED unitialized 
 */
static void oled_init_function (void)
{
  esp_lcd_panel_io_i2c_config_t io_config = 
  {
    .dev_addr = OLED_I2C_HW_ADDR,                     // According to SSD1306 datasheet
    .on_color_trans_done = color_trans_done_callback, // Finish collor transmit callback
    .user_ctx = NULL,                                 // User context
    .control_phase_bytes = 1,                         // According to SSD1306 datasheet
    .dc_bit_offset = 6,                               // According to SSD1306 datasheet
    .lcd_cmd_bits = 8,                                // According to SSD1306 datasheet
    .lcd_param_bits = 8,                              // According to SSD1306 datasheet
    .flags = 
    {
      .dc_low_on_data = 0,                            // DC line = 0 means command, 1 means data
      .disable_control_phase = 0,                     // Use control phase
    }
  };
  esp_lcd_new_panel_io_i2c(I2C_NUM_0, &io_config, &io_handle);
  ESP_LOGI(TAG, "Install SSD1306 panel driver");

  esp_lcd_panel_dev_config_t panel_config = 
  {
    .reset_gpio_num = GPIO_NUM_NC,
    .color_space = ESP_LCD_COLOR_SPACE_MONOCHROME,
    .bits_per_pixel = 1,
  };
  esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle);
}

/**
 * @brief OLED clear 
 */
void clear_display(esp_lcd_panel_handle_t panel_handle, int width, int height) 
{
    uint8_t clear_bitmap[] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    for(int i = 0; i < height; i++)
    {
      for(int z = 0; z < width; z++)
      {
        esp_lcd_panel_draw_bitmap(panel_handle, z, (i*8), z+1, ((i+1)*8), clear_bitmap);
        vTaskDelay(25/portTICK_PERIOD_MS);
      }
      i=i+8;
    }        
}

/**
 * @brief OLED write 
 */
void draw_text(esp_lcd_panel_handle_t panel_handle, const char *text, int x, int y) 
{
  while (*text) 
  {
    char c = *text++;
    if (c < 32 || c > 127) 
    {
      c = ' ';
    }
    printf("chat = %c \n", c);
    esp_lcd_panel_draw_bitmap(panel_handle, x, y, x + 8, y + 8, ssd1306_font8x8[c - 32]);
    x += 9;
    vTaskDelay(25/portTICK_PERIOD_MS);
  }
}


/**
  * @brief  The application set up freertos tasks and configs.
  * @retval Void
  */
void app_main(void)
{
	i2c_marter_init_function();
  oled_init_function();
    // Initialize the display
  esp_lcd_panel_reset(panel_handle);
  esp_lcd_panel_init(panel_handle);
     
  if( xTaskCreate( task_adc, "task_adc", 1024 * 2 , NULL, 2, NULL ) != pdTRUE )
  {
    if( DEBUG )
      ESP_LOGI( TAG, "error - Nao foi possivel alocar task_ADC.\r\n" );  
      return;   
  }

}

/* USER CODE CALLBACK*/
_Bool color_trans_done_callback(esp_lcd_panel_io_handle_t handle, esp_lcd_panel_io_event_data_t *event_data, void *user_ctx) 
{
    // Handle the completion of a color transmission
    ESP_LOGI("OLED", "Color transmission done");
    return true; // Return true to indicate the event was handled
}
/* USER CODE CALLBACK*/

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */

  /* User can add his own implementation to report the HAL error return state */

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