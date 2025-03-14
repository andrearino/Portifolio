/**
 * Lib C
 */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/**
 * FreeRTOS
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

/**
 * ESP
 */
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

/**
 * Drives;
 */
#include "driver/uart.h"
#include "driver/gpio.h"
#include "driver/i2c.h"

/**
 * kconf;
 */
#include "sdkconfig.h"

/**
 * Configura o canal I2C responsável em habilitar o regulador
 * do kit ttgo GSM.
 */
#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00

#define _I2C_NUMBER(num) I2C_NUM_##num
#define I2C_NUMBER(num) _I2C_NUMBER(num)

#define I2C_MASTER_SCL_IO 22               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 21               /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUMBER(0)       /*!< I2C port number for master dev */

#define I2C_MASTER_FREQ_HZ 100000          /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0        /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0        /*!< I2C master doesn't need buffer */
#define ESP_SLAVE_ADDR IP5306_ADDR         /*!< ESP32 slave address, you can set any 7bit value */

#define WRITE_BIT I2C_MASTER_WRITE         /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ           /*!< I2C master read */
#define ACK_CHECK_EN  0x1                  /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                  /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                        /*!< I2C ack value */
#define NACK_VAL 0x1                       /*!< I2C nack value */

/**
 *  Protótipos;
 */
static esp_err_t i2c_master_init(void);
static esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t *data_wr, size_t size);

/**
 *  Função responsável pela configuração do regulador IP5306 utilizando
 *  no kit ttgo v1.3.
 */
void ttgo_power_init( void )
{
	/**
	 *  Habilita regulador I2C do kit ttgo v1.3 GSM; 
	 *  Caso utilize outro kit, comente as linhas i2C abaixo;
	 *  A referencia de pinagens do I2C são encontradas no topo desta página.
	 */
	/** ------------------------------------------------- */
	uint8_t data_wr[] = { IP5306_REG_SYS_CTL0, 0x37 };
	i2c_master_init();
	i2c_master_write_slave(I2C_MASTER_NUM, data_wr, 2);
	/** ------------------------------------------------- */
}


/**
 *  Inicializa canal I2C como master;
 */
static esp_err_t i2c_master_init( void )
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
	conf.clk_flags = 0;
    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, I2C_MODE_MASTER,
                              I2C_MASTER_RX_BUF_DISABLE,
                              I2C_MASTER_TX_BUF_DISABLE, 0);
}

/**
 *  Função de escrita Master -> Slave via I2C; 
 */
static esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t *data_wr, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
	
    return ret;
}
