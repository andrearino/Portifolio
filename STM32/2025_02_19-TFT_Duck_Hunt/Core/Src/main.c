/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @modify    		: 02/27/2025
  * @version		: V2.2
  * @project        : Duck_Hunt
  * @designer       : André_Arino
  * @start data    	: 02/19/2025
  * @file           : main.c
  * @brief          : Main program body
  * @Hardware       : Nucleo F446RE Development Kit
  * @Dependences    : TFT Lib | Duck.c and Target.c Vectors Imagens
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fonts.h"	// Biblioteca de Fontes para TFT
#include "tft.h"	// Biblioteca para TFT
#include "user_setting.h"	// Biblioteca de Ajustes para o TFT
#include "stdlib.h"	// Inclusão de stdlib.h para alocação de memória
#include "stm32f4xx_hal_adc.h" // Inclusão de API para ADC
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

// Cria struct de 2 elementos para guardar endereços e dados das matrizes de alocação dinâmica
typedef struct {
    int32_t address;
    int32_t address_x;
    int32_t address_y;
    uint16_t data;
} Element;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
uint16_t ID=0;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
// Vetores das Imagens
extern const unsigned short Duck[76800];	// Carrega Imagem Pato (Imagem de Fundo)
extern const unsigned short Target[76800];	// Carrega Imagem Mira (Imagem Flutuante)

// Variável para mensagens de debug
volatile char Transmissao[100]; // Variável para enviar mensagens de debug

// Variáveis Auxiliares do Programa
volatile int32_t i = 0;

// Variáveis ADC
volatile uint8_t cad_ch = 0;	// Seleciona canal do leitura do ADC
volatile uint8_t cad = 0;	// guarda valor analógico do ADC
volatile uint8_t cad_x = 0;	// guarda valor analógico do joystik x
volatile uint8_t cad_y = 0;	// guarda valor analógico do joystik y
volatile uint8_t joystique_x = 0;	// Variável para leitura do eixo x do joystique
volatile uint8_t joystique_y = 0;	// Variável para leitura do eixo y do joystique

// Flag para Interrupção no botão
volatile uint8_t shot = 0;	// Variável para leitura do botão do joystique

// Variáveis de Deslocamento do Objeto
volatile int32_t n_Object = 0;	// Define número de elementos da Matriz Object
volatile int32_t x_pixel_max = 0x00000000;  // Variável para obter a posição máxima x do pixel do Objeto
volatile int32_t x_pixel_min = 2147483647;  // Variável para obter a posição mínima x do pixel do Objeto
volatile int32_t y_pixel_max = 0x00000000;  // Variável para obter a posição máxima y do pixel do Objeto
volatile int32_t y_pixel_min = 2147483647;  // Variável para obter a posição mínima y do pixel do Objeto
volatile int32_t cx_pixel = 0;  // Variável para obter a posição central de x do pixel do Objeto
volatile int32_t cy_pixel = 0;  // Variável para obter a posição central de y do pixel do Objeto
volatile int32_t shift_x = 0;	// guarda valor analógico do joystik x
volatile int32_t shift_y = 0;	// guarda valor analógico do joystik y

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
/* USER CODE BEGIN PFP */
uint16_t ADC_Value(uint8_t ch);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  __set_BASEPRI (1);	// Seta Basepri para Interrupção
  // __set_PRIMASK (0);	// Nível Primask para Interrupção
  // __set_FAULTMASK (0);	// Nível Faultmask para Interrupção
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  /* USER CODE BEGIN 2 */
  //Sequência de inicialização do LCD
  tft_gpio_init(); //Inicializa os GPIOs do LCD (evita uso do CubeMX)
  HAL_TIM_Base_Start(&htim1); //Inicializa o Timer1 (base de tempo de us do LCD)
  ID = tft_readID(); //Lê o ID do LCD (poderia ser chamada pela inicialização do LCD)
  HAL_Delay(100);
  tft_init (ID); //Inicializa o LCD de acordo com seu ID
  tft_setRotation(3); //Ajusta a orientação da tela
  tft_fillScreen(BLACK); //Preenche a tela em uma só cor

  // Imprime no TFT o fundo da Imagem
  tft_drawRGBBitmap(1, 240, Duck, 320, 240);

  //Teste de Crição da Matriz do Objeto
  HAL_Delay(1000);	// Aguarda 1 segundo

  n_Object = 0;
  /* Análise e geração de matriz dinâmica para objeto*/
  for (i = 0; i < 76800; i++) // Percorre o tamanho da imagem
  {
	  if(Target[i] != 0xFFFF)	// Procura elementos diferentes de Branco
	  {
		  n_Object++;	// Incrementa tamanho do
	  }
  }

  // Informa o tamanho da matriz e quantidade de bytes ocupados pela serial de debug
