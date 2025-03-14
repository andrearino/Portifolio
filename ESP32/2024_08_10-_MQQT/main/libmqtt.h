#ifndef __LIB_MQTT_H__
#define __LIB_MQTT_H__

#ifdef __cplusplus
extern "C" {
#endif

enum 
{
	RESERVED1=1,/*!< Reserved */
	CONNECT,	/*!< Requisição do cliente para conexão com o servidor */ 
	CONNACK, 	/*!< Connect Acknowledgment */ 
	PUBLISH, 	/*!< Publish message */ 
	PUBACK, 	/*!< Publish Acknowledgment */ 
	PUBREC, 	/*!< Publish received */ 
	PUBREL, 	/*!< Publish release */ 
	PUBCOMP, 	/*!< Publish complete */ 
	SUBSCRIBE,  /*!< Client Subscribe complete */ 
	SUBACK,     /*!< Subscribe Acknowledgment */ 
	UNSUBSCRIBE,/*!< Client Unsubscribe request */  
	UNSUBACK, 	/*!< Unsubscribe Acknowledgment */
	PINGREQ, 	/*!< Ping request */
	PINGRESP, 	/*!< Ping response */
	DISCONNECT	/*!< Client is disconnecting */
	RESERVED2   /*!< Reserved */
}; 

typedef struct f_mqtt 
{
	uint8_t control_header; 
	uint8_t remaining_length;
	uint8_t * variable_header; 
	uint8_t * payload;
} mqtt_t;

#ifdef __cplusplus
}
#endif

#endif /*_LIB_MQTT_H_*/