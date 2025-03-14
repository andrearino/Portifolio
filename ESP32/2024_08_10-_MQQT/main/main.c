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
 * WiFi
 */
#include "esp_wifi.h"
/**
 * Callback do WiFi
 */
#include "esp_event.h"

/**
 *  Logs;
 */
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"

/**
 *  Drivers;
 */
#include "nvs_flash.h"
#include "driver/gpio.h"

/**
 *  LWIP;
 */
#include "esp_netif.h"
#include "netdb.h"
#include "lwip/dns.h"

/**
 * LWIP
 */
#include "lwip/api.h"
#include "lwip/err.h"
#include <sys/socket.h>
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/ip4_addr.h"
#include "lwip/ip_addr.h"

/**
 *  mDNS lib;
 */
#include "mdns.h"

/**
 *  Config;
 */
#include "sys.h"


/**
 * Protótipos
 */
static void event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data);
static void wifi_init_sta( void );
void app_main( void );
static void http_server( void *pvParameters );

/**
 * Variáveis
 */
static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;
static const char *TAG = "main: ";


/**
 * Task Responsável pela configuração do servidor Web porta 80;
 * https://controllerstech.com/how-to-use-stm32-as-tcp-server-and-client-using-netconn/
 * https://lwip.fandom.com/wiki/Netconn_connect
 */
