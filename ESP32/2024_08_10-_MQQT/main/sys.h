#ifndef SYS_H__
#define SYS_H__
/**
 * Configuração de Rede;
 */
#define IP_FIXO 0

/**
 *  https://www.ibm.com/docs/en/snips/4.6.0?topic=uzcn-using-bonjour-from-windows-command-line-discover-services
 *  dns-sd -B _http._tcp
 *  dns-sd -L "ESP32-WebServer" _http._tcp
 */
#define ESP_WIFI_SSID "FSimplicio_2.4G"
#define ESP_WIFI_PASS "fsimpliciokzz5"	


#define MQTT_IP_ADDRESS "10.10.0.94"
#define MQTT_PORT 1883

#define DEBUG 1									
#endif