/***********************************************************************\
 * Programa Expansão de Entradas do Arduino, utilizando Shift Register *
 * CI utilizado: 74HC165                                               *
 * Por: João Sérgio Caciola  -  Eletrônica Projetos e Treinamentos     *
 * Data: 09/04/2017                                                    *
 * Créditos: Baseado no playground.arduino.cc                          *
\***********************************************************************/

// Definições de Labels
#define nCIs  2               //Registra o número de CIs cascateados
#define nExtIn  nCIs * 8      //Registra o número de novas entradas externas
#define TempoDeslocamento 50  //Registra o tempo de queverá ter o pulso para leitura e gravação, (milesegundos)
#define Atraso  200           //Registra o atraso de segurança entre leituras, (milesegundos)
#define BYTES_VAL_T unsigned int  //Altera de int para long, se o a quantidade de CIs cascateados for maior que 2


// Declaração de variáveis globais
const int ploadPin        = 7;    //Conecta ao pino 1 do 74HC165 (LH/LD - asynchronous parallel load input)(PL)
const int clockEnablePin  = 8;    //Conecta ao pino 15 do 74HC165 (CE - Clock Enable Input)(CE)
const int dataPin         = 12;   //Conecta ao pino 9 do 74HC165 (Q7 - serial output from the last stage)(Q7)
const int clockPin        = 13;   //Conecta ao pino 2 do 74HC165 (CP - Clock Input)(CP)

BYTES_VAL_T pinValues;
BYTES_VAL_T oldPinValues;


//Função para definir um rotina shift-in, lê os dados do 74HC165
BYTES_VAL_T read_shift_regs()
{
    long bitVal=0;
    BYTES_VAL_T bytesVal = 0;
    
    digitalWrite(clockEnablePin, HIGH);
    digitalWrite(ploadPin, LOW);
    delayMicroseconds(TempoDeslocamento);
    digitalWrite(ploadPin, HIGH);
    digitalWrite(clockEnablePin, LOW);

    // Efetua a leitura de um bit da saida serial do 74HC165
    for(int i = 0; i < nExtIn; i++)
    {
        bitVal = digitalRead(dataPin);

        //Realiza um shift left e armazena o bit correspondente em bytesVal
        bytesVal |= (bitVal << ((nExtIn-1) - i));

        //Lança um pulso de clock e desloca o próximo bit
        digitalWrite(clockPin, HIGH);
        delayMicroseconds(TempoDeslocamento);
        digitalWrite(clockPin, LOW);
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
    //Avita a comunicação via monitor serial
    Serial.begin(38400);
    
    //Inicializa e configura os pinos
    pinMode(ploadPin, OUTPUT);
    pinMode(clockEnablePin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, INPUT);

    digitalWrite(clockPin, HIGH);
    digitalWrite(ploadPin, HIGH);

    //Lê e mostra o estado dos pinos
    pinValues = read_shift_regs();
    display_pin_values();
    oldPinValues = pinValues;
        
}

//Função do loop principal
void loop()
{
    //Lê todos as portas externas
    pinValues = read_shift_regs();
    
    //Se houver modificação no estado dos pinos, mostra o estado atual
    if(pinValues != oldPinValues)
    {
        Serial.print("*Alteracao detectada*\r\n");
        display_pin_values();
        oldPinValues = pinValues;
    }

    delay(Atraso);
}
