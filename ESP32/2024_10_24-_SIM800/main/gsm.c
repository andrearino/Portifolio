/** --------------------------------------------------------------------------
  Autor: Prof. Fernando Simplicio;
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
 * LWIP;
 */
#include "lwip/dns.h"
#include "esp_netif.h"
#include "netif/ppp/pppos.h"
#include "netif/ppp/ppp.h"
#include "netif/ppp/pppapi.h"

/**
 * gsm;
 */
#include "gsm_config.h"

/**
 * Macro responsável em encapusular a variável 'gsm_status' visto que esta está sendo utilizada
 * em tasks diferentes;
 * GSM_SET_STATUS -> altera o status de gsm_status;
 * GSM_GET_STATUS -> faz a leitura do status de gsm_status;
 */
#define GSM_SET_STATUS(x) do{ \
							  xSemaphoreTake(pppos_mutex,1000/portTICK_RATE_MS); \
							  gsm_status=x; \
							  xSemaphoreGive(pppos_mutex); \
							} while(0)

#define GSM_GET_STATUS()  ({  int ret; \
							  xSemaphoreTake(pppos_mutex,1000/portTICK_RATE_MS); \
							  ret=gsm_status; \
							  xSemaphoreGive(pppos_mutex); \
							  ret; \
						  })

/**
 * Define o tamanho do buffer da UART na recepção serial;
 */									
#define GSM_RX_BUFFER_SIZE (1024)
	
/**
 *  Protótipos;
 */
int gsm_write_command(char * cmd, int cmd_size, char * ret, int timeout, char **resp, int resp_size);
int gsm_config(void);  
int pppos_init(void);
int pppos_is_connected(void);
static void info_command(char *cmd, int cmd_size, char *info);
static esp_err_t gsm_uart_init(void); 
void ppposDisconnect(uint8_t end_task, uint8_t rfoff);
static void enable_all_commands(void); 

/**
 * Variáveis Globais
 */
static const char * TAG = "GSM";
static int uart_num = CONFIG_UART_NUM;
EventGroupHandle_t pppos_event_group;
const int PPPOS_CONNECTED_BIT = BIT0;
static QueueHandle_t pppos_mutex = NULL;
TaskHandle_t xHandle = NULL;
struct netif ppp_netif;       // The PPP IP interface
static ppp_pcb * ppp = NULL;  // The PPP control block

/**
 * Esta variável é utilizada para sinalizar o status da conexão PPP;
 * Para ler ou modificar o status dessa variável utilize as macros seguintes:
 * GSM_SET_STATUS -> altera o status de gsm_status;
 * GSM_GET_STATUS -> faz a leitura do status de gsm_status; 
 */		 
static Gsm_status_t gsm_status = GSM_STATE_FIRSTINIT;

/**
 * Comando AT para teste inicial da comunicação entre modem gsm e ESP32;
 */
static Gsm_cmd_t cmd_AT =
{
	.cmd = "AT\r\n",
	.cmd_size = sizeof("AT\r\n")-1,
	.cmd_response = "OK",
	.timeout_ms = 300,
	.delay_ms = 0,
	.skip = 0,
};

/**
 * Comando AT -> Echo Mode off;
 */
static Gsm_cmd_t cmd_ATE0 =
{
	.cmd = "ATE0\r\n",
	.cmd_size = sizeof("ATE0\r\n")-1,
	.cmd_response = "OK",
	.timeout_ms = 300,
	.delay_ms = 0,
	.skip = 0,
};

/**
 * Comando AT -> Disable phone both transmit and receive RF circuits.;
 */
static Gsm_cmd_t cmd_CFUN4 =
{
	.cmd = "AT+CFUN=4\r\n",
	.cmd_size = sizeof("ATCFUN=4,0\r\n")-1,
	.cmd_response = "OK",
	.timeout_ms = 10000,
	.delay_ms = 1000,
	.skip = 0,
};

/**
 * Comando AT -> Disconnect Existing Connection;
 * Disconnect existing call by local TE from Command line and terminate call
 */
static Gsm_cmd_t cmd_ATH =
{
	.cmd = "ATH\r\n",
	.cmd_size = sizeof("ATH\r\n")-1,
	.cmd_response = "OK",
	.timeout_ms = 300,
	.delay_ms = 0,
	.skip = 0,
};

