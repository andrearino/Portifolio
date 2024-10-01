// Biblioteca
#include <stdio.h> // Biblioteca Padrão

// Definições
#define PERIOD 1 // Valor do período de clock do HW262 ms
#define ADC_Time 500 // Valor de Tempo de Atividade do SERIAL em ms

#define BAUDRATE 115200 // Valor de Baudrate em BPS
#define LED1 13 // Define LED1 para ligação do Pino 13 do uC ao terminal LED1 do Learn Shield
#define LED2 12 // Define LED1 para ligação do Pino 13 do uC ao terminal LED1 do Learn Shield
#define RCLK 4 // Define RCLK como 4 para ligação do Pino 4 do uC ao terminal RCLK ao HW262
#define SRCLK 7 // Define SRCLK como 7 para ligação do Pino 7 do uC ao terminal SRCLK ao HW262
#define SER 8 // Define SER como 8 para ligação do Pino 8 do uC ao terminal SER ao HW262
#define ADC A0 // Define LED1 para ligação do Pino 13 do uC ao terminal LED1 do Learn Shield

// Variávei Globais
uint16_t ADC_Value = 0;  // Cria Variável para o valor do ADC com 16 bits (ATmega32U4 = 10 bits = 0 ~ 1023)
int ADC_Pin = ADC;   // Seleciona a entrada do ADC A0 ligada ao potenciômetro
uint8_t Data[5][2] =
{
  {0,0b00000000}, // Display 1
  {1,0b00000000}, // Display 2 
  {2,0b00000000}, // Display 3 
  {3,0b00000000},  // Display 4
  {4,0b00000000},  // Apaga
}; // Cria vetor de 4 elementos para guardar os dados
uint8_t Check[10][2] = 
{  
  {0, 0b11111100}, //0 
  {1, 0b01100000}, //1 
  {2, 0b11011010}, //2 
  {3, 0b11110010}, //3 
  {4, 0b01100110}, //4 
  {5, 0b10110110}, //5 
  {6, 0b10111110}, //6 
  {7, 0b11100000}, //7 
  {8, 0b11111110}, //8 
  {9, 0b11110110}, //9 
}; // Tabela com variáveis de conversão BCD para 7 segmentos
uint8_t Sequencia[5][2] =
{
  {'0',0b00010000}, // Display 1
  {'1',0b00100000}, // Display 2 
  {'2',0b01000000}, // Display 3 
  {'3',0b10000000},  // Display 4
  {'3',0b00000000},  // Apaga
}; // Sequência dos Displays
uint8_t clock = PERIOD/2; // Com  base no período de clock definido, gera o ciclo de clock de exibição do display em ms
uint8_t D_PAR[5][2]; // definição matriz com todos dos 4 dados de 16 bits (8 sequência e 8 dados) do display em binário a serem transmitidos em serial
bool D_SER[5][16];  // definição matriz com todos dos 4 dados do display em binário a serem transmitidos em serial
uint8_t temp; // Variável temperária para gravar o dado
bool temp_not; // Variável temperária para gravar o dado
int i = 0; // variáveis para os loops for
int x = 0; // variáveis para os loops for

// Handles

// Protótipos
uint16_t ADC_Func(void); // Protótipo ADC
void SERIAL_Func(uint16_t ADC_Value_S); // Protótipo SERIAL
void DISPLAY_Func(uint16_t ADC_Value_S);   // Protótipo DISPLAY

void setup() 
{
  Serial.begin(BAUDRATE); // Inicia serial no baudrate definido
  pinMode(RCLK, OUTPUT);
  pinMode(SRCLK, OUTPUT);
  pinMode(SER, OUTPUT);
}

void loop() 
{
  uint16_t ADC_var; // Variável para guardar valor da serial e passar a outra função
  while(1)
  {
    ADC_var = ADC_Func();
    SERIAL_Func(ADC_var);
    DISPLAY_Func(ADC_var);
  }
}

// Tarefa ADC
uint16_t ADC_Func(void)
{
 ADC_Value = analogRead(ADC_Pin); // Lê valor do ADC no pino A0 (potenciômetro 0 ~ 5V)
 delay(ADC_Time); // Aguarda tempo do ADC
 return ADC_Value; // Retorna valor do ADC
}

// Tarefa Serial
void SERIAL_Func(uint16_t ADC_Value_S)
{
  Serial.print("Leitura ADC = "); // Envia Texto
  Serial.print(ADC_Value_S); // Envia Dado
  Serial.println(); // Pula 1 linha
}

void DISPLAY_Func(uint16_t ADC_Value_S)   // dados da sequência S1 e de dados D1 do Display 
{
  /** Transmissão para o Display da Leitura**/
  Data[0][0] = (uint8_t)(ADC_Value_S % 10); // Converte Unidade e guarda em Data 0
  Data[1][0] = (uint8_t)((ADC_Value_S / 10) % 10); // Converte Dezena e guarda em Data 1
  Data[2][0] = (uint8_t)((ADC_Value_S / 100) % 10); // Converte Centena e guarda em Data 2
  Data[3][0] = (uint8_t)((ADC_Value_S / 1000) % 10); // Converte Milhar e guarda em Data 3

  // Geração de sequência e dados na variável D_SER
  for(i = 0; i < 5; i++) // percorre as 4 variáveis guardadas Data 0 ~ 3
  {
    for(x = 0; x <= 9; x++)  // percorre tabela 0 ~ 9
    {
      if(Data[i][0] == Check[x][0]) // Compara o dado com a tabela e se for igual executa o comando abaixo
      {
        D_PAR[i][1] = Check[x][1]; // Escreve segundo byte do dado com a tabela de dados com base em i e x
      }
    }
    D_PAR[i][0] = Sequencia[i][1]; // Escreve primeiro byte do dado com a tabela de sequência com base em i
    x = 0; // Zera contagem do x para o próximo display (i)  
  }
  i=0; // Zera contagem do i para a próxima conversão

  // conversão de paralelo para serial
  for(i = 0; i < 5; i++) // percorre as 4 variáveis guardadas Data 0 ~ 3
  {
    for(x = 0; x < 16; x++) // gera um vetor de 16 bits em booleano para guardar na variavel da matriz referente ao display
    {
      if(x <= 7) // filtra os dados de seleção do display
      {
        temp = D_PAR[i][1]; // Copia os dados do display a ser acionado a uma variável temp 
        temp = (temp >> x); // Move para a direita conforme incremento de x (bits 1 a 8)
        temp_not = (bool)(temp & 0b00000001); // Filtra apenas o primeiro bit, transforma em booleano e copia em temp_not 
        D_SER[i][x] = (temp_not ^ 0b1); // inverte o dado (seleção aciona em zero volts) e copia para o display i na posição do vetor x da variável D_SER
      }
      if(x > 7) // filtra os dados dos seguimentos do display
      {
        temp = D_PAR[i][0]; // Copia os dados do seguimento do display a ser acionado a uma variável temp 
        temp = (temp >> (x-8)); // Move para a direita conforme incremento de x-8 (pois x já vale 8)
        D_SER[i][x] = (bool)(temp & 0b00000001); // Filtra apenas o primeiro bit, transforma em booleano e copia para o display i na posição do vetor x da variável D_SER
      } 
    }
    x=0; // Zera contagem do x para o próximo display (i)
  }
  i=0; // Zera contagem do i para a próxima conversão 
  
  // Inicia transmissão dos dados para os 4 displays
  for(i = 0; i < 5; i++) // percorre os 4 displays
  {
    digitalWrite(SRCLK, LOW); // Sequência de inicialização SRCLK em 0
    digitalWrite(RCLK, HIGH); // Sequência de inicialização RCLK em 1
    // Começa a gerar o clock SRCLK e RCLK transmissão de dados
    delayMicroseconds(clock); // Primeiro semi ciclo
    for(x = 0; x < 16; x++)  // Inicia a transmissão da sequência de dados
    {
      if(x == 0) // Protocolo para o primeiro ciclo de envio de dados
      {
        // Altera estado clock em SRCLK
        digitalWrite(SRCLK, HIGH); 
        delayMicroseconds(clock);
        // Transmite primeiro bit da sequência
        digitalWrite(SER, D_SER[i][x]);
        // Gera ciclo de clock completo em SRCLK sincronizado com o tamanho do dado
        digitalWrite(SRCLK, LOW);
        delayMicroseconds(clock);
        digitalWrite(SRCLK, HIGH);
        delayMicroseconds(clock);
      }
      else if((x > 0) && (x < 15)) // Protocolo para os demais ciclos de envio de dados
      {
        // Envia próximo dado
        digitalWrite(SER, D_SER[i][x]);
        // Gera ciclo de clock completo em SRCLK sincronizado com o tamanho do dado
        digitalWrite(SRCLK, LOW);
        delayMicroseconds(clock);
        digitalWrite(SRCLK, HIGH);
        delayMicroseconds(clock);
      }
      else if(x == 15) // Protocolo para o fim do ciclo de envio de dados
      {
        // Envia último dado
        digitalWrite(SER, D_SER[i][x]);
        // Gera ciclo de clock completo em SRCLK sincronizado com o tamanho do dado
        digitalWrite(SRCLK, LOW);
        delayMicroseconds(clock);
        digitalWrite(SRCLK, HIGH);
        // Gera ciclo de clock completo em RCLK e SRCLK para habilitar saída ao display
        digitalWrite(RCLK, LOW);
        delayMicroseconds(clock*100);
        digitalWrite(SRCLK, LOW);
        digitalWrite(RCLK, HIGH);
        delayMicroseconds(clock);
        digitalWrite(SRCLK, HIGH);
        // Delay mais longo para a transmissão do próximo display
        delayMicroseconds(clock);
      }
    }
    x = 0; // zera x para a transmissão ao próximo display
  }
  i = 0; // Zera contagem do i para a próxima conversão
}