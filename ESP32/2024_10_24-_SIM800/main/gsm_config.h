#ifndef __GSM_CONFIG_H_
#define __GSM_CONFIG_H_

/**
 * POST request;
 */
#define IFTTT_TOKEN "dexPv7Fz68IcQzPf9E_me3"
#define IFTTT_SERVICE_NAME "sim800"
#define URL_REQUEST "http://maker.ifttt.com/trigger/"IFTTT_SERVICE_NAME"/with/key/"IFTTT_TOKEN

/**
 *  Função responsável pela configuração do regulador IP5306 utilizado
 *  no kit ttgo v1.3. Comente caso esteja usando o kit  ttgo v1.4.
 */
#define TTGO_IP5306

/**
 *  Função responsável pela configuração do regulador AXP192 utilizado
 *  no kit ttgo v1.4. Comente caso esteja usando o kit ttgo v1.3.
 */
//#define TTGO_AXP192

/**
 *  Custom sim800L board;
 *  Caso esteja utilizando um Shield SIM800L, habilite o comando abaixo;
 */
//#define CUSTOM_SIM800L_BOARD


/**
 * SIM;
 */
#define GSM_APN 	"claro.com.br"
#define GSM_USER 	"claro"
#define GSM_PWD  	"claro"
#define GSM_IP  	"claro.com.br"

/**
 *  UART;
 */
#define CONFIG_UART_NUM 		1
#define CONFIG_GSM_TX 			27
#define CONFIG_GSM_RX 			26
#define CONFIG_GSM_BDRATE 		115200
#define CONFIG_GSM_RST 			5
#define CONFIG_GSM_PWKEY 		4
#define CONFIG_GSM_POWER_ON 	23


#if defined TTGO_IP5306
	#include "include/ttgo_IP5306.h"	
#elif defined TTGO_AXP192
	#include "include/ttgo_AXP192.h"
#elif defined CUSTOM_SIM800L_BOARD
	#define ttgo_power_init() (void*)0
#elif 
	#error gsm_config.h error
#endif

/**
 * biblioteca gsm;
 */
#include "gsm.h"

#endif