/**
 * Comando AT -> Reset Default Configuration .
 */
static Gsm_cmd_t cmd_ATZ =
{
	.cmd = "ATZ\r\n",
	.cmd_size = sizeof("ATZ\r\n")-1,
	.cmd_response = "OK",
	.timeout_ms = 300,
	.delay_ms = 0,
	.skip = 0,
};

/**
 * Comando AT -> Full functionality (Default);
 */
static Gsm_cmd_t cmd_CFUN1 =
{
	.cmd = "AT+CFUN=1\r\n",
	.cmd_size = sizeof("ATCFUN=1,0\r\n")-1,
	.cmd_response = "OK",
	.timeout_ms = 10000,
	.delay_ms = 1000,
	.skip = 0,
};

/**
 * Comando AT -> TA returns an alphanumeric string indicating whether some password is required or not;
 * READY MT is not pending for any password;
 */
static Gsm_cmd_t cmd_CPIN =
{
	.cmd = "AT+CPIN?\r\n",
	.cmd_size = sizeof("AT+CPIN?\r\n")-1,
	.cmd_response = "CPIN: READY",
	.timeout_ms = 5000,
	.delay_ms = 0,
	.skip = 0,
};

/**
 * Comando AT -> Network Registration;
 * 1 Registered, home network;
 */
static Gsm_cmd_t cmd_CREG =
{
	.cmd = "AT+CREG?\r\n",
	.cmd_size = sizeof("AT+CREG?\r\n")-1,
	.cmd_response = "CREG: 0,1",
	.timeout_ms = 15000,
	.delay_ms = 2000,
	.skip = 0,
};

/**
 * Comando AT -> Define PDP Context e Access Point Name;
 */
static Gsm_cmd_t cmd_APN =
{
	.cmd = "AT+CGDCONT=1,\"IP\",\""GSM_APN"\"\r\n",
	.cmd_size = sizeof("AT+CGDCONT=1,\"IP\",\""GSM_APN"\"\r\n")-1,
	.cmd_response = "OK",
	.timeout_ms = 8000,
	.delay_ms = 0,
	.skip = 0,
};

/**
 * Comando AT -> Enter Data State Point to Point protocol for a PDP such as IP;
 */
static Gsm_cmd_t cmd_CGDATA =
{
	.cmd = "AT+CGDATA=\"PPP\",1\r\n",
	.cmd_size = sizeof("AT+CGDATA=\"PPP\",1\r\n")-1,
	.cmd_response = "CONNECT",
	.timeout_ms = 30000,
	.delay_ms = 1000,
	.skip = 0,
};

/**
 * 
 */
static Gsm_cmd_t * GSM_Init[] =
{
	&cmd_AT,
	&cmd_ATZ,		
	&cmd_ATE0,
	&cmd_CFUN4,
	&cmd_ATH,
	&cmd_CFUN1,
	&cmd_CPIN,
	&cmd_CREG,
	&cmd_APN,
	&cmd_CGDATA,
};

/**
 * Quantidade de comandos carregada em GSM_Init; 
 */
#define GSM_InitCmdsSize  (sizeof(GSM_Init)/sizeof(Gsm_cmd_t *))

/**
 * Função de callback da interface PPP, chamada quando ocorrer mudança de estado na conexão ip entre dispositivos;
 */
