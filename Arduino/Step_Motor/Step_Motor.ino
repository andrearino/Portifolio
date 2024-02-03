/*
Andre Arino
Data: 28/01/2024 - Ver. 1.0
Sketch para Motor de Passo 28BYJ-48 e Drive ULN2003A
Teste básico de controle de motor de passo em modos Wave e Normal
Seleção do modo de controle e frequência de rotação.

Conexões:
ULN2003A IN4 -> Arduino 11
ULN2003A IN3 -> Arduino 10
ULN2003A IN2 -> Arduino 09
ULN2003A IN1 -> Arduino 08
ULN2003A VCC -> Arduino 5V
ULN2003A GND -> Arduino GND
*/

const int IN1 = 8; //Bobina 1
const int IN2 = 9; //Bobina 2
const int IN3 = 10; //Bobina 3
const int IN4 = 11; //Bobina 4
int step; //tempo de cada pulso (ms)
float frequency; // frequência (Hz)
int modo; //seleção do modo de operação
int escolha; //seleção do a opção do menu

void setup()
{
  pinMode(IN1, OUTPUT); //Pinos como saídas
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  digitalWrite(IN1, LOW);//Bobinas desligadas
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void wave()
{
  // primeiro passo
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  delay(step);

  // segundo passo
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  delay(step);

  // terceiro passo
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  delay(step);

  // quarto passo
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  delay(step);
}

void normal()
{
  // primeiro passo
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  delay(step);

  // segundo passo
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  delay(step);

  // terceiro passo
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, HIGH);
  delay(step);

  // quarto passo
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  delay(step);
}

void ajuste ()
{
  Serial.begin(9600);
  // Seleção da frequência de operação do motor de passo
  Serial.print("Digite a frequencia de operação do motor de passo(2~100 Hz):\n");
  frequency = Serial.parseInt ();
  while(frequency <2 || frequency>100)
  {
    frequency = Serial.parseInt ();
    delay(500);
  }
  step = ( 1 / frequency ) * 1000;
  Serial.print(step, DEC);
  // Seleção do modo de operação do motor de passo
  Serial.print("Digite o modo de operação do motor de passo: \nWave(1) / Normal(2) \n");
  modo = Serial.parseInt ();
  while(modo < 1 || modo > 2 )
  {
    modo = Serial.parseInt ();
    delay(500);
  }
  Serial.print("Pressione: \nConfigurar o Motor(1) \nIniciar o Motor (2) \nParar o Motor (3)\n");
  escolha = 0;
  Serial.end();
}

void loop() 
{
  Serial.print("Ajuste a configuração inicial do motor. \n");
  ajuste();
  while(1)
  {
    while(escolha < 1 || escolha > 3)
    {
      Serial.begin(9600);
      delay(500);
      escolha = Serial.parseInt ();
      Serial.end();
    }

    Serial.begin(9600);

    if(escolha == 1)
    {
      ajuste();
    }

    if(escolha == 2)
    {
      switch (modo) 
      {
        case 1:
        wave();
        break;
        case 2:
        normal();
        break;
        default:
        break;
      }
    }

    if(escolha == 3)
    {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      escolha = 0;
    }

    if(Serial.available() > 0)
    {
      escolha = Serial.parseInt ();
    }

    Serial.end();

  }
}
