/*
Andre Arino
Data: 23/02/2024 - Ver. 1.0
Estudo: ESP32
Objetivo: Trabalhar com interface I2C, RTC e LCD 16x2
Placa: ESP32 WROOM DEVKIT V1 DOIT

Conexões DS1307:
GPIO22 -> SCL
GPIO21 -> SDA

ID_RTC 0x68

Conexões PCF8574:
GPIO22 -> SCL
GPIO21 -> SDA

Saída do PCF8574
P0 -> RS
P1 -> RW
P2 -> EN
P3 -> BT
P4 -> D4
P5 -> D5
P6 -> D6
P7 -> D7

A0, A1 e A2 ligados a VCC = ID_LCD 0x27
B7: data bit 3
B6: data bit 2
B5: data bit 1
B4: data bit 0
B3: backlight (BL): Desligado = 0, Ligado = 1
B2: enable (EN): Mudança de 1 para 0 para clock de dados no controlador
B1: read/write (RW): Escrita = 0, Leitura = 1
B0: register select (RS): Comando = 0, Dados = 1
*/

/*Biblioteca padrão C*/
#include <stdio.h>
#include <string.h>

/*Bliblioteca para IOs do ESP*/
#include <driver/gpio.h>
#include <driver/i2c.h>

/*Blibliotecas FreeRTOS*/
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

/*Bibliotecas Criadas*/

/*Definições*/
#define I2C_MASTER_SCL_IO 22      // GPIO do I2C master clock
#define I2C_MASTER_SDA_IO 21      // GPIO do I2C master data
#define I2C_MASTER_NUM I2C_NUM_1  // I2C número do port para master dev
#define I2C_MASTER_FREQ_HZ 100000 // Frequencia do I2C master clock

/*Handles*/
SemaphoreHandle_t xSemaphoreMutex; // Cria Handle para Semáforo Mutex
QueueHandle_t xQueueSec; // Cria fila para transferência de valores entre tarefas RTC_Read e LCD
QueueHandle_t xQueueMin; // Cria fila para transferência de valores entre tarefas RTC_Read e LCD
QueueHandle_t xQueueHour; // Cria fila para transferência de valores entre tarefas RTC_Read e LCD
QueueHandle_t xQueueWeek; // Cria fila para transferência de valores entre tarefas RTC_Read e LCD
QueueHandle_t xQueueDay; // Cria fila para transferência de valores entre tarefas RTC_Read e LCD
QueueHandle_t xQueueMon; // Cria fila para transferência de valores entre tarefas RTC_Read e LCD
QueueHandle_t xQueueYear; // Cria fila para transferência de valores entre tarefas RTC_Read e LCD
TaskHandle_t xTaskI2CHandle; // Cria Handle para TaskI2C
TaskHandle_t xTaskRTC_ConfigHandle; // Cria Handle para TaskRTC_Config
TaskHandle_t xTaskRTC_ReadHandle; // Cria Handle para TaskRTC_Read
TaskHandle_t xTaskDisplayHandle; // Cria Handle para TaskDisplay

// Variável Global.
uint8_t ID_LCD = 0x27; // Carrega ID do PCF8574
uint8_t ID_RTC = 0x68; // Carrega ID do DS1307
uint8_t data; // Variável global para uso na gravação de comandos e dados do LCD

// Descritor

/*Protótipos*/

// I2C
void I2C_Write(uint8_t I2C_ID, uint8_t Data, uint8_t TimeOut); // cria função I2C_Write que recebe parâmetros:
// Endereço do RTC, dado a ser escrito e tempo de espera
void I2C_Write_Add(uint8_t I2C_ID, uint8_t Address, uint8_t Data);// cria função I2C_Write que recebe parâmetros:
// Endereço do RTC, Endereço da função mapeada na memória e dado a ser escrito
uint8_t I2C_Read(uint8_t I2C_ID, uint8_t Address); // cria função I2C_Read que retorna dado e recebe parâmetros:
// Endereço do RTC, Endereço da função mapeada na memória

// LCD
void LCD_Init();                                                          // Função de Inicialização do LCD
void LCD_Command(u_int8_t cmd);                                           // Função para envio de comandos ao LCD
void LCD_Clear();                                                         // Limpar o LCD
void LCD_Hex_Char(unsigned char CHAR);                                    // Escrita de dados tipo char no LCD
void LCD_String(char str[]);                                              // Função para passar dados em ponteiro unsigned char 8 bits
void LCD_String_xy(uint8_t line, uint8_t position, char str[]);           // Envia string para o LCD com a posição xy
void LCD_Hex_Char_xy(uint8_t line, uint8_t position, unsigned char CHAR); // Envia um char para o LCD com a posição xy