//  sprintf(&Transmissao,"A Matriz Object Possui: %u Elemetros com %u bytes.\r\n", n_Object, (n_Object*32));
//  HAL_UART_Transmit(&huart2, (uint8_t *)Transmissao, strlen(Transmissao), HAL_MAX_DELAY);

  // Cria Matriz dinâmico para guardar Pixels e Endereços da Imagem Flutuante
  Element *arr_Object = (Element *)malloc(n_Object * sizeof(Element));
  Element *arr_Recovery = (Element *)malloc(n_Object * sizeof(Element));
  n_Object = 0;
  for (i = 0; i < 76800; i++) // Percorre o tamanho da imagem
  {
	  if(Target[i] != 0xFFFF)	// Procura elementos diferentes de Branco
	  {
		  arr_Object[n_Object].address = (i+1);	// Insere o endereço do vetor do pixel linha n_Object da Matriz
		  arr_Recovery[n_Object].address = (i+1);	// Guarda o endereço do vetor do pixel linha n_Object da Matriz
		  arr_Object[n_Object].address_x = ((i+1)-((i/320)*320));	// Insere o endereço y do pixel linha n_Object da Matriz
		  arr_Recovery[n_Object].address_x = ((i+1)-((i/320)*320));	// Guarda o endereço do vetor do pixel linha n_Object da Matriz
		  // Guarda Valores de endereço x máximo e mínimo
		  if(x_pixel_max <= arr_Object[n_Object].address_x)
		  {
			  x_pixel_max = arr_Object[n_Object].address_x;
		  }
		  if(x_pixel_min >= arr_Object[n_Object].address_x)
		  {
			  x_pixel_min = arr_Object[n_Object].address_x;
		  }
		  arr_Object[n_Object].address_y = ((i+1)/320);	// Insere o endereço y do pixel linha n_Object da Matriz
		  arr_Recovery[n_Object].address_y = ((i+1)/320);	// Insere o endereço y do pixel linha n_Object da Matriz
		  // Guarda Valores de endereço y máximo e mínimo
		  if(y_pixel_max <= arr_Object[n_Object].address_y)
		  {
			  y_pixel_max = arr_Object[n_Object].address_y;
		  }
		  if(y_pixel_min >= arr_Object[n_Object].address_y)
		  {
			  y_pixel_min = arr_Object[n_Object].address_y;
		  }
		  arr_Object[n_Object].data = Target[i]; // Insere o valor do pixel linha n_Object da Matriz
		  arr_Recovery[n_Object].data = Duck[i]; // Guarda o valor do pixel linha n_Object da Matriz
		  // imprime objeto no TFT
		  tft_drawPixel(arr_Object[n_Object].address_x, arr_Object[n_Object].address_y, arr_Object[n_Object].data);
//		  sprintf(&Transmissao,"Objeto: x %u | y %u | vetor %X | valor %X.\r\n",
//		  arr_Object[n_Object].address_x, arr_Object[n_Object].address_y, arr_Object[n_Object].address, arr_Object[n_Object].data);
//		  HAL_UART_Transmit(&huart2, (uint8_t *)Transmissao, strlen(Transmissao), HAL_MAX_DELAY);
//		  sprintf(&Transmissao,"Fundo: x %u | y %u | vetor %X | valor %X.\r\n",
//		  arr_Recovery[n_Object].address_x, arr_Recovery[n_Object].address_y, arr_Recovery[n_Object].address, arr_Recovery[n_Object].data);
//		  HAL_UART_Transmit(&huart2, (uint8_t *)Transmissao, strlen(Transmissao), HAL_MAX_DELAY);
		  n_Object++;	// Incrementa tamanho do tamanho da matriz
	  }
  }
  // Obtem os a posição central do objeto
  cx_pixel = ((x_pixel_max - x_pixel_min)/2);
  cy_pixel = ((y_pixel_max - y_pixel_min)/2);

// 	Informa as posições x e y máximas e mínimas do objeto e a posição central
//  sprintf(&Transmissao,"Object Possui: x_min %u | x_max %u | y_min %u | y_min %u | x_centro %u | x_centro %u.\r\n",
//		  x_pixel_min, x_pixel_max, y_pixel_min, y_pixel_max, cx_pixel, cy_pixel);
//  HAL_UART_Transmit(&huart2, (uint8_t *)Transmissao, strlen(Transmissao), HAL_MAX_DELAY);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	 // tratamento da interrupção (Botão PB1 - tiro)
	 if(shot == 1)	// Caso a interrupção tenha sido ativada, o valor de shot será 1
	 {
		 tft_drawCircle(cx_pixel, cy_pixel, 5, RED);	// imprime um círculo de 5 pixel de diâmetro no centro do objeto
		 shot = 0;	// limpa flag
	 }

	 // Leitura dos valores do Joystick x e y (valores centrais por volta de 25 contagens com valores entre 0 e 255)
	 cad_ch = 1; // Seleciona o canal
	 cad_x = ADC_Value(cad_ch);	// Envia número de canal a função e guarda o valor lido na variável cad_x