static void ppp_status_cb(ppp_pcb *pcb, int err_code, void *ctx)
{
	struct netif *pppif = ppp_netif(pcb);
	LWIP_UNUSED_ARG(ctx);

	switch (err_code) 
	{
		case PPPERR_NONE: 
			ESP_LOGI(TAG,"status_cb: Connected");
			#if PPP_IPV4_SUPPORT
				ESP_LOGI(TAG,"   ipaddr    = %s", ipaddr_ntoa(&pppif->ip_addr));
				ESP_LOGI(TAG,"   gateway   = %s", ipaddr_ntoa(&pppif->gw));
				ESP_LOGI(TAG,"   netmask   = %s", ipaddr_ntoa(&pppif->netmask));
			#endif

			#if PPP_IPV6_SUPPORT
				ESP_LOGI(TAG,"   ip6addr   = %s", ip6addr_ntoa(netif_ip6_addr(pppif, 0)));
			#endif
			
			/**
			 * Eventgroup é usado para sinalizar quando a conexão ocorrer com sucesso via PPP; 
			 */
			xEventGroupSetBits(pppos_event_group, PPPOS_CONNECTED_BIT);
			GSM_SET_STATUS(GSM_STATE_CONNECTED);
			break;
		
		case PPPERR_PARAM: 
			ESP_LOGE(TAG,"status_cb: Invalid parameter");
			break;
		
		case PPPERR_OPEN:
			ESP_LOGE(TAG,"status_cb: Unable to open PPP session");
			break;
		
		case PPPERR_DEVICE: 
			ESP_LOGE(TAG,"status_cb: Invalid I/O device for PPP");
			break;

		case PPPERR_ALLOC:
			ESP_LOGE(TAG,"status_cb: Unable to allocate resources");
			break;
		
		case PPPERR_USER: 
			ESP_LOGW(TAG,"status_cb: User interrupt (disconnected)");
			xEventGroupClearBits(pppos_event_group, PPPOS_CONNECTED_BIT);
			GSM_SET_STATUS(GSM_STATE_DISCONNECTED);
			break;
		
		case PPPERR_CONNECT: 
			ESP_LOGE(TAG,"status_cb: Connection lost");	
			xEventGroupClearBits(pppos_event_group, PPPOS_CONNECTED_BIT);
			GSM_SET_STATUS(GSM_STATE_DISCONNECTED);
			break;
		
		case PPPERR_AUTHFAIL: 
			ESP_LOGE(TAG,"status_cb: Failed authentication challenge");
			break;

		case PPPERR_PROTOCOL: 
			ESP_LOGE(TAG,"status_cb: Failed to meet protocol");
			break;
		
		case PPPERR_PEERDEAD:
			ESP_LOGE(TAG,"status_cb: Connection timeout");
			break;
		
		case PPPERR_IDLETIMEOUT: 
			ESP_LOGE(TAG,"status_cb: Idle Timeout");
			break;
		
		case PPPERR_CONNECTTIME:
			ESP_LOGE(TAG,"status_cb: Max connect time reached");
			break;
	
		case PPPERR_LOOPBACK: 
			ESP_LOGE(TAG,"status_cb: Loopback detected");
			break;
		
		default:
			ESP_LOGE(TAG,"status_cb: Unknown error code %d", err_code);
			break;
		
	}
}

/**
 *  Função de callback chamada pela interface PPP quando houver bytes a serem transmitidos
 *  na camada de rede da LWIP; Sendo assim, os bytes recebidos nesta função são retransmitidos
 *  via uart para o módulo GSM -> USANDO O PROTOCOLO PPP; 
 *  A função retorna a quantidade de bytes que definitivamente foi enviada pela UART;
 */
static u32_t ppp_output_cb(ppp_pcb *pcb, u8_t *data, u32_t len, void *ctx)
{
	uint32_t ret = uart_write_bytes(uart_num, (const char*)data, len);
    uart_wait_tx_done(uart_num, 10/portTICK_RATE_MS);
    return ret;
}


/**
 * Quando um comando AT é recebido e processado com sucesso pelo módulo GSM, este é marcado como já processado (1);
 * Assim, caso ocorra falha durante o processamento do comando AT, será retransmitido novamente somente os comandos AT
 * que ainda não foram processados da lista. 
 * Esta função habilita novamente todos os comandos AT da lista GSM_Init;
 */
static void enable_all_commands(void)
{
	for (int idx = 0; idx < GSM_InitCmdsSize; idx++) {
		GSM_Init[idx]->skip = 0;
	}
}



/**
 * Esta task é responsável por configurar via comandos AT o modem gsm, colocando-o em modo PPP, 
 * assim como envia todos os bytes recebidos pela UART do ESP32 e a encaminha para a camada de rede, via PPP;
 */