static void http_server( void *pvParameters ) 
{
	char msgc[100];
	struct netconn *conn;
	err_t err, connect_error;
    struct ip_addr dest_addr; 
	
	for(;;)
	{

		/**
		 * Aguarda a conexão do ESP32 a rede WiFi local;
		 */
		xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);	
	
		/**
		 * Associa o endereço IP do ESP32 com a porta 80;
		 */
		conn = netconn_new( NETCONN_TCP );
		if (conn == NULL) { 
		 /* No memory for new connection? */
		 continue;
		}
			
		err = netconn_bind(conn, IP_ADDR_ANY, 0);
		
		if (err == ERR_OK)
		{
			//dest_addr.addr = MQTT_IP_ADDRESS; //IP_ADDR4(&dest_addr, 192, 168, 0, 2);
			IP_ADDR4(&dest_addr, 10, 10, 0, 94);
			connect_error = netconn_connect(conn, &dest_addr, MQTT_PORT);
			if (connect_error == ERR_OK)
			{
				struct netbuf *inbuf;
				char *buf;
				u16_t buflen;
				err_t err;

				/**
				 * Recebe o request HTTP do client;
				 * conn é o handle da conexão e inbuf é a struct
				 * que internamente contém o buffer;
				 https://openlabpro.com/guide/mqtt-packet-format/
				 https://docs.solace.com/API/MQTT-311-Prtl-Conformance-Spec/MQTT%20Control%20Packets.htm#_Ref363648298
				 https://www.emqx.com/en/blog/use-of-mqtt-will-message
				 */
				msgc[ 0] = 0X10;
				msgc[ 1] = 0X11;
				msgc[ 2] = 0X00;
				msgc[ 3] = 0X04;
				msgc[ 4] = 'M';
				msgc[ 5] = 'Q';
				msgc[ 6] = 'T';
				msgc[ 7] = 'T';
				msgc[ 8] = 0x04;
				msgc[ 9] = 0x02;//2
				msgc[10] = 0x00;
				msgc[11] = 0x3C;
				msgc[12] = 0x00;
				msgc[13] = 0x05;
				msgc[14] = 'P';
				msgc[15] = 'Q';
				msgc[16] = 'R';
				msgc[17] = 'S';
				msgc[18] = 'T';
				
				netconn_write(conn, msgc, 19, NETCONN_COPY);
				
				err = netconn_recv(conn, &inbuf);
				if (err == ERR_OK) {
				  
					/**
					 * Os dados recebidos são apontados por buf. 
					 * Em buflen é carregado a quantidade de bytes que foram
					 * recebidos e armazenados em buf; 
					 */
					netbuf_data(inbuf, (void**)&buf, &buflen);
					ESP_LOGI(TAG, "len: %d\n", buflen);
					ESP_LOG_BUFFER_HEX(TAG, buf, buflen); 	

					//resposta:  20 02 00 00
					/*
						20 -> CONNACK Packet fixed header https://docs.solace.com/API/MQTT-311-Prtl-Conformance-Spec/MQTT%20Control%20Packets.htm#_Figure_3.8_%E2%80%93
						
						02 -> Remaining Length (2) -> quantidade de bytes do 'Variable Header';
						00 -> Connect Acknowledge Flags (7-1) reservado | LSB bit (Session Present);
						https://docs.solace.com/API/MQTT-311-Prtl-Conformance-Spec/MQTT%20Control%20Packets.htm#_Table_3.1_-
					
					*/
					
					if(buflen == 4 && buf[0] == 0x20 && buf[3] == 0x00 )  //0x00 Connection Accepted
					{
						ESP_LOGI(TAG, "PUBLISH");
						msgc[ 0] = 0X30;
						msgc[ 1] = 0X0B;
						msgc[ 2] = 0X00;
						msgc[ 3] = 0X04;
						msgc[ 4] = 'T';
						msgc[ 5] = 'E';
						msgc[ 6] = 'M';
						msgc[ 7] = 'P'; 

						msgc[8] = 'H';
						msgc[9] = 'E';
						msgc[10] = 'L';
						msgc[11] = 'L';
						msgc[12] = '0';	

						//https://doc.ecoscentric.com/ref/lwip-api-sequential-netconn-write.html
						netconn_write(conn, msgc, 13, NETCONN_COPY);
						//err = netconn_recv(conn, &inbuf);
						if (err == ERR_OK) {
							//netbuf_data(inbuf, (void**)&buf, &buflen);
							//ESP_LOG_BUFFER_HEX(TAG, buf, buflen); 	

							//SUBSCRIBE
							ESP_LOGI(TAG, "SUBSCRIBE");
							msgc[ 0] = 0X82;
							msgc[ 1] = 0X08;
							msgc[ 2] = 0X00;
							msgc[ 3] = 0X01;
							msgc[ 4] = 0X00;
							msgc[ 5] = 0X03;
							msgc[ 6] = 'M'; //4D 50 42
							msgc[ 7] = 'P';
							msgc[ 8] = 'B';
							msgc[ 9] = 0x00;
							netconn_write(conn, msgc, 10, NETCONN_COPY);
							err = netconn_recv(conn, &inbuf);
							if (err == ERR_OK) {
								//SUBACK
								netbuf_data(inbuf, (void**)&buf, &buflen);
								ESP_LOGI(TAG, "SUBACK");
								ESP_LOG_BUFFER_HEX(TAG, buf, buflen); 
								
								//resposta -> 90 03 00 01 00
								/*
								   90 -> SUBACK
								   03 -> Remaining Length field
								   00 01 -> packet id 
								   00 -> 0x00 - Success - Maximum QoS 0
								*/
								
								err = netconn_recv(conn, &inbuf);
								if (err == ERR_OK) {
									netbuf_data(inbuf, (void**)&buf, &buflen);
									ESP_LOGI(TAG, "Publish message Server-client");
									ESP_LOG_BUFFER_HEX(TAG, buf, buflen);
									
									//resposta -> 30 0c 00 03 4d 50 42 61 62 61 63 61 78 69
									/*
									   30 -> Publish
									   0c -> variable length + payload length
									   00 03 -> topic name length
									   4d 50 42 -> "MPB"
									   61 62 61 63 61 78 69 -> "abacaxi"
									*/	
									
									//ESP_LOGI(TAG, "PUBACK"); //QoS > 1
									//msgc[ 0] = 0X40;
									//msgc[ 1] = 0X02;
									//msgc[ 2] = 0X00;
									//msgc[ 3] = 0X01;
									//netconn_write(conn, msgc, 4, NETCONN_COPY);
									
									vTaskDelay( 5000/portTICK_PERIOD_MS );	

									int t = 1;
								    do {
										//3.12 PINGREQ – PING request
										ESP_LOGI(TAG, "PING");
										msgc[ 0] = 0XC0;
										msgc[ 1] = 0X00;
										netconn_write(conn, msgc, 2, NETCONN_COPY);
										
										err = netconn_recv(conn, &inbuf);
										if (err == ERR_OK) {
											netbuf_data(inbuf, (void**)&buf, &buflen);
											ESP_LOG_BUFFER_HEX(TAG, buf, buflen);
											//resposta -> d0 00
											/*
											   d0 -> PING response
											   00 -> Remaining Length (0)
											*/												
										}

										vTaskDelay(5000/portTICK_PERIOD_MS );										
									} while(t--);
									
									//SUBSCRIBE
									ESP_LOGI(TAG, "UNSUBSCRIBE");
									msgc[ 0] = 0XA2;
									msgc[ 1] = 0X07;
									msgc[ 2] = 0X00;
									msgc[ 3] = 0X01;
									msgc[ 4] = 0X00;
									msgc[ 5] = 0X03;
									msgc[ 6] = 'M'; //4D 50 42
									msgc[ 7] = 'P';
									msgc[ 8] = 'B';
			
									netconn_write(conn, msgc, 9, NETCONN_COPY);									
									err = netconn_recv(conn, &inbuf);
									if (err == ERR_OK) {
										netbuf_data(inbuf, (void**)&buf, &buflen);
										ESP_LOG_BUFFER_HEX(TAG, buf, buflen);
										//resposta -> b0 02 00 01
										/*
										   B0 -> PING response
										   02 -> Remaining Length (0)
										   00 01 -> Packet Identifier
										*/	

										//SUBSCRIBE
										ESP_LOGI(TAG, "DISCONNECT");
										msgc[ 0] = 0XE0;
										msgc[ 1] = 0X00;				
										netconn_write(conn, msgc, 2, NETCONN_COPY);	
										
										netconn_close(conn);
										netconn_delete(conn);
										goto exit;
										
									}	
									
								}
							}
							
						}
					}
				}
				
				for(;;) { vTaskDelay( 5000/portTICK_PERIOD_MS ); }  //10s
				
				netbuf_delete(inbuf);								
			}

			netconn_close(conn);
			netconn_delete(conn);			
			
		} else {
			netconn_delete(conn);
		}

exit:		
		ESP_LOGI(TAG, "Conexão Socket Encerrada: Handle");
        vTaskDelay( 5000/portTICK_PERIOD_MS );  //10s
    }

    /**
     * Em teoria, esta linha nunca será executada...
     */
	vTaskDelete(NULL); 
}