//	 sprintf(&Transmissao,"ADC X = %u V.\r\n", cad_x);
//	 HAL_UART_Transmit(&huart2, (uint8_t *)Transmissao, strlen(Transmissao), HAL_MAX_DELAY);
//	 HAL_Delay(1000);

	 cad_ch = 2; // Seleciona o canal
	 cad_y = ADC_Value(cad_ch);		// Envia número de canal a função e guarda o valor lido na variável cad_y
//	 sprintf(&Transmissao,"ADC Y = %u V.\r\n", cad_y);
//	 HAL_UART_Transmit(&huart2, (uint8_t *)Transmissao, strlen(Transmissao), HAL_MAX_DELAY);
//	 HAL_Delay(1000);

	 // Ajuste de Sensibilidade de velocidade de deslocamento x
	 if(cad_x >= 7 && cad_x <= 10)	// baixa sensibilidade
	 {
		 shift_x = -2;	// deloca 2 posições esquerda
	 }
	 else if(cad_x >= 0 && cad_x <= 6)	// alta sensibilidade
	 {
		 shift_x = -10;	// deloca 10 posições esquerda
	 }
	 else if(cad_x >= 26  && cad_x <= 35)		// baixa sensibilidade
	 {
		 shift_x = 2;	// deloca 2 posições direita
	 }
	 else if(cad_x >= 35 && cad_x <= 255)	// alta sensibilidade
	 {
		 shift_x = 10;	// deloca 10 posições direita
	 }
	 else	// zona entre 11 e 49
	 {
		 shift_x = 0;	// inativa (controle de ruído)
	 }
	 // Ajuste de Sensibilidade de velocidade de deslocamento y
	 if(cad_y >= 7 && cad_y <= 10)	// baixa sensibilidade
	 {
		 shift_y = -2;	// deloca 2 posições cima
	 }
	 else if(cad_y >= 0 && cad_y <= 6)	// alta sensibilidade
	 {
		 shift_y = -10;	// deloca 10 posições cima
	 }
	 else if(cad_y >= 26  && cad_y <= 35)	// baixa sensibilidade
	 {
		 shift_y = 2;	// deloca 2 posições baixo
	 }
	 else if(cad_y >= 36 && cad_y <= 255)	// alta sensibilidade
	 {
		 shift_y = 10;	// deloca 10 posições baixo
	 }
	 else	// zona entre 11 e 49
	 {
		 shift_y = 0;	// inativa (controle de ruído)
	 }

	 // tratamento de deslocamento da imagem
	 if(shift_x != 0 || shift_y != 0)	//	Caso sensibilidade do ADC seja diferente de 0
	 {
		// Avaliação das alteraçoes de X e Y
//		sprintf(&Transmissao,"Deslicamento: x %u | y %u.\r\n",
//		shift_x, shift_y);
//		HAL_UART_Transmit(&huart2, (uint8_t *)Transmissao, strlen(Transmissao), HAL_MAX_DELAY);

		// Checa se os valores somados ao deslocamento estão fora das dimenões do TFT
		if((x_pixel_min + shift_x) >= 0 && (x_pixel_max + shift_x) <= 320 &&
		   (y_pixel_min + shift_y) >= 0 && (y_pixel_max + shift_y) <= 240)
		{
			// Caso estejam dentro, segue o loop

			for (i = 0; i < n_Object; i++) // Percorre o tamanho do objeto
			{
				// Imprime matriz recovery (posições atuais da matriz objeto, com dados do fundo) - Apaga objeto anterior
				tft_drawPixel(arr_Recovery[i].address_x, arr_Recovery[i].address_y, arr_Recovery[i].data);
//			    sprintf(&Transmissao,"Fundo: x %u | y %u | vetor %X | valor %X.\r\n",
//			    arr_Recovery[i].address_x, arr_Recovery[i].address_y, arr_Recovery[i].address, arr_Recovery[i].data);
//			    HAL_UART_Transmit(&huart2, (uint8_t *)Transmissao, strlen(Transmissao), HAL_MAX_DELAY);
			}

			// Calcula os novos pontos do objeto com o deslocamento e guarda os valores do fundo da nova prosição
			for (i = 0; i < n_Object; i++) // Percorre o tamanho do objeto
			{
				arr_Object[i].address = ((arr_Object[i].address + shift_x) + (shift_y * 320));
				arr_Recovery[i].address = (arr_Object[i].address); // Guarda posição do vetor para obter o valor do fundo
				arr_Recovery[i].data = Duck[(arr_Recovery[i].address)];	 // Guada o valor do fundo com base no endereço do vetor
				arr_Object[i].address_x =  (((arr_Object[i].address)+1)-(((arr_Object[i].address)/320)*320)); // Atualiza posição x do objeto
				arr_Recovery[i].address_x = arr_Object[i].address_x; // Guarda a posição na matriz de fundo
				arr_Object[i].address_y = (((arr_Object[i].address)+1)/320); // Atualiza posição x do objeto
				arr_Recovery[i].address_y = arr_Object[i].address_y; // Guarda a posição na matriz de fundo
			}

			//	Atualiza novas posições para limitar movimentos fora do TFT
			x_pixel_min = x_pixel_min + shift_x;	// Atualiza X mínimo
			x_pixel_max = x_pixel_max + shift_x;	// Atualiza X máximo
			y_pixel_min = y_pixel_min + shift_y;	// Atualiza y mínimo
			y_pixel_max = y_pixel_max + shift_y;	// Atualiza y máximo
			cx_pixel = (((x_pixel_max - x_pixel_min)/2) + x_pixel_min);	// Atualiza ponto central x
			cy_pixel = (((y_pixel_max - y_pixel_min)/2) + y_pixel_min);	// Atualiza ponto central y

			for (int32_t i = 0; i < n_Object; i++) // Imprime objeto na nova posição
			{
				tft_drawPixel(arr_Object[i].address_x, arr_Object[i].address_y, arr_Object[i].data);
//			    sprintf(&Transmissao,"Objeto: x %u | y %u | vetor %X | valor %X.\r\n",
//			    arr_Object[i].address_x, arr_Object[i].address_y, arr_Object[i].address, arr_Object[i].data);
//			    HAL_UART_Transmit(&huart2, (uint8_t *)Transmissao, strlen(Transmissao), HAL_MAX_DELAY);

			}
		}
		// Avaliação das posições atuais de X e Y
//		sprintf(&Transmissao,"Atual: x_min %u | x_max %u | y_min %u | y_max %u.\r\n",
//		x_pixel_min, x_pixel_max, y_pixel_min, y_pixel_max);
//		HAL_UART_Transmit(&huart2, (uint8_t *)Transmissao, strlen(Transmissao), HAL_MAX_DELAY);
	 }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_14;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.ScanConvMode = DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DMAContinuousRequests = DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_15;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 84-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 0xFFFF-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_8
                          |GPIO_PIN_9|GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_10|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC1 PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA4 PA8
                           PA9 PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_8
                          |GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB10 PB3 PB4
                           PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_10|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : Button_Pin */
  GPIO_InitStruct.Pin = Button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Button_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