// RTC
void RTC_Set (uint8_t sec, uint8_t min, uint8_t hour, uint8_t week, uint8_t day, uint8_t mon, uint8_t year); // Função para ajuste do relógio
uint8_t RTC_Read_Year();
uint8_t RTC_Read_Mon();
uint8_t RTC_Read_Day();
uint8_t RTC_Read_Week();
uint8_t RTC_Read_Hour();
uint8_t RTC_Read_Min();
uint8_t RTC_Read_Sec();

// Tasks
void vTaskDisplay(void *pvParameters); // cria tarefa vTaskDisplay que esqueve no display
void vTaskRTC_Config(void *pvParameters); // cria tarefa VTaskLED que recebe parâmetro não identificado via ponteiro
void vTaskRTC_Read(void *pvParameters); // cria tarefa VTaskOLED que recebe parâmetro não identificado via ponteiro

/*Main do Sistema FreeRTOS*/
void app_main(void)
{
    /*Inicialização do I2C*/
    i2c_config_t conf;                               // Cria Estrutura para i2c_config_t
    conf.mode = I2C_MODE_MASTER;                     // Seleção de modo ESP32 como MASTER
    conf.sda_io_num = (gpio_num_t)I2C_MASTER_SDA_IO; // SDA definido como pino 21
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;         // Habilitado modo PullUp para bus SDA
    conf.scl_io_num = (gpio_num_t)I2C_MASTER_SCL_IO; // SCL definido como pino 22
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;         // Habilitado modo PullUp para bus SDA
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;      // Chama definição de frequência
    conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;    // Definição da fonte de Clock

    BaseType_t xReturned1; // Variável para verificar a configuração e instalação do drive i2c

    xReturned1 = i2c_param_config(I2C_MASTER_NUM, &conf); // Configura I2C, passando número do port 1
    // e estrutura de configuração
    if (xReturned1 == ESP_ERR_INVALID_ARG) // Verifica criação da tarefa
    {
        printf("Não foi possível configurar I2C. \n"); // Comunica que a task não foi criada
        while (1)
        {
            vTaskDelay(pdMS_TO_TICKS(1000)); // Trava o programa
        }
    }
    xReturned1 = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0); // Instala drive, passando
    // número do port, modo, buffer de recebimento e envio e flag
    if (xReturned1 == ESP_ERR_INVALID_ARG) // Verifica criação da tarefa
    {
        printf("Não foi possível criar Drive I2C. \n"); // Comunica que a task não foi criada
        while (1)
        {
            vTaskDelay(pdMS_TO_TICKS(1000)); // Trava o programa
        }
    }

    /*Inicialização do LCD*/
    LCD_Init();

    //Inicialização do Semáforo
    xSemaphoreMutex = xSemaphoreCreateMutex();
    if(xSemaphoreMutex == NULL) // Verifica se semáforo é igual a zero
    {
        printf("Não foi possível criar semáforo"); // Comunica que semáforo não foi criado
        while (1)
        {
            vTaskDelay(pdMS_TO_TICKS(1000)); // Trava o programa
        }  
    }

    xQueueSec = xQueueCreate(5,sizeof(int)); // Cria Fila xQueue
    if(xQueueSec == NULL) // Verifica se a fila é igual a zero
    {
        while (1);
    }    

    xQueueMin = xQueueCreate(1,sizeof(int)); // Cria Fila xQueue
    if(xQueueMin == NULL) // Verifica se a fila é igual a zero
    {
        while (1);
    }

    xQueueHour = xQueueCreate(1,sizeof(int)); // Cria Fila xQueue
    if(xQueueHour == NULL) // Verifica se a fila é igual a zero
    {
        while (1);
    }

    xQueueWeek = xQueueCreate(1,sizeof(int)); // Cria Fila xQueue
    if(xQueueWeek == NULL) // Verifica se a fila é igual a zero
    {
        while (1);
    }

    xQueueDay = xQueueCreate(1,sizeof(int)); // Cria Fila xQueue
    if(xQueueDay == NULL) // Verifica se a fila é igual a zero
    {
        while (1);
    } 

    xQueueMon = xQueueCreate(1,sizeof(int)); // Cria Fila xQueue
    if(xQueueMon == NULL) // Verifica se a fila é igual a zero
    {
        while (1);
    }

    xQueueYear = xQueueCreate(1,sizeof(int)); // Cria Fila xQueue
    if(xQueueYear == NULL) // Verifica se a fila é igual a zero
    {
        while (1);
    }


    // Inicialização das Tasks
    BaseType_t xReturned2; // Variável para verificar a criação das tarefas

    xReturned2 = xTaskCreate(vTaskDisplay, "vTaskDisplay", configMINIMAL_STACK_SIZE + 1024, NULL, 1, &xTaskDisplayHandle);
    // Verifica vTaskRTC_Config
    // com lable blink_task, tamanho de memória de 1024,
    // envia parâmetro NULL, prioridade 1
    // endereça ao Handle xTaskRTC_ConfigHandle
    if (xReturned2 == pdFAIL) // Verifica criação da tarefa
    {
        printf("Não foi possível criar a task RTC_Config. \n"); // Comunica que a task não foi criada
        while (1)
        {
            vTaskDelay(pdMS_TO_TICKS(1000)); // Trava o programa
        }
    }

    xReturned2 = xTaskCreate(vTaskRTC_Config, "vTaskRTC_Config", configMINIMAL_STACK_SIZE + 1024, NULL, 2, &xTaskRTC_ConfigHandle); 
    // Verifica vTaskRTC_Config
    // com lable blink_task, tamanho de memória de 1024,
    // envia parâmetro NULL, prioridade 1
    // endereça ao Handle xTaskRTC_ConfigHandle
    if (xReturned2 == pdFAIL) // Verifica criação da tarefa
    {
        printf("Não foi possível criar a task RTC_Config. \n"); // Comunica que a task não foi criada
        while (1)
        {
            vTaskDelay(pdMS_TO_TICKS(1000)); // Trava o programa
        }
    }

    xReturned2 = xTaskCreate(vTaskRTC_Read, "vTaskRTC_Read", configMINIMAL_STACK_SIZE + 1024, NULL, 1, &xTaskRTC_ReadHandle); 
    // Verifica vTaskRTC_Read
    // com lable blink_task, tamanho de memória de 1024,
    // envia parâmetro NULL, prioridade 1
    // endereça ao Handle xTaskRTC_ReadHandle
    if (xReturned2 == pdFAIL) // Verifica criação da tarefa
    {
        printf("Não foi possível criar a task RTC_Config. \n"); // Comunica que a task não foi criada
        while (1)
        {
            vTaskDelay(pdMS_TO_TICKS(1000)); // Trava o programa
        }
    }
}

