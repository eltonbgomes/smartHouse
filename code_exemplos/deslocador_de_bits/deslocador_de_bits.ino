/********************************************************************************\
 * Programa Expansão de Entradas e saídas do Arduino, utilizando Shift Register *
 * CI utilizado: 74HC165 com 74HC595                                            *
 * Por: Elton Barbosa Gomes                                                     *
 * Data: 28/09/2020                                                             *
 * Créditos: Baseado no playground.arduino.cc                                   *
\********************************************************************************/

//74hc165
// Definições de Labels
#define nCIs  2               //Registra o número de CIs cascateados
#define nExtIn  nCIs * 8      //Registra o número de novas entradas externas
#define TempoDeslocamento 50  //Registra o tempo de queverá ter o pulso para leitura e gravação, (milesegundos)
#define Atraso  200           //Registra o atraso de segurança entre leituras, (milesegundos)
#define BYTES_VAL_T unsigned int  //Altera de int para long, se o a quantidade de CIs cascateados for maior que 2


// Declaração de variáveis globais 165
const int ploadPin165        = 7;    //Conecta ao pino 1 do 74HC165 (LH/LD - asynchronous parallel load input)(PL)
const int clockEnablePin165  = 8;    //Conecta ao pino 15 do 74HC165 (CE - Clock Enable Input)(CE)
const int dataPin165         = 12;   //Conecta ao pino 9 do 74HC165 (Q7 - serial output from the last stage)(Q7)
const int clockPin165        = 13;   //Conecta ao pino 2 do 74HC165 (CP - Clock Input)(CP)

BYTES_VAL_T pinValues;
BYTES_VAL_T oldPinValues;


// Declaração de variáveis globais 595
int clockPin595 = 2; //Pino conectado a SRCLK (pino 11 no 74HC595), registrador de deslocamento
int latchPin595 = 3; //Pino conectado a RCLK (pino 12 no 74HC595), registrador de armazenamento
int dataPin595 = 4; //Pino conectado a SER (pino 14 no 74HC595), entrada de dados serial


//Função para definir um rotina shift-in, lê os dados do 74HC165
BYTES_VAL_T read_shift_regs()
{
    long bitVal=0;
    BYTES_VAL_T bytesVal = 0;
    
    digitalWrite(clockEnablePin165, HIGH);
    digitalWrite(ploadPin165, LOW);
    delayMicroseconds(TempoDeslocamento);
    digitalWrite(ploadPin165, HIGH);
    digitalWrite(clockEnablePin165, LOW);

    // Efetua a leitura de um bit da saida serial do 74HC165
    for(int i = 0; i < nExtIn; i++)
    {
        bitVal = digitalRead(dataPin165);

        //Realiza um shift left e armazena o bit correspondente em bytesVal
        bytesVal |= (bitVal << ((nExtIn-1) - i));

        //Lança um pulso de clock e desloca o próximo bit
        digitalWrite(clockPin165, HIGH);
        delayMicroseconds(TempoDeslocamento);
        digitalWrite(clockPin165, LOW);
    }

    return(bytesVal);
}

//Mostra os dados recebidos
void display_pin_values()
{
    Serial.print("Estado das entradas:\r\n");

    for(int i = 0; i < nExtIn; i++)
    {
        Serial.print("  Pin0-");
        Serial.print(i);
        Serial.print(": ");

        if((pinValues >> i) & 1)
            Serial.print("ALTO");
        else
            Serial.print("BAIXO");

        Serial.print("\r\n");
    }

    Serial.print("\r\n");
}

// Configuração do Programa
void setup()
{
    //habilita a comunicação via monitor serial
    Serial.begin(9600);
    
    //Inicializa e configura os pinos do 165
    pinMode(ploadPin165, OUTPUT);
    pinMode(clockEnablePin165, OUTPUT);
    pinMode(clockPin165, OUTPUT);
    pinMode(dataPin165, INPUT);

    digitalWrite(clockPin165, HIGH);
    digitalWrite(ploadPin165, HIGH);

    //incializa pinos do 595
    pinMode(latchPin595, OUTPUT);
    pinMode(clockPin595, OUTPUT);
    pinMode(dataPin595, OUTPUT);

    //Lê e mostra o estado dos pinos
    pinValues = read_shift_regs();
    display_pin_values();
    oldPinValues = pinValues;
        
}

//Função do loop principal
void loop(){
    //Lê todos as portas externas
    pinValues = read_shift_regs();
    
    //Se houver modificação no estado dos pinos, mostra o estado atual
    if(pinValues != oldPinValues){
        Serial.print("*Alteracao detectada*\r\n");
        display_pin_values();
        Serial.print("Valor das entradas em decimal\n");
        Serial.print(pinValues);
        oldPinValues = pinValues;
    }


    //alterar saida do 595
    digitalWrite(latchPin595, LOW);
    shiftOut(dataPin595, clockPin595, LSBFIRST, pinValues);
    digitalWrite(latchPin595, HIGH);

    digitalWrite(latchPin595, LOW);
    shiftOut(dataPin595, clockPin595, LSBFIRST, pinValues >> 8);
    digitalWrite(latchPin595, HIGH);

    delay(Atraso);
}