// Rotina de interrupção
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)	// recebe o número do pino que causou a interrupção
{
	if(GPIO_Pin == GPIO_PIN_1) // Check if PIN is 1 (no caso PB1)
	{
		shot = 1;	// Caso positivo, atribui valor 1 a variável shot de flag da rotina principal
	}
}

//	Rotina de leitura do ADC
uint16_t ADC_Value(uint8_t ch)	// recebe o canal do ADC a ser lido e retorna valor lido
{
	switch (ch)	// checa qual canal
	{
		case 1:	// se 1 - posição X
		HAL_ADC_Start(&hadc1);	// Inicia ADC 1
		if(HAL_ADC_PollForConversion(&hadc1, 5) == HAL_OK)	// aguarda 5 ms para receber dados
		{
			cad = HAL_ADC_GetValue(&hadc1);	// guarda dados na variável cad
			HAL_Delay(50);	//	espera 50ms para a próxima leitura
			HAL_ADC_Stop(&hadc1);	// desliga ADC
			return cad;	//	retona valor da leitura para a função chamada
		}
		break;	//	encerra case 1

		case 2:	// se 2 - posição Y
		HAL_ADC_Start(&hadc2);	// Inicia ADC 2
		if(HAL_ADC_PollForConversion(&hadc2, 5) == HAL_OK)	// aguarda 5 ms para receber dados
		{
			cad = HAL_ADC_GetValue(&hadc2);	// guarda dados na variável cad
			HAL_Delay(50);	//	espera 50ms para a próxima leitura
			HAL_ADC_Stop(&hadc2);	// desliga ADC
			return cad;	//	retona valor da leitura para a função chamada
		}
		break;	//	encerra case 2

		default:	// caso outras
		HAL_UART_Transmit(&huart2, (uint8_t *)"Erro leitura ADC\r\n", 19, HAL_MAX_DELAY);	// Debug de Erro
		break;	//	encerra case default
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
