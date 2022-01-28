/********************************************************************************\
 * Programa Expansão de Entradas e saídas do Arduino, utilizando Shift Register *
 * CI utilizado: 74HC165 com 74HC595                                            *
 * Por: Elton Barbosa Gomes                                                     *
 * Data: 28/09/2020                                                             *
 * Créditos: Baseado no playground.arduino.cc                                   *
\********************************************************************************/

//74hc165
// Definições de Labels
#define nCIs  3               //Registra o número de CIs cascateados
#define BYTES 8
#define TempoDeslocamento 50  //Registra o tempo de que deverá ter o pulso para leitura e gravação, (milesegundos)
#define Atraso  100           //Registra o atraso de segurança entre leituras, (milesegundos)

// Declaração de constantes globais 165
const int ploadPin165        = 6;    //Conecta ao pino 1 do 74HC165 (LH/LD - asynchronous parallel load input)(PL)
const int clockEnablePin165  = 5;    //Conecta ao pino 15 do 74HC165 (CE - Clock Enable Input)(CE)
const int dataPin165         = 8;   //Conecta ao pino 9 do 74HC165 (Q7 - serial output from the last stage)(Q7)
const int clockPin165        = 7;   //Conecta ao pino 2 do 74HC165 (CP - Clock Input)(CP)

//inicialização das variaveis onde serão armazenados os status
byte pinValues[nCIs];
byte oldPinValues[nCIs];
byte pinValuesOut[nCIs];
byte oldPinValuesOut[nCIs];
byte auxOut[nCIs]; // variavel usada para armazenar valores das saídas para serem alteradas na borda de descida
bool auxOutBool[nCIs];

//auxiliares para converter bases
bool auxBin[8];
bool bin[8] = {0,0,0,0,0,0,0,0};

// Declaração de constantes globais 595
const int clockPin595 = 2; //Pino conectado a SRCLK (pino 11 no 74HC595), registrador de deslocamento
const int latchPin595 = 3; //Pino conectado a RCLK (pino 12 no 74HC595), registrador de armazenamento
const int dataPin595  = 4; //Pino conectado a SER (pino 14 no 74HC595), entrada de dados serial

//usado para contar o tempo do botao pressionado
long time;

//Função para definir um rotina shift-in, lê os dados do 74HC165
void read_shift_regs(){
    bool bitVal=0; //variavel para que armazena bit a bit
    byte bytesVal[nCIs];
    byte bytesValOut[nCIs];
    for(int i = 0; i < nCIs; i++){
        bytesVal[i] = 0;
        bytesValOut[i] = pinValuesOut[i];
    }

    //desloca todos os bits para o pino de dados para leitura
    digitalWrite(clockEnablePin165, HIGH);
    digitalWrite(ploadPin165, LOW);
    delayMicroseconds(TempoDeslocamento);
    digitalWrite(ploadPin165, HIGH);
    digitalWrite(clockEnablePin165, LOW);

    // Efetua a leitura de um bit da saida serial do 74HC165

    for(int i = 0; i < nCIs; i++){
        int Ci = nCIs - 1 - i;
        for (int j = 0; j < BYTES; j++){
            bitVal = digitalRead(dataPin165);

            //Realiza um shift left e armazena o bit correspondente em bytesVal
            bytesVal[Ci] |= (bitVal << ((BYTES-1) - j));

            //compara se ouve mudanca para alterar o 595
            bytesValOut[Ci] ^= (bitVal << ((BYTES-1) - j));

            //Lança um pulso de clock e desloca o próximo bit
            digitalWrite(clockPin165, HIGH);
            delayMicroseconds(TempoDeslocamento);
            digitalWrite(clockPin165, LOW);
        }
        pinValues[Ci] = bytesVal[Ci];

        //condição para somente alterar as saídas se soltar o botão
        if(pinValues[Ci] != oldPinValues[Ci] && !auxOutBool[Ci]){
            time = millis();
            auxOut[Ci] = bytesValOut[Ci];
            auxOutBool[Ci] = true;
        }

        //condicao para impedir a alteração com o botao pressionado
        if(pinValues[Ci] == 0 && auxOutBool[Ci]){
            pinValuesOut[Ci] = auxOut[Ci];
            auxOutBool[Ci] = false;
        }

        //condiçao para funcionar apenas com o botao 07 Ci 0
        if((millis() - time) > 2000 && auxOutBool[Ci] && pinValues[Ci] == 128 && Ci == 0){
            convDecBin(Ci); // converte o valor decimal para binario de pinValuesOut
            
            //desliga as saidas desejadas
            auxBin[7] = 0;
            auxBin[6] = 0;
            
            //inversao
            for(int i = 7; i >= 0; i--){    
                bin[7-i] = auxBin[i];
            }

            pinValuesOut[Ci] = convBinDec();
            auxOutBool[Ci] = false;
        }

        //desliga as saidas após tempo pressionado
        if((millis() - time) > 3500 && auxOutBool[Ci]){
            pinValuesOut[Ci] = 0;
            auxOutBool[Ci] = false;
        }
    }
}

void convDecBin (int Ci){
    for(int i = 0; i < 8; i++){
        auxBin[i] = 0; 
    }
    int decimal = pinValuesOut[Ci];
    int i;
    for(i = 0; (decimal > 1); i++){
        auxBin[i] = decimal % 2;
        decimal /= 2; 
    }
    auxBin[i] = decimal;
}

int convBinDec(){
    float dec = 0;
    for(int i = 0; i < 8; i++){
        dec += bin[i]*pow(2, 7 - i);
    }
    return (int)(dec + 1);
}

void alteraSaida(){
    //alterar saida do 595
    for (int i = 0; i < nCIs; ++i){
        digitalWrite(latchPin595, LOW);
        shiftOut(dataPin595, clockPin595, LSBFIRST, pinValuesOut[i]);
        digitalWrite(latchPin595, HIGH);
    }
}

//Mostra os dados recebidos
void display_pin_values()
{
    Serial.print("Estado das entradas:\r\n");

    for(int i = 0; i < nCIs; i++){
        for(int j = 0; j < BYTES; j++){
            Serial.print("  Pin0-");
            Serial.print(i);
            Serial.print(j);
            Serial.print(": ");

            if((pinValues[i] >> j) & 1)
                Serial.print("ALTO");
            else
                Serial.print("BAIXO");

            Serial.print("\r\n");
        }
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

    //reseta o estado dos pinos
    for(int i = 0; i < nCIs; i++){
        pinValues[i] = 0;
        oldPinValues[i] = 0;
        pinValuesOut[i] = 0;
        oldPinValuesOut[i] = 0;
        auxOut[i] = 0;
        auxOutBool[i] = false;
    }

    alteraSaida();
    display_pin_values();
}

//Função do loop principal
void loop(){
    //Lê todos as portas externas

    read_shift_regs();
    
    //Se houver modificação no estado dos pinos, mostra o estado atual
    for(int i = 0; i < nCIs; i++){
        if(pinValues[i] != oldPinValues[i]){
            Serial.print("\n*Alteracao detectada*\r\n");
            display_pin_values();
            Serial.print("Valor das entradas em decimal:\n");
            Serial.print(i);
            Serial.print("\t");
            Serial.print(pinValues[i]);
            oldPinValues[i] = pinValues[i];
        }
    }
    //altera a saída se existir alguma mudança
    for(int i = 0; i < nCIs; i++){
        if (oldPinValuesOut[i] != pinValuesOut[i]){
            alteraSaida();
            oldPinValuesOut[i] = pinValuesOut[i];
        }
    }

    delay(Atraso);
}