static void pppos_client_task(void * pvParameter)
{
    /**
     *  Define o buffer de recepção com 1024 bytes;
     */
    char data[GSM_RX_BUFFER_SIZE];

    /**
     *  É necessário primeiramente configurar o modem gsm, portanto parte no estado GSM_STATE_FIRSTINIT;
     */
	GSM_SET_STATUS(GSM_STATE_FIRSTINIT);

	for (;;)	
	{
		
		if (GSM_GET_STATUS() == GSM_STATE_FIRSTINIT)  
		{
			ESP_LOGI(TAG, "STATE GSM_STATE_FIRSTINIT");
			
			/**
			 * Configura o modem gsm enviando os comandos AT de inicialização;
			 * returno 0 -> ok;
			 *        -1 -> error;
			 */			
			if (gsm_config() == 0) 
			{
				ESP_LOGE(TAG, "inicializando PPPoS...");
				
				/**
				 *  Inicializa a interface PPP da LWIP customizada do IDF, onde:
				 *  ppp_output_cb: é a função de callback chamada quando dados chegarem na camada de rede da LWIP;
				 *  ppp_status_cb: é a função de callback que informa o status da interface PPP;
				 *  ctx_cb, é opcional 'user-provided callback context pointer';
				 *  Sintaxe: ppp_pcb * 	pppapi_pppos_create (struct netif *pppif, pppos_output_cb_fn output_cb, ppp_link_status_cb_fn link_status_cb, void *ctx_cb)
				 */
				ppp = pppapi_pppos_create(&ppp_netif, ppp_output_cb, ppp_status_cb, NULL);
				if (ppp == NULL) {
					ESP_LOGE(TAG, "Error na ininicialização da PPPoS");
					break; //end task;
				}

				/**
				 *  Define a interface PPP como a de roteamento padrão da rede;
				 */
				pppapi_set_default(ppp);
				
				/**
				 *  Autenticação das sessões PPP;
				 *  Password Authentication Protocol (PAP): funciona como um procedimento de login padrão. 
				 *  O sistema remoto se autentica usando uma combinação estática de nome de usuário e senha.
				 *  O PAP é um protocolo de autenticação cliente-servidor baseado em senha. 
				 *  A autenticação ocorre apenas uma vez no início de um processo de estabelecimento de sessão.
				 *  Pode ser do tipo: PPPAUTHTYPE_NONE, PPPAUTHTYPE_PAP, PPPAUTHTYPE_CHAP, PPPAUTHTYPE_MSCHAP,
				 *  PPAUTHTYPE_MSCHAP_V2, PPPAUTHTYPE_EAP, PPPAUTHTYPE_ANY
				 *  Também é possível configurar assim: pppapi_set_auth(ppp, PPPAUTHTYPE_NONE, "", "");
				 *  
				 *  Atenção: Garanta via menuconfig que a opção PAP esteja habilitada: 
					CONFIG_PPP_PAP_SUPPORT=y
					# CONFIG_PPP_CHAP_SUPPORT is not set
					# CONFIG_PPP_MSCHAP_SUPPORT is not set
					# CONFIG_PPP_MPPE_SUPPORT is not set
				 */
				pppapi_set_auth(ppp, PPPAUTHTYPE_PAP, GSM_USER, GSM_PWD);
				
				/**
				 * Solicita ao par até 2 endereços de servidor DNS;
				 */			
				ppp_set_usepeerdns(ppp, 1);
				
				/**
				 *  Iniciar negociação PPP, sem esperar (holdoff = 0), só pode ser chamado se a sessão PPP estiver desconectada.
				 */
				pppapi_connect(ppp, 0);
				
				/**
				 *  troca de estado;
				 */					
				GSM_SET_STATUS(GSM_STATE_CONNECTED);
			}
			
		} else
		if (GSM_GET_STATUS() == GSM_STATE_CONNECTED)
		{
			
			/**
			 *  Os dados transmitidos pelo módulo GSM são recebidos pela uart do ESP32. 
			 *  Esses dados são transmitidos pela interface PPP para a camada de rede TCP da LWIP;
			 */	
			memset(data, 0, GSM_RX_BUFFER_SIZE);
			int len = uart_read_bytes(uart_num, (uint8_t*)data, GSM_RX_BUFFER_SIZE, 30/portTICK_RATE_MS);
			if (len > 0) {
				pppos_input_tcpip(ppp, (u8_t*)data, len);
			}
			
		} else 		
		if (GSM_GET_STATUS() == GSM_STATE_DISCONNECTED)
		{
			    /**
				 *  Se chegou aqui é porque a interface PPP foi desconectada (foi chamada a função de callback da PPP)
				 *  Portanto, para reiniciar a conexão é necessário forçar o estado	 GSM_STATE_FIRSTINIT;
				 */
				ESP_LOGI(TAG, "STATE GSM_STATE_DISCONNECTED");
				
				xSemaphoreTake(pppos_mutex, 100/portTICK_RATE_MS);	
				/**
				 *  A interface PPP precisa ser encerrada;
				 */	
				if (ppp != NULL) { 
					pppapi_close(ppp, 0);
					ppp_free(ppp);
				}
				/**
				 *  ... e os comandos AT precisam ser novamente habilitados para serem transmitidos.
				 */	
				enable_all_commands();
				xSemaphoreGive(pppos_mutex);
				
				/**
				 *  Muda o estado para GSM_STATE_FIRSTINIT;
				 */
				GSM_SET_STATUS(GSM_STATE_FIRSTINIT);
					
		}
		
		/**
		 *  Este delay é necessário para que esta task seja bloqueada em algum momento;
		 */		
		vTaskDelay(500/portTICK_PERIOD_MS);

	}

	/**
	 *  A princípio, os códigos seguintes não deveriam ser executados...
	 */	
	ESP_LOGE(TAG, "PPPoS TASK TERMINATED");
	vTaskDelete(NULL);
}