/*Funções*/

/*Função para escrever no barramento I2C*/
void I2C_Write(uint8_t I2C_ID, uint8_t Data, uint8_t TimeOut)
{
    esp_err_t ret;                                                                    // Cria variável para checar ESP_ERR
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();                                     // Inicia I2C no Handle cmd
    ret = i2c_master_start(cmd);                                                      // Envia bit de Start
    assert(ESP_OK == ret);                                                            // pula para próxima linha se se retorno = ESP_OK
    ret = i2c_master_write_byte(cmd, I2C_ID << 1 | I2C_MASTER_WRITE, I2C_MASTER_ACK); // Envia ID + BIT W + ACK
    assert(ESP_OK == ret);                                                            // pula para próxima linha se se retorno = ESP_OK
    ret = i2c_master_write_byte(cmd, Data, I2C_MASTER_ACK);                           // Envia Dados, quantidade de Dados + ACK
    assert(ESP_OK == ret);                                                            // pula para próxima linha se se retorno = ESP_OK
    ret = i2c_master_stop(cmd);                                                       // Envia bit de parada
    assert(ESP_OK == ret);                                                            // pula para próxima linha se se retorno = ESP_OK
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(TimeOut));          // Bloqueia a tarefa I2C até o fim do Timeout
    i2c_cmd_link_delete(cmd);                                                         // Deleta Handle
    return;                                                                           // Retorna sem dados
}

/*Função para escrever no barramento I2C com endereço*/
void I2C_Write_Add(uint8_t I2C_ID, uint8_t Address, uint8_t Data)
{
    esp_err_t ret;                                                                    // Cria variável para checar ESP_ERR
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();                                     // Inicia I2C no Handle cmd
    ret = i2c_master_start(cmd);                                                      // Envia bit de Start
    assert(ESP_OK == ret);                                                            // pula para próxima linha se se retorno = ESP_OK
    ret = i2c_master_write_byte(cmd, I2C_ID << 1 | I2C_MASTER_WRITE, I2C_MASTER_ACK); // Envia ID + BIT W + ACK
    assert(ESP_OK == ret);
    ret = i2c_master_write_byte(cmd, Address, I2C_MASTER_ACK);                           // Envia Dados, quantidade de Dados + ACK
    assert(ESP_OK == ret);                                                            // pula para próxima linha se se retorno = ESP_OK
    ret = i2c_master_write_byte(cmd, Data, I2C_MASTER_ACK);                           // Envia Dados, quantidade de Dados + ACK
    assert(ESP_OK == ret);                                                            // pula para próxima linha se se retorno = ESP_OK
    ret = i2c_master_stop(cmd);                                                       // Envia bit de parada
    assert(ESP_OK == ret);                                                            // pula para próxima linha se se retorno = ESP_OK
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(10));               // Bloqueia a tarefa I2C até o fim do Timeout
    i2c_cmd_link_delete(cmd);                                                         // Deleta Handle
    return;                                                                           // Retorna sem dados
}

