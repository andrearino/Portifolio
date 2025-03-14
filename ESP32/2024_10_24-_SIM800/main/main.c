/** --------------------------------------------------------------------------
  Autor: Prof. Fernando Simplicio;
  Modificado: André Arino
  Modificação: Execução para Estudos
  Hardware: kit TTGO ESP32 GSM v1.3 ou 1.4;
  Espressif SDK-IDF: v4.2 e v4.3;
  Curso: Formação em Internet das Coisas (IoT) com ESP32
  Link: https://www.microgenios.com.br/formacao-iot-esp32/
  --------------------------------------------------------------------------

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/**
 * Lib C
 */
#include <stdio.h>
#include <stdint.h>  
#include <string.h>

/**
 * FreeRTOS
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

/**
 * ESP
 */
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

/**
 * Driver
 */
#include "nvs_flash.h"

/**
 * LWIP
 */
#include "netif/ppp/pppos.h"
#include "netif/ppp/ppp.h"
#include "netif/ppp/pppapi.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

/**
 * kconfig;
 */
#include "sdkconfig.h"

/**
 * GSM Lib;
 */
#include "gsm_config.h"

/**
 * JSON;
 */
#include "cJSON.h"

/**
 * Variáveis Globais
 */
static const char * TAG = "main: ";
static unsigned int atraso_entre_commandos = 300;
static unsigned int numero_de_retransmissao = 5;

/**
 * Protótipos
 */
static void sim800_task(void * pvParameter);
static int sim800_init(void);
static int sim800_http_request(char * data_request, int data_request_size, char * data_response, int data_response_size);
static int sim800_write(char * cmd, int cmd_size, char *ret, int timeout, char **resp, int resp_size);

/**
 * Task responsável em enviar o valor da variável count para o servidor IFTTT;
 */
static void sim800_task(void * pvParameter)
{
	unsigned int n_erros = 0; 
	unsigned int count = 0;
	char data_request[256] = {0,};
	char data_response[1024] = {0,};
	int nbyte = 0;

    /**
     * Inicializa o módulo sim800l;
     */
	while(sim800_init() == -1) {
		ESP_LOGE(TAG, "error - falha durante a configuracao do módulo gsm.");  
	}
		
    for(;;) 
	{

		/**
		 * Prepara o JSON a ser enviado via GET;
		 * data_request: contém a string 'data' a ser enviada via GET; 
		 * data_response: armazena a mensagem de retorno da requisição GET;
		 */		
		snprintf(data_request, sizeof(data_request)-1, "{\"value1\":\"%d\"}", count++);	
		if ((nbyte = sim800_http_request(data_request, strlen(data_request), data_response, sizeof(data_response)-1)) > 0) {

			/**
			 * Imprime a mensagem recebida da requisição GET; 
			 */	
			ESP_LOGI(TAG,"Mensagem Recebida: [%.*s]. nBytes: %d", nbyte, data_response, nbyte);
			
		
		} else 	{
			
			/**
			 * Se chegou aqui é porque houve erro durante a requisição HTTP;
			 */	
			ESP_LOGE(TAG, "error - falha na requisicao http.");  
			
			/**
			 * Caso, por ventura o módulo gsm deixar de responder será gerado erros;
			 * Sendo assim, força o resete e reinicialização do modem gsm via sim800_init(); 
			 */			
			n_erros++;
			if(n_erros > 4) 	//número de tentativas antes de gerar o reset do modem gsm;
			{
				while(sim800_init() == -1) {
					ESP_LOGE(TAG, "error - falha durante a configuracao do módulo gsm.");  
				}
				n_erros = 0;
			}
		}
		
		/**
		 * Será enviado uma mensagem GET a cada 5seg;
		 */			
		vTaskDelay(5000/portTICK_PERIOD_MS );	
    }
	
	vTaskDelete(NULL);
}

/**
 *  Esta função tem o objetivo executar de forma sequencial todos os comandos habilitados e
 *  carregados em GSM_Init, de forma que o módulo GSM seja configurado.
 */
static int sim800_write(char * cmd, int cmd_size, char *ret, int timeout, char **resp, int resp_size)
{
	unsigned int count = 0;
	int rbyte = 0;
	do{ 
	    count++;
		if ((rbyte = gsm_write_command(cmd, cmd_size, ret, timeout, resp, resp_size)) >= 0) {
			/**
			 *  Entre cada mensagem enviada com sucesso aguarda 300ms;
			 */			
			vTaskDelay(atraso_entre_commandos/portTICK_PERIOD_MS);
			ESP_LOGE(TAG,"PROCESSADO COM SUCESSO");
			return rbyte;
		}  
		/**
		 *  Entre cada mensagem enviada com falha aguarda 5segundos;
		 */	
		vTaskDelay(5000/portTICK_PERIOD_MS);
	} while(count < numero_de_retransmissao);

	/**
	 *  Se chegou aqui é porque o número de tentativas de retransmissão da mensagem foi excedido;
	 */	
	return -1;
}

static int sim800_init(void)
{
	/**
	 *  Caso não esteja usado o kit TTGO GSM, comente o comando 'ttgo_power_init()';
	 */	
	ttgo_power_init();
	
	/**
	 *  RESETE e Inicializa do modem sim800L;
	 */
	gsm_init();
	
	/**
	 *  Testa a comunicação com o modem gsm;
	 */
	if (sim800_write("AT\r\n",	        //Comando AT
					sizeof("AT\r\n")-1,    	//Quantidade de bytes do comando AT
					"OK",                //String que será comparada com a string de retorno do módulo gsm;
					300,                 //Timeout: Tempo que ficará aguardando a recepção da mensagem do gsm;
					NULL,                //Caso queira ler a mensagem de retorno do gsm, é necessário passar o endereço de um ponteiro que aponta para um buffer;
					0                    //Tamanho do buffer que armazenará as mensagens de retorno. Válido somente se o paramentro acima for usado.
				 )==-1) return -1;
	
	/**
	 *  Desliga o comando de eco enviado pelo modem gsm;
	 */	
	if (sim800_write("ATE0\r\n", sizeof("ATE0\r\n")-1, "OK", 300, NULL, 0)==-1) return -1;

	/**
	 *  Imprime o modelo do modem gsm;
	 */	
	if (sim800_write("AT+GMM\r\n",	sizeof("AT+GMM\r\n")-1, "OK", 300, NULL, 0)==-1) return -1;
	
	/**
	 *  Todos os erros serão apresentados no formato de texto;
	 */	
	if (sim800_write("AT+CMEE=2\r\n", sizeof("AT+CMEE=2\r\n")-1, "OK", 3000, NULL, 0)==-1) return -1;

	/**
	 *  Garante inicialmente que o serviço GPRS esteja desabilitado;
	 */
	if (sim800_write("AT+CGATT=0\r\n",	sizeof("AT+CGATT=0\r\n")-1, "OK", 10000, NULL, 0)==-1) return -1;

	/**
	 *  Define a APN, USER, PWD e IP, conforme a operadora do cartão SIM; 
	 */	 
	if (sim800_write("AT+CSTT=\""GSM_APN"\",\""GSM_USER"\",\""GSM_PWD"\"\r\n",	sizeof("AT+CSTT=\""GSM_APN"\",\""GSM_USER"\",\""GSM_PWD"\"\r\n")-1, "OK", 30000, NULL, 0)==-1) return -1;
	
	/**
	 *  AT+CGACT command is used to activate or deactivate PDP(Packet Data Profile) context;
	 */	
	if (sim800_write("AT+CGACT=1\r\n", sizeof("AT+CGACT=1\r\n")-1, "OK", 40000, NULL, 0)==-1) return -1;

	/**
	 *  Abre a conexão; 
	 */
	if (sim800_write("AT+CIICR\r\n", sizeof("AT+CIICR\r\n")-1, 	"OK", 30000, NULL, 0)==-1) return -1;

	/**
	 *  Se chegou aqui é porque todos os comandos foram setados no módulo gsm;
	 */	
	return 0;
}

static int sim800_http_request(char * data_request, int data_request_size, char * data_response, int data_response_size)
{
	int rbyte = -1;
	
	/**
	 *  Calcula o tamanho da string total (header + body);
	 *  Reserva memória para alocar toda a string;
	 */ 			
	int len = snprintf(0, 0, "POST /trigger/"IFTTT_SERVICE_NAME"/with/key/"IFTTT_TOKEN" HTTP/1.1\r\nHOST: maker.ifttt.com\r\ncontent-type: application/json\r\ncontent-length: %d\r\n\r\n%s\r\n\r\n", data_request_size, data_request);
	char * pt = (char *) pvPortMalloc(len+1); //+null
	if (pt == NULL) { 
		return -1;   //error
	}
	memset(pt, 0, len+1);

	/**
	 *  Armazena a string na memória dinâmica;
	 */ 				
	sprintf(pt, "POST /trigger/"IFTTT_SERVICE_NAME"/with/key/"IFTTT_TOKEN" HTTP/1.1\r\nHOST: maker.ifttt.com\r\ncontent-type: application/json\r\ncontent-length: %d\r\n\r\n%s\r\n\r\n", data_request_size, data_request);


	/**
	 *  Verifica se o modem gsm está registrado na rede;
	 */		
	if (sim800_write("AT+CREG?\r\n", sizeof("AT+CREG?\r\n")-1, 	"+CREG: 0,1", 30000, NULL, 0) != -1)
	{
		/**
		 *  Get the local IP address
		 */	
		if (sim800_write("AT+CIFSR\r\n",	sizeof("AT+CIFSR\r\n")-1, NULL, 3000, NULL, 0) != -1) 
		{	
			/**
			 *  Start a TCP connection to remote address. Port 80 is TCP.
			 */					
			if (sim800_write("AT+CIPSTART=\"TCP\",\"maker.ifttt.com\",\"80\"\r\n", sizeof("AT+CIPSTART=\"TCP\",\"maker.ifttt.com\",\"80\"\r\n")-1, "CONNECT OK", 20000, NULL, 0)!=-1)
			{

				/**
				 *  Informa a quantidade de bytes a serem enviados via socket;
				 */	
				char buffer[32] = {0,}; 				 
				snprintf(buffer, sizeof(buffer)-1, "AT+CIPSEND=%d\r\n", len);				 
				if (sim800_write(buffer, strlen(buffer), ">", 5000, NULL, 0)!=-1)
				{				
					/**
					 *  Faz o envio do pacote HTTP via GET. O código 200 indica sucesso na requisição GET;
					 *  Aguarda por até 30segundos;
					 *  rbyte = -1 -> error; 
					 *  rbyte = 0 -> não aplicado neste caso, devido ao NULL da função; 
					 *  rbyte > 0 -> quantidade de bytes recebidos e armazenados em 'data_response';
					 */		
					if ((rbyte = sim800_write(pt, len, NULL, 30000, &data_response, data_response_size)) != -1)
					{
						if (rbyte > 0)
						{
							if (strnstr(data_response, "SEND OK", data_response_size))
							{
								/**
								 *  A conexão socket precisa ser encerrada, pois não podemos abrir uma nova conexão sem terminar a anterior. Portanto força o encerramento;
								 *  Em caso de falha, -1;
								 *  Em caso de sucesso, rbyte já armazena a quantida de bytes recebido do servidor.
								 */
								if (sim800_write("AT+CIPCLOSE\r\n", sizeof("AT+CIPCLOSE\r\n")-1, "OK", 3000, NULL, 0) == -1) //if  error
								{
									rbyte = -1;
								} 
																					
							}
						
						}
					}
				}	
			}
		}
	}
	

	if (pt != NULL) {
		vPortFree(pt);
	}
	return rbyte;
}



/**
 *  Inicio do programa;
 */
void app_main(void)
{
    /**
     *  Inicializa a NVS;
     */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

	/**
	 *  Cria a task socket client responsável em publicar o valor de 'count';
	 */
	if (xTaskCreate(sim800_task, "sim800_task", 1024*5, NULL, 5, NULL ) != pdTRUE ) {
        ESP_LOGE(TAG, "error - Nao foi possivel alocar sim800_task.\r\n");  
		return;   
    }
}