/**
 *  Verifica o status da conexão PPP;
 *  retorno -1 -> error
 *           0 -> ok
 */
int pppos_is_connected(void)
{
	EventBits_t uxBits = xEventGroupWaitBits(pppos_event_group, PPPOS_CONNECTED_BIT, false, true, 0);
	if((uxBits & PPPOS_CONNECTED_BIT) == PPPOS_CONNECTED_BIT)
	{
		return 0;
	}
	return -1;
}

/**
 *  Função responsável pela inicialização da camada PPP da LWIP;
 */
int pppos_init(void)
{

	/**
	 *  Durante a execução do programa, pode ocorrer da conexão PPP ficar offline. 
	 *  Caso seja a primeira chamada de pppos_init(), inicializa o sistema; 
	 *  Caso a task pppos já tiver sido inicializada, ela não deverá ser recriada;
	 */
	if (xHandle == NULL)
	{
		/**
		 *  Este mutex é utilizado para proteger a variável 'gsm_status';
		 */
		if (pppos_mutex == NULL) {
			pppos_mutex	= xSemaphoreCreateMutex();
			xSemaphoreGive(pppos_mutex);
		}

		/**
		 *  Utilizado para sinalizar quando estivermos conectado ou desconectado com a 
		 *  interface PPP;
		 */		
		if (pppos_event_group == NULL) {
			pppos_event_group = xEventGroupCreate();
		}
		
		/**
		 *  Caso esteja utilizando a versão IDF 3.x utilize a função
		 *  tcpip_adapter_init()
		 *  Para versão IDF 4.x use esp_netif_init();
		 */			
		if (esp_netif_init() == ESP_FAIL) {
			ESP_LOGE(TAG, "error na inicialização da netif.\r\n");  
			return -1;		
		}
		
		/** 
		 *  Os comandos abaixo podem ser uteis em algum momento, portanto
		 *  vou mantê-los aqui, porém desabilitados. 
		 *  Use a configuração do DNS abaixo caso seja apresentado a mensagem de error:
		 *  E (55346) example: DNS lookup failed err=202 res=0x0
		 */
		#if 0    			
			ip_addr_t dnsserver;
			inet_pton(AF_INET, "8.8.8.8", &dnsserver);
			dns_setserver(0, &dnsserver);
			inet_pton(AF_INET, "8.8.4.4", &dnsserver);
			dns_setserver(1, &dnsserver);
		#endif
			
		/**
		 *  Cria task pppos;
		 *  Preferencialmente mantenha suas demais tasks com prioridade menor que 10, visto que 
		 *  a task pppos_client_task precisa ter prioridade mais alta;
		 */
		if (xTaskCreate(&pppos_client_task, "pppos_client_task", 4*1024, NULL, 10, &xHandle) != pdTRUE) {
			ESP_LOGE(TAG, "error - Nao foi possivel alocar pppos_client_task.\r\n");  
			return -1;   
		}
		
	} 
	
	/**
     * Aguarda a conexão da interface PPP a internet;
	 * Isso somente irá acontecer quando for recebido o ES32 receber o endereço IP
	 * da operadora;
     */
    xEventGroupWaitBits(pppos_event_group, PPPOS_CONNECTED_BIT, false, true, portMAX_DELAY);

    /** sucesso. Módulo GSM foi conectado a internet; */
	return 0;
}