/*Função para ler o barramento I2C*/
uint8_t I2C_Read(uint8_t I2C_ID, uint8_t Address)
{
    uint8_t data;                                                            // Cria variável para obter os dados de leitura
    esp_err_t ret;                                                           // Cria variável para checar ESP_ERR
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();                            // Inicia I2C no Handle cmd
    ret = i2c_master_start(cmd);                                             // Envia bit de Start
    assert(ESP_OK == ret);                                                   // pula para próxima linha se se retorno = ESP_OK
    ret = i2c_master_write_byte(cmd, I2C_ID << 1 | I2C_MASTER_WRITE, true);  // Envia ID + BIT W + ACK
    assert(ESP_OK == ret);                                                   // pula para próxima linha se se retorno = ESP_OK
    ret = i2c_master_write_byte(cmd, Address, true);                         // Envia Endereço + ACK
    assert(ESP_OK == ret);                                                   // pula para próxima linha se se retorno = ESP_OK
    ret = i2c_master_start(cmd);                                             // Envia bit de Start
    assert(ESP_OK == ret);                                                   // pula para próxima linha se se retorno = ESP_OK
    ret = i2c_master_write_byte(cmd, (I2C_ID << 1) | I2C_MASTER_READ, true); // Envia ID + BIT R + ACK
    assert(ESP_OK == ret);                                                   // pula para próxima linha se se retorno = ESP_OK
    ret = i2c_master_read_byte(cmd, &data, I2C_MASTER_LAST_NACK);            // Lê retorno I2C, armazena em data e envia NACK
    assert(ESP_OK == ret);                                                   // pula para próxima linha se se retorno = ESP_OK
    ret = i2c_master_stop(cmd);                                              // Envia bit de parada
    assert(ESP_OK == ret);                                                   // pula para próxima linha se se retorno = ESP_OK
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(10));      // Bloqueia a tarefa I2C até o fim do Timeout
    if (ret != ESP_OK)                                                       // Verifica mensagem diferente de ESP_OK
        printf("Não pode ler o dispositivo \n");                             // Comunica falha ao ler dados
    i2c_cmd_link_delete(cmd);                                                // Deleta Handle
    return data;                                                             // Retorna data para a função
}

/*Função para envio de comandos à interface do LCD via I2C*/
void LCD_Command(u_int8_t cmd)
{
    //printf("Comando %02x \n", cmd);
    data = 0xFF;
    data = (data & 0xF0) & (cmd << 0); // Envia o nibble superior (comando) para D4 ~D7
    data = data & (~(1 << 0));         // Seleção de registro = 0 (Comandos)
    data = data & (~(1 << 1));         // Escrita RW
    data = data | (1 << 2);            // Habilitado EN
    data = data | (1 << 3);            // Habilita Backligh
    //printf("Com_Sup %02x \n", data);
    I2C_Write(ID_LCD, data, 0); // Envia nible superior por 2ms
    vTaskDelay(pdMS_TO_TICKS(2));
    data = data & (~(1 << 2)); // Desabilita EN
    //printf("Com_Sup %02x \n", data);
    I2C_Write(ID_LCD, data, 0); // Envia nible superior por 10ms
    vTaskDelay(pdMS_TO_TICKS(10));

    data = 0xFF;
    data = (data & 0xF0) & (cmd << 4); // Envia o nibble inferior (comando) para D4 ~D7
    data = data & (~(1 << 0));         // Seleção de registro = 0 (Comandos)
    data = data & (~(1 << 1));         // Escrita RW
    data = data | (1 << 2);            // Habilitado EN
    data = data | (1 << 3);            // Habilita Backligh
    //printf("Com_Inf %02x \n", data);
    I2C_Write(ID_LCD, data, 0); // Envia nible inferior por 2ms
    vTaskDelay(pdMS_TO_TICKS(2));
    data = data & (~(1 << 2)); // Desabilita EN
    //printf("Com_Inf %02x \n", data);
    I2C_Write(ID_LCD, data, 0); // Envia nible inferior por 10ms
    vTaskDelay(pdMS_TO_TICKS(10));
    return; // Retorna sem dados
}

void LCD_Hex_Char(unsigned char CHAR)
{
    //printf("Letra %02x \n", CHAR);
    data = 0xFF;
    data = (data & 0xF0) & (CHAR << 0); // Envia o nibble superior (Dado) para D4 ~D7
    data = data | (1 << 0);             // Seleção de registro = 1 (Dados)
    data = data & (~(1 << 1));          // Escrita RW
    data = data | (1 << 2);             // Habilitado EN
    data = data | (1 << 3);             // Habilita Backligh
    //printf("Char_Sup %02x \n", data);
    I2C_Write(ID_LCD, data, 0); // Envia nible superior por 2ms
    vTaskDelay(pdMS_TO_TICKS(2));
    data = data & (~(1 << 2));  // Desabilita EN    
    //printf("Char_Sup %02x \n", data);
    I2C_Write(ID_LCD, data, 10); // Envia nible superior por 10ms
    vTaskDelay(pdMS_TO_TICKS(10));

    data = 0xFF;
    data = (data & 0xF0) & (CHAR << 4); // Envia o nibble inferior (Dado) para D4 ~D7
    data = data | (1 << 0);             // Seleção de registro = 1 (Dados)
    data = data & (~(1 << 1));          // Escrita RW
    data = data | (1 << 2);             // Habilitado EN
    data = data | (1 << 3);             // Habilita Backligh
    //printf("Char_Inf %02x \n", data);
    I2C_Write(ID_LCD, data, 0); // Envia nible inferior por 2ms
    vTaskDelay(pdMS_TO_TICKS(2));
    data = data & (~(1 << 2)); // Desabilita EN
    //printf("Char_Inf %02x \n", data);
    I2C_Write(ID_LCD, data, 10); // Envia nible inferior por 10ms
    vTaskDelay(pdMS_TO_TICKS(10));
    return; // Retorna sem dados
}

void LCD_String(char str[]) // Função para passar dados em ponteiro char array
{
    int i = 0;          // cria veriável i para contador do ponteiro
    while (str[i] != 0) // aguarda que dado do ponteiro seja NULL
    {
        LCD_Hex_Char(str[i]); // Envia caractere até caractere NULL
        i++;                  // incrementa para próximo dado do ponteiro
    }
    return; // Retorna sem dados
}

void LCD_String_xy(uint8_t line, uint8_t position, char str[]) // Envia string para o LCD com a posição xy
{
    if (line == 0 && position < 16)            // analisa de a entrada de foi 0 e coluna menor que 16
        LCD_Command((position & 0x0F) | 0x80); // Comando da primeira linha e posição desejada menor que 16
    else if (line == 1 && position < 16)       // analisa de a entrada de foi 1 e coluna menor que 16
        LCD_Command((position & 0x0F) | 0xC0); // Comando da segunda linha e posição desejada menor que 16
    LCD_String(str);                           // Chamada da função LCD_String
    return;                                    // Retorna sem dados
}

void LCD_Hex_Char_xy(uint8_t line, uint8_t position, unsigned char CHAR) // Envia um char para o LCD com a posição xy
{
    if (line == 0 && position < 16)            // analisa de a entrada de foi 0 e coluna menor que 16
        LCD_Command((position & 0x0F) | 0x80); // Comando da primeira linha e posição desejada menor que 16
    else if (line == 1 && position < 16)       // analisa de a entrada de foi 1 e coluna menor que 16
        LCD_Command((position & 0x0F) | 0xC0); // Comando da segunda linha e posição desejada menor que 16
    LCD_Hex_Char(CHAR);                        // Chamada da função Hex_Char
}

void LCD_Init()
{
    vTaskDelay(pdMS_TO_TICKS(20)); // Aguarda 20ms após power on, conforme datasheet

    LCD_Command(0X02);            // Define operação em 4 bits
    vTaskDelay(pdMS_TO_TICKS(1)); // Aguarda 1ms

    LCD_Command(0X28);            // Cursor incrementando em 1 e deslocamento para direita
    vTaskDelay(pdMS_TO_TICKS(1)); // Aguarda 1ms

    LCD_Command(0X0C);            // Cursor Desligado
    vTaskDelay(pdMS_TO_TICKS(1)); // Aguarda 1ms

    LCD_Command(0X06);            // Define o modo para incrementar o endereço e deslocar o cursor para a direita
    vTaskDelay(pdMS_TO_TICKS(1)); // Aguarda 1ms

    LCD_Command(0X01);            // limpa o display
    vTaskDelay(pdMS_TO_TICKS(4)); // Aguarda 4ms
}

void LCD_Clear()
{
    LCD_Command(0x01);            // Limpa o Display
    vTaskDelay(pdMS_TO_TICKS(4)); // Aguarda 4ms
    LCD_Command(0x80);            // Desloca display para posição inicial
    vTaskDelay(pdMS_TO_TICKS(1)); // Aguarda 1ms
}

void vTaskDisplay(void *pvParameters)
{
    // Monta formato do display
    xSemaphoreTake(xSemaphoreMutex,portMAX_DELAY);
    LCD_Hex_Char_xy(0,3,0x3A); // Insere caractere : na coluna 3 da linha 0
    LCD_Hex_Char_xy(0,6,0x3A); // Insere caractere : na coluna 6 da linha 0
    LCD_Hex_Char_xy(0,10,0x2D); // Insere caractere - na coluna 10 da linha 0
    LCD_Hex_Char_xy(1,3,0x2F); // Insere caractere / na coluna 3 da linha 1
    LCD_Hex_Char_xy(1,6,0x2F); // Insere caractere / na coluna 6 da linha 1
    LCD_Hex_Char_xy(1,7,0x32); // Insere caractere 2 na coluna 7 da linha 1
    LCD_Hex_Char_xy(1,8,0x30); // Insere caractere 0 na coluna 8 da linha 1
    xSemaphoreGive(xSemaphoreMutex);
    unsigned char DATA;
    unsigned char DATA_LCD0;
    unsigned char DATA_LCD1;
    uint8_t *Receive_Value = 0;
    while (1)
    {
        printf("início\n");
        if(xQueueReceive(xQueueSec,&Receive_Value,pdMS_TO_TICKS(500))==pdTRUE)
        {
            xSemaphoreTake(xSemaphoreMutex,100 / portTICK_PERIOD_MS);
            DATA = Receive_Value;
            DATA_LCD0 = ((DATA & 0x0F) + 0x30); // Seleciona o primeiro nible e soma 0x30 para transformar o valor hexa em ASCII
            DATA_LCD1 = ((DATA>>4) + 0x30); // Rotaciona o segundo nible para os primeiro nible e soma 0x30 para transformar o valor hexa em ASCII
            LCD_Hex_Char_xy(0,8,DATA_LCD0); // Insere unidade de segundo na coluna 8 da linha 0
            LCD_Hex_Char_xy(0,7,DATA_LCD1); // Insere dezena de segundo na coluna 7 da linha 0
            xSemaphoreGive(xSemaphoreMutex);

        }
        if(xQueueReceive(xQueueMin,&Receive_Value,pdMS_TO_TICKS(500))==pdTRUE)
        {
            xSemaphoreTake(xSemaphoreMutex,100 / portTICK_PERIOD_MS);
            DATA = Receive_Value;
            DATA_LCD0 = ((DATA & 0x0F) + 0x30); // Seleciona o primeiro nible e soma 0x30 para transformar o valor hexa em ASCII
            DATA_LCD1 = ((DATA>>4) + 0x30); // Rotaciona o segundo nible para os primeiro nible e soma 0x30 para transformar o valor hexa em ASCII
            LCD_Hex_Char_xy(0,5,DATA_LCD0); // Insere unidade de munito na coluna 5 da linha 0
            LCD_Hex_Char_xy(0,4,DATA_LCD1); // Insere dezena de munito na coluna 4 da linha 0
            xSemaphoreGive(xSemaphoreMutex);
        }
        if(xQueueReceive(xQueueHour,&Receive_Value,pdMS_TO_TICKS(500))==pdTRUE)
        {
            xSemaphoreTake(xSemaphoreMutex,100 / portTICK_PERIOD_MS);
            DATA = Receive_Value;
            DATA_LCD0 = ((DATA & 0x0F) + 0x30); // Seleciona o primeiro nible e soma 0x30 para transformar o valor hexa em ASCII
            DATA_LCD1 = ((DATA>>4) + 0x30); // Rotaciona o segundo nible para os primeiro nible e soma 0x30 para transformar o valor hexa em ASCII
            LCD_Hex_Char_xy(0,2,DATA_LCD0); // Insere unidade de hora na coluna 2 da linha 0
            LCD_Hex_Char_xy(0,1,DATA_LCD1); // Insere dezena de hora na coluna 1 da linha 0
            xSemaphoreGive(xSemaphoreMutex);
        }
        if(xQueueReceive(xQueueWeek,&Receive_Value,pdMS_TO_TICKS(500))==pdTRUE)
        {
            xSemaphoreTake(xSemaphoreMutex,100 / portTICK_PERIOD_MS);
            DATA = Receive_Value;
            unsigned char DAY = (DATA & 0x07); // recebe dado na variável DAY
            switch(DAY) // cria rotinas para valores da variável DAY
            {
                case 0x01: // em caso de DAY = 1
                LCD_String_xy(0,12,"DOM"); // Escreve DOM (Domingo) na posição 12
                break; // sai da rotina
                case 0x02: // em caso de DAY = 2
                LCD_String_xy(0,12,"SEG"); // Escreve SEG (Segunda) na posição 12
                break; // sai da rotina
                case 0x03: // em caso de DAY = 3
                LCD_String_xy(0,12,"TER"); // Escreve TER (Terça) na posição 12
                break; // sai da rotina
                case 0x04: // em caso de DAY = 4
                LCD_String_xy(0,12,"QUA"); // Escreve QUA (Quarta) na posição 12
                break; // sai da rotina
                case 0x05: // em caso de DAY = 5
                LCD_String_xy(0,12,"QUI"); // Escreve QUI (Quinta) na posição 12
                break; // sai da rotina
                case 0x06: // em caso de DAY = 6
                LCD_String_xy(0,12,"SEX"); // Escreve SEX (Sexta) na posição 12
                break; // sai da rotina
                case 0x07: // em caso de DAY = 7
                LCD_String_xy(0,12,"SAB"); // Escreve SAB (Sábado) na posição 12
                break; // sai da rotina
            }
            xSemaphoreGive(xSemaphoreMutex);
        }
        if(xQueueReceive(xQueueYear,&Receive_Value,pdMS_TO_TICKS(500))==pdTRUE)
        {
            xSemaphoreTake(xSemaphoreMutex,100 / portTICK_PERIOD_MS);
            DATA = Receive_Value;
            DATA_LCD0 = ((DATA & 0x0F) + 0x30); // Seleciona o primeiro nible e soma 0x30 para transformar o valor hexa em ASCII
            DATA_LCD1 = ((DATA>>4) + 0x30); // Rotaciona o segundo nible para os primeiro nible e soma 0x30 para transformar o valor hexa em ASCII
            LCD_Hex_Char_xy(1,10,DATA_LCD0); // Insere unidade de dias 
            LCD_Hex_Char_xy(1,9,DATA_LCD1); // Insere dezena de dias 
            xSemaphoreGive(xSemaphoreMutex);
        }
        if(xQueueReceive(xQueueMon,&Receive_Value,pdMS_TO_TICKS(500))==pdTRUE)
        {
            xSemaphoreTake(xSemaphoreMutex,100 / portTICK_PERIOD_MS);
            DATA = Receive_Value;
            DATA_LCD0 = ((DATA & 0x0F) + 0x30); // Seleciona o primeiro nible e soma 0x30 para transformar o valor hexa em ASCII
            DATA_LCD1 = ((DATA>>4) + 0x30); // Rotaciona o segundo nible para os primeiro nible e soma 0x30 para transformar o valor hexa em ASCII
            LCD_Hex_Char_xy(1,5,DATA_LCD0); // Insere unidade de mês
            LCD_Hex_Char_xy(1,4,DATA_LCD1); // Insere dezena de mês 
            xSemaphoreGive(xSemaphoreMutex);
        }
        if(xQueueReceive(xQueueDay,&Receive_Value,pdMS_TO_TICKS(500))==pdTRUE)
        {
            xSemaphoreTake(xSemaphoreMutex,100 / portTICK_PERIOD_MS);
            DATA = Receive_Value;
            DATA_LCD0 = ((DATA & 0x0F) + 0x30); // Seleciona o primeiro nible e soma 0x30 para transformar o valor hexa em ASCII
            DATA_LCD1 = ((DATA>>4) + 0x30); // Rotaciona o segundo nible para os primeiro nible e soma 0x30 para transformar o valor hexa em ASCII
            LCD_Hex_Char_xy(1,2,DATA_LCD0); // Insere unidade de Ano na coluna 2 da linha 0
            LCD_Hex_Char_xy(1,1,DATA_LCD1); // Insere dezena de Ano na coluna 1 da linha 0
            xSemaphoreGive(xSemaphoreMutex);
        }        
    }
}

/*Tarefa para ajustar o relógio*/
void vTaskRTC_Config(void *pvParameters)
{
    while (1)
    {
        xSemaphoreTake(xSemaphoreMutex,portMAX_DELAY);
        //printf("Inicia configuração do RTC \n"); // Informa Início da configuração do relógio
        //RTC_Set(0x00,0x57,0x04,0x07,0x24,0x02,0x24);
        //printf("Termina configuração do RTC \n"); // Informa fim da configuração do relógio   
        xSemaphoreGive(xSemaphoreMutex); // Libera o semáforo
        vTaskSuspend(xTaskRTC_ConfigHandle); // Suspende Task
    }
}

/*Tarefa para ler o relógio*/
void vTaskRTC_Read(void *pvParameters)
{
    uint8_t SEC = 0; // Cria variável para segundos
    uint8_t MIN = 0; // Cria variável para minutos
    uint8_t HOUR = 0; // Cria variável para horas
    uint8_t WEEK = 0; // Cria variável para obter retorno do dia da semana
    char WEEK_[5]; // Cria variável para guardar o dia da semana em char[]    
    uint8_t DAY = 0; // Cria variável para dia
    uint8_t MON = 0; // Cria variável para mês
    uint8_t YEAR = 0; // Cria variável para ano
     
    while (1)
    {
        xSemaphoreTake(xSemaphoreMutex,portMAX_DELAY);
        printf("Inicia leitura do RTC \n"); // Imprimi o início da leitura do Relógio
        SEC = RTC_Read_Sec();
        
        MIN = RTC_Read_Min();
        
        HOUR = RTC_Read_Hour(); 
        
        WEEK = RTC_Read_Week();
        
        switch (WEEK) // cria rotinas para valores da variável WEEK
        {
        case 0x01:                      // em caso de WEEK = 1
            sprintf(WEEK_,"DOM");       // Grava dia da semana em WEEK_
        
            break;                      // sai da rotina
        case 0x02:                      // em caso de WEEK = 2
            sprintf(WEEK_,"SEG");       // Grava dia da semana em WEEK_
        
            break;                      // sai da rotina
        case 0x03:                      // em caso de WEEK = 3
            sprintf(WEEK_,"TER");       // Grava dia da semana em WEEK_
        
            break;                      // sai da rotina
        case 0x04:                      // em caso de WEEK = 4
            sprintf(WEEK_,"QUA");       // Grava dia da semana em WEEK_
        
            break;                      // sai da rotina
        case 0x05:                      // em caso de WEEK = 5
            sprintf(WEEK_,"QUI");       // Grava dia da semana em WEEK_
        
            break;                      // sai da rotina
        case 0x06:                      // em caso de WEEK = 6
            sprintf(WEEK_,"SEX");       // Grava dia da semana em WEEK_
        
            break;                      // sai da rotina
        case 0x07:                      // em caso de WEEK = 7
            sprintf(WEEK_,"SAB");       // Grava dia da semana em WEEK_
        
            break;                      // sai da rotina
        }

        DAY = RTC_Read_Day();
        
        MON = RTC_Read_Mon();
        
        YEAR = RTC_Read_Year();

        xQueueSend(xQueueSec,&SEC,pdMS_TO_TICKS(100));
        xQueueSend(xQueueMin,&MIN,pdMS_TO_TICKS(100));
        xQueueSend(xQueueHour,&HOUR,pdMS_TO_TICKS(100));
        xQueueSend(xQueueWeek,&WEEK,pdMS_TO_TICKS(100));
        xQueueSend(xQueueDay,&DAY,pdMS_TO_TICKS(100));
        xQueueSend(xQueueMon,&MON,pdMS_TO_TICKS(100));
        xQueueSend(xQueueYear,&YEAR,pdMS_TO_TICKS(100));

        printf("%02x:%02x:%02x | %s | %02x/%02x/20%02x\n",HOUR, MIN, SEC, WEEK_, DAY, MON, YEAR); // Imprime calendário

        xSemaphoreGive(xSemaphoreMutex);
        vTaskDelay(pdMS_TO_TICKS(250)); // Tempo para outra tarefa acesse a próxima tarefa
    }
}

void RTC_Set (uint8_t sec, uint8_t min, uint8_t hour, uint8_t week, uint8_t day, uint8_t mon, uint8_t year) // Função para ajuste do relógio
{
    I2C_Write_Add(ID_RTC, 0x06, year); // Configura ANO para 24
    I2C_Write_Add(ID_RTC, 0x05, mon); // Configura mês para 02
    I2C_Write_Add(ID_RTC, 0x04, day); // Configura dia para 24
    I2C_Write_Add(ID_RTC, 0x03, week); // Configura dia da semana para 7 (1 para Domingo ~ 7 para Sábado)
    I2C_Write_Add(ID_RTC, 0x02, hour); // Configura hora para 09
    I2C_Write_Add(ID_RTC, 0x01, min); // Configura minuto para 00
    I2C_Write_Add(ID_RTC, 0x00, sec); // Configura segundo para 00
}

uint8_t RTC_Read_Sec()
{
    return I2C_Read(ID_RTC, 0x00); // Envia ID RTC (68), registrador de segundo
}

uint8_t RTC_Read_Min()
{
    return I2C_Read(ID_RTC, 0x01); // Envia ID RTC (68), registrador de minuto
}

uint8_t RTC_Read_Hour()
{
    return I2C_Read(ID_RTC, 0x02); // Envia ID RTC (68), registrador de hora
}

uint8_t RTC_Read_Week()
{
    return I2C_Read(ID_RTC, 0x03); // Envia ID RTC (68), registrador de segundo
}

uint8_t RTC_Read_Day()
{
    return I2C_Read(ID_RTC, 0x04); // Envia ID RTC (68), registrador de dia
}

uint8_t RTC_Read_Mon()
{
    return I2C_Read(ID_RTC, 0x05); // Envia ID RTC (68), registrador de mês
}

uint8_t RTC_Read_Year()
{
    return I2C_Read(ID_RTC, 0x06); // Envia ID RTC (68), registrador de ano
}