/**
 * Função de callback chamada quando é alterado o estado da
 * conexão WiFi;
 */
static void event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
	{
        esp_wifi_connect();
    } else 
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
	{
        esp_wifi_connect();
        xEventGroupClearBits( wifi_event_group, WIFI_CONNECTED_BIT );
    }else 
	if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
	{
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));   
		xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta( void )
{
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_t *netif_sta = esp_netif_create_default_wifi_sta();
    /**
     * Deseja trabalhar com o endereço IP fixo na rede? 
     */		
#if IP_FIXO
    esp_netif_dhcpc_stop(netif_sta);
    esp_netif_ip_info_t ip_info;

	IP4_ADDR(&ip_info.ip,      192,168,0,222 );
	IP4_ADDR(&ip_info.gw,      192,168,0,1   );
	IP4_ADDR(&ip_info.netmask, 255,255,255,0 );
                                    
    esp_netif_set_ip_info(netif_sta, &ip_info);

	/**
		I referred from here.
		https://www.esp32.com/viewtopic.php?t=5380
		if we should not be using DHCP (for example we are using static IP addresses),
		then we need to instruct the ESP32 of the locations of the DNS servers manually.
		Google publicly makes available two name servers with the addresses of 8.8.8.8 and 8.8.4.4.
	*/
	
	ip_addr_t d;
	d.type = IPADDR_TYPE_V4;
	d.u_addr.ip4.addr = 0x08080808; //8.8.8.8 dns
	dns_setserver(0, &d);
	d.u_addr.ip4.addr = 0x08080404; //8.8.4.4 dns
	dns_setserver(1, &d);
#else 
	(void)netif_sta;
#endif

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

	ESP_LOGI(TAG, "wifi_init_sta finished.");
	ESP_LOGI(TAG, "connect to ap SSID:%s password:%s", ESP_WIFI_SSID, ESP_WIFI_PASS);
}


void app_main(void)
{
    /**
		Inicialização da memória não volátil para armazenamento de dados (Non-volatile storage (NVS)).
		Necessário para realização da calibração do PHY. 
	*/
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
	
	/**
	   Event Group do FreeRTOS. 
	   Só podemos enviar ou ler alguma informação TCP quando a rede WiFi 
	   estiver configurada, ou seja, somente após o aceite de conexão e a 
	   liberação do IP pelo roteador da rede (nos casos de IPs dinâmicos).
	*/
	wifi_event_group = xEventGroupCreate();
	
	/**
	  Configura a rede WiFi.
	*/
    wifi_init_sta();
	
	/**
	 * Cria a task responsável em criar o socket tcp server; 
	 */
	if( xTaskCreate( http_server, "http_server", 1024*10, NULL, 5, NULL ) != pdTRUE )
	{
		if( DEBUG )
			ESP_LOGI(TAG, " http_server error " );
	}
}