/**
 *  Esta função somente é utilizada em modo DEBUG, para que 
 *  vejamos o valor da mensagem de retorno do módulo GSM, recebido pelo ESP32; 
 */
static void info_command(char *cmd, int cmd_size, char *info)
{
	char buf[cmd_size+2];
	memset(buf, 0, cmd_size+2);

	for (int i=0; i<cmd_size;i++) 
	{	
		/**
		 *  Todos os caracteres recebidos que forem 
		 *  menor que 32 e maior que 127 são representados por '.' em buf; 
		 */
		if ((cmd[i] != 0x00) && ((cmd[i] < 0x20) || (cmd[i] > 0x7F))) {
			buf[i] = '.';
		} else {
			buf[i] = cmd[i];
		}
 
		if (buf[i] == '\0') { 
			break;
		}
	}
	ESP_LOGI(TAG,"%s [%s]", info, buf);
}

/**
 *  Esta função tem o objetivo executar de forma sequencial todos os comandos habilitados e
 *  carregados em GSM_Init, de forma que o módulo GSM seja configurado.
 *  return  0 -> ok;
 *  	   -1 -> error;
 */
int gsm_config(void)
{
	int index = 0;
	int nfail = 0;
	while(index < GSM_InitCmdsSize)
	{
		/**
		 *  No descritor dos comandos existe um campo chamado skip. 
		 *  Por meio dele é possível informar se o comando AT é para ser executado;
		 *  quando skip = 0, sim é para ser executado;			 
		 */
		if (GSM_Init[index]->skip) {
			info_command(GSM_Init[index]->cmd, GSM_Init[index]->cmd_size, "Skip command:");
			/**
			 *  Salta e pega o próximo comando AT da lista a ser executado;
			 */
			index++;
			continue;
		}
		
		/**
		 *  Envia o comando AT da lista (GSM_Init[index]->cmd) para o módulo GSM e aguarda
		 *  por (GSM_Init[index]->timeout_ms). A mensagem de retorno é comparado com 
		 *  a string carregada em GSM_Init[index]->cmd_response;
		 *  Caso o retorno da função gsm_write_command seja diferente de zero, indica que houve falha
		 *  de comunicação ou devido a mensagem de retorno ser diferente do esperado ou devido a timeout;
		 *  Em caso de erro, a lista de comandos AT será enviada novamente, por até 20 vezes;
		 */
		if (gsm_write_command(GSM_Init[index]->cmd,
				GSM_Init[index]->cmd_size,
				GSM_Init[index]->cmd_response,
				GSM_Init[index]->timeout_ms, NULL, 0) == -1)
		{
			/**
			 *  Se chegou aqui é porque houve erro na comunicação com o módulo GSM;
			 *  Portanto, é inicializado novamente o envio da lista de comandos AT, por até 20 vezes;
			 */
			ESP_LOGW(TAG,"Wrong response, restarting...");

			
			nfail++;
			if (nfail > NUM_MAX_RETRANSMISSAO_CMD_AT) return -1;

			vTaskDelay(3000/portTICK_PERIOD_MS);
			index = 0;
			continue;
		}

		/**
		 *  Se chegou aqui é porque o comando AT enviado foi executado com sucesso.
		 *  Entre um comando AT e outro, aguarda por GSM_Init[index]->delay_ms;
		 */			
		if (GSM_Init[index]->delay_ms > 0) vTaskDelay(GSM_Init[index]->delay_ms / portTICK_PERIOD_MS);

		/**
		 *  Como este comando AT já foi processado com sucesso, não há necessidade de retransmiti-lo;
		 */				
		GSM_Init[index]->skip = 1;
		if (GSM_Init[index] == &cmd_CREG) GSM_Init[index]->delay_ms = 0; //CREG com timeout =0 ??
		index++;
	}
		
	return 0;
}


