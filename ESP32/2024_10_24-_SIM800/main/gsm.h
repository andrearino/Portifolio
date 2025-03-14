#ifndef __GSM_H__
#define __GSM_H__

#define MAX_SIZE_AT_COMMAND 200
#define MAX_SIZE_AT_OK_RESPONSE 50
#define NUM_MAX_RETRANSMISSAO_CMD_AT 2

typedef struct
{
	char		cmd[MAX_SIZE_AT_COMMAND];
	uint16_t	cmd_size;
	char		cmd_response[MAX_SIZE_AT_OK_RESPONSE];
	uint16_t	timeout_ms;
	uint16_t	delay_ms;
	uint8_t		skip;
} Gsm_cmd_t;


typedef enum 
{
	GSM_STATE_DISCONNECTED, //0
	GSM_STATE_CONNECTED,    //1
	GSM_STATE_IDLE,         //2
	GSM_STATE_FIRSTINIT     //3
} Gsm_status_t; 


extern esp_err_t gsm_init( void );
extern int gsm_config(void);
extern int gsm_write_command(char * cmd, int cmd_size, char * ret, int timeout, char **resp, int resp_size); 
extern int pppos_init(void);
extern int pppos_is_connected(void);
extern EventGroupHandle_t pppos_event_group;
extern const int PPPOS_CONNECTED_BIT;
#endif