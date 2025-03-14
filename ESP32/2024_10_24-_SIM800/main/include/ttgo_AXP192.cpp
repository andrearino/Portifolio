/////////////////////////////////////////////////////////////////
/*
MIT License

Copyright (c) 2020 lewis he
Modificado por Fernando Simplicio
www.microgenios.com.br

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

main.cpp - AXP202X_Library idf Programming example
Created by Lewis he on July 29, 2020.
github:https://github.com/lewisxhe/ESP_IDF_AXP20x_Library
*/
/////////////////////////////////////////////////////////////////

#include "ttgo_AXP192.h" 

static const char *TAG = "AXP20x";

#define _I2C_NUMBER(num)            I2C_NUM_##num
#define I2C_NUMBER(num)             _I2C_NUMBER(num)

#define I2C_MASTER_SCL_IO           22                   /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO           21                   /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUMBER(1)  		 /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ          100000               /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                    /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                    /*!< I2C master doesn't need buffer */

#define WRITE_BIT                   I2C_MASTER_WRITE     /*!< I2C master write */
#define READ_BIT                    I2C_MASTER_READ      /*!< I2C master read */
#define ACK_CHECK_EN                0x1                  /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS               0x0                  /*!< I2C master will not check ack from slave */
#define ACK_VAL                     (i2c_ack_type_t)0x0  /*!< I2C ack value */
#define NACK_VAL                    (i2c_ack_type_t)0x1  /*!< I2C nack value */

AXP20X_Class axp;


/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

/**
 * @brief apx library i2c read callblack
 */
uint8_t twi_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len)
{
    if (len == 0) {
        return ESP_OK;
    }
    if (data == NULL) {
        return ESP_FAIL;
    }
    i2c_cmd_handle_t cmd;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret =  i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        return ESP_FAIL;
    }
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | READ_BIT, ACK_CHECK_EN);
    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, &data[len - 1], NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

/**
 * @brief apx library i2c write callblack
 */
uint8_t twi_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len)
{
    if (data == NULL) {
        return ESP_FAIL;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    i2c_master_write(cmd, data, len, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

/**
 *  Função responsável pela configuração do regulador AXP192 utilizando
 *  no kit ttgo v1.4.
 */
void ttgo_power_init(void)
{
    ESP_ERROR_CHECK(i2c_master_init());

    if (axp.begin(twi_read, twi_write, AXP192_SLAVE_ADDRESS)) {
        ESP_LOGE(TAG, "Error init axp20x !!!");
        while (1){ vTaskDelay(1000 / portTICK_RATE_MS); };
    }
    ESP_LOGI(TAG, "Success init axp20x !!!");

    //! Turn off unused power
    axp.setPowerOutPut(AXP192_DCDC1, AXP202_OFF);
    axp.setPowerOutPut(AXP192_LDO2, AXP202_OFF);
    axp.setPowerOutPut(AXP192_LDO3, AXP202_OFF);
    axp.setPowerOutPut(AXP192_DCDC2, AXP202_OFF);
    axp.setPowerOutPut(AXP192_EXTEN, AXP202_OFF);
	
    //! Do not turn off DC3, it is powered by esp32
    // axp.setPowerOutPut(AXP192_DCDC3, AXP202_ON);

    // Set the charging indicator to turn off
    // Turn it off to save current consumption
    // axp.setChgLEDMode(AXP20X_LED_OFF);

    // Set the charging indicator to flash once per second
    // axp.setChgLEDMode(AXP20X_LED_BLINK_1HZ);

}