/**
 *  Esta função é responsável em transmitir e receber as mensagens via comando AT com o módulo GSM;
 */
int gsm_write_command(char * cmd, int cmd_size, char *ret, int timeout, char **resp, int resp_size)
{
	/**
	 *  retorno: -1 -> error
	 *  		  0 -> ok
	 *           >0 -> quantidade de bytes que foi carregado no buffer 
	 */
	char sresp[256] = {'\0'};
	char data[256] = {'\0'};
    int len, res = -1, idx = 0, tot = 0, timeoutCnt = 0;

	vTaskDelay(100/portTICK_PERIOD_MS);
	
	/**
	 *  Limpa o buffer da uart;
	 */
	uart_flush(uart_num);

	if (cmd != NULL) {
		if (cmd_size == -1) cmd_size = strlen(cmd);
		info_command(cmd, cmd_size, "AT COMMAND:");
		
		/**
		 *  Envia comando AT para o módulo GSM;
		 *  Observação: Neste momento não estamos conectado a interface PPP;
		 */
		uart_write_bytes(uart_num, (const char*)cmd, cmd_size);
		uart_wait_tx_done(uart_num, 100/portTICK_RATE_MS);
	}

	/**
	 *  Aguarda a mensagem de retorno do módulo GSM;
	 *  Processa comandos AT como este:
	 *  int res = gsm_write_command("AT+CFUN?\r\n", NULL, NULL, -1, 2000, &pbuf, 63);
	 */
	if (resp != NULL) {
		char *pbuf = *resp;
		len = uart_read_bytes(uart_num, (uint8_t*)data, sizeof(data)-1, timeout/portTICK_RATE_MS);
		
		/**
		 *  Algum byte recebido do módulo GSM durante timeout?
		 */
		while (len > 0) {
			
			/**
			 *  Caso a mensagem recebida seja maior que 256 bytes, o buffer de recebimento dos bytes de leitura
			 *  será realocado em memória (via comando realloc) em um novo bloco de memória de tamanho resp_size+512;
			 *  Essa condição é importante para receber pacotes http, por exemplo; 
			 */			
			if ((tot+len) >= resp_size) {
				char *ptemp = realloc(pbuf, resp_size+512);
				if (ptemp == NULL) return 0;
				resp_size += 512;
				pbuf = ptemp;
			}
			memcpy(pbuf+tot, data, len);
			tot += len;
			
			if (tot < resp_size) {
				resp[tot] = '\0';
			}
			len = uart_read_bytes(uart_num, (uint8_t*)data, sizeof(data)-1, 100/portTICK_RATE_MS);
		}
		
		/**
		 *  Retorna a quantidade de bytes que foi armazenada no buffer que armazena a mensagem de retorno do módulo GSM; 
		 */		
		*resp = pbuf;
		return tot;
	}

	/**
	 *  Se ret for NULL então significa que não é para verificar o retorno do módulo gsm;
	 *  Ou seja,  não há necessidade de ler a mensagem de retorno enviada pelo modem gsm;
	 */
	if(ret == NULL) {
		vTaskDelay(timeout/portTICK_PERIOD_MS);
		return 0;
	}

	/**
	 *  Se chegou aqui é porque o comando transmitido é similar a: 
	 *  int res = gsm_write_command("AT\r\n", GSM_OK_Str, NULL, 4, 1000, NULL, 0);
	 */
	idx = 0;
	while (1) {

		memset(data, 0, sizeof(data));
		len = 0;
		len = uart_read_bytes(uart_num, (uint8_t*)data, sizeof(data)-1, 100/portTICK_RATE_MS);
		if (len > 0) {
			for (int i=0; i<len;i++) {
				if (idx < sizeof(data)-1) {
					/**
					 *  Armazena no buffer de recepção sresp todos os bytes válidos recebidos, 
					 *  a partir do 32 (caractere space da tabela ASC);
					 *  Qualquer outro caractere recebido fora do intervalo 32 a 128 (decimal) será atribuido o '.' (corresponde a 0x2e);
					 */	
					if ((data[i] >= 0x20) && (data[i] < 0x80)) sresp[idx++] = data[i];
					else sresp[idx++] = 0x2e;
				}
			}
			tot += len;
		}
		else {
			/**
			 *  Algum byte foi recebido pela uart?
			 */			
			if (tot > 0) {
				
				/**
				 *  Compara a mensagem recebida com 'ret';
				 */
				if (strstr(sresp, ret) != NULL) {
					ESP_LOGI(TAG,"AT RESPONSE: [%s]", sresp);
					res = 0;
					break;
				} else {				
					//ESP_LOGE(TAG,"AT BAD RESPONSE: [%s]", sresp);
					//res = -1;
					//break;
				}
				
			}
		}

		timeoutCnt += 100;
		if (timeoutCnt > timeout) {
			ESP_LOGE(TAG,"AT: TIMEOUT");
			res = -1;
			break;
		}
	}

	return res;
}

/**
 *  Função responsável pela inicialização do modem gsm;
 */
esp_err_t gsm_init( void )
{
	gpio_pad_select_gpio(CONFIG_GSM_RST);
	gpio_set_direction(CONFIG_GSM_RST, GPIO_MODE_OUTPUT);
	gpio_pad_select_gpio(CONFIG_GSM_PWKEY);
	gpio_set_direction(CONFIG_GSM_PWKEY, GPIO_MODE_OUTPUT);
	gpio_pad_select_gpio(CONFIG_GSM_POWER_ON);
	gpio_set_direction(CONFIG_GSM_POWER_ON, GPIO_MODE_OUTPUT);
	
	/**
	 *  Turn off;
	 */	
	gpio_set_level(CONFIG_GSM_RST, 0);
	gpio_set_level(CONFIG_GSM_PWKEY, 0);
	gpio_set_level(CONFIG_GSM_POWER_ON, 0);	
	ESP_LOGI(TAG, "Module GSM un-powered");
	vTaskDelay(500/portTICK_PERIOD_MS);			
	
	/**
	 *  Turn on;
	 */	
	gpio_set_level(CONFIG_GSM_RST, 1);
	gpio_set_level(CONFIG_GSM_PWKEY, 1);
	gpio_set_level(CONFIG_GSM_POWER_ON, 1);		
	ESP_LOGI(TAG, "Module GSM powered");
	vTaskDelay(500/portTICK_PERIOD_MS);		
	
	/**
	 *   Pull down PWRKEY;
	 */	
	gpio_set_level(CONFIG_GSM_PWKEY, 0);
	ESP_LOGI(TAG, "Module GSM PWKEY LOW");
	vTaskDelay(1100/portTICK_PERIOD_MS);		
	
	/**
	 *   Modem is ready 10s after pull down of PWKEY
	 */	
	gpio_set_level(CONFIG_GSM_PWKEY, 1);
	ESP_LOGI(TAG, "Module GSM PWKEY HIGH");
	vTaskDelay(10000/portTICK_PERIOD_MS);	
	
	gsm_uart_init(); 
	
    return ESP_OK;
}

/**
 *  Função responsável pela inicialização da UART conectada
 *  ao módulo GSM;
 */
static esp_err_t gsm_uart_init(void)
{
    /**
     *  Configura a UART conectada ao módulo GSM;
     */
    gpio_set_direction(CONFIG_GSM_TX, GPIO_MODE_OUTPUT);
	gpio_set_direction(CONFIG_GSM_RX, GPIO_MODE_INPUT);
	gpio_set_pull_mode(CONFIG_GSM_RX, GPIO_PULLUP_ONLY);

	uart_config_t uart_config = {
		.baud_rate = CONFIG_GSM_BDRATE,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};
	uart_param_config(uart_num, &uart_config);
    
	/**
     *  Define os pinos da UART, conforme: uart_set_pin(uart, TX, RX, RTS, CTS);
     */
	uart_set_pin(uart_num, CONFIG_GSM_TX, CONFIG_GSM_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	uart_driver_install(uart_num, 2048, 2048, 0, NULL, 0);
    
	return ESP_OK;
}