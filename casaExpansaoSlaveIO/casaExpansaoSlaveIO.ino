/********************************************************************************\
 * Programa Expansão de Entradas e saídas do Arduino, utilizando Shift Register *
 * CI utilizado: 74HC165 com 74HC595                                            *
 * Por: Elton Barbosa Gomes                                                     *
 * Data: 28/09/2020                                                             *
 * Créditos: Baseado no playground.arduino.cc                                   *
\********************************************************************************/

/*Variaveis I2C ligar A4 e A5 entre arduinos
 *Variavel 0 -> valor da saida para o IC 0
 *Variavel 1 -> valor da saida para o IC 1
 *Variavel 2 -> valor da saida para o IC 2
 *Variavel 3 -> valor do portal AdaFruit para stualizar o IC 0
 *Variavel 4 -> valor do portal AdaFruit para stualizar o IC 1
 *Variavel 5 -> valor do portal AdaFruit para stualizar o IC 2
 *Variavel 6
 *Variavel 7
 *Variavel 8
 *Variavel 9 -> variavel usada para atualizar valores do master quando houver mudança no I2C
 *Variavel 10 -> temperatura externa HighByte
 *Variavel 11 -> temperatura externa LowByte
 *Variavel 12 -> temperatura interna HighByte
 *Variavel 13 -> temperatura interna LowByte
 *Variavel 14 ->
 *Variavel 15 ->
 *Variavel 16
 *Variavel 17
 *Variavel 18
 *Variavel 19 -> variavel usada para atualizar valores do master quando houver mudança no I2C
*/

#include <A2a.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define boolArduinoMaster 9        //variavel usada para atualizar valores do master quando houver mudança no I2C
//74hc165
// Definições de Labels
#define nICs  3              //Registra o número de CIs cascateados
#define BYTES 8
#define readTime 50          //Registra o tempo de que deverá ter o pulso para leitura e gravação, (milesegundos)
#define DELAY  100           //Registra o atraso de segurança entre leituras, (milesegundos)
#define address 0x08         //address usado na comunicacao I2C entre arduino
#define buttonTime 750       //tempo de intervalo segura botao

// Declaração de constantes globais 165
const int ploadPin165        = 6;   //Conecta ao pino 1 do 74HC165 (LH/LD - asynchronous parallel load input)(PL)
const int clockEnablePin165  = 5;   //Conecta ao pino 15 do 74HC165 (CE - Clock Enable Input)(CE)
const int dataPin165         = 8;   //Conecta ao pino 9 do 74HC165 (Q7 - serial output from the last stage)(Q7)
const int clockPin165        = 7;   //Conecta ao pino 2 do 74HC165 (CP - Clock Input)(CP)

const int pinDataTemp        = 9;   //Sensor temperatura

int nTempSensors = 0; //quantidade de sensores

//inicialização das variaveis onde serão armazenados os status
byte pinValues[nICs];
byte oldPinValues[nICs];
byte helpOut[nICs]; // variavel usada para armazenar valores das saídas para serem alteradas na borda de descida
bool helpOutBool[nICs];

bool alterSlave = false; //variavel para alterar as saidas
bool alterMaster = false; //variavel para alterar no Master e enviar para nuvem

//auxiliares para impedir o acesso de condicionais quando ocorrer o desligamento por tempo
bool helpByte = false;
bool helpAll = false;

// Declaração de constantes globais 595
const int clockPin595 = 2; //Pino conectado a SRCLK (pino 11 no 74HC595), registrador de deslocamento
const int latchPin595 = 3; //Pino conectado a RCLK (pino 12 no 74HC595), registrador de armazenamento
const int dataPin595  = 4; //Pino conectado a SER (pino 14 no 74HC595), entrada de dados serial

//usado para contar o tempo do botao pressionado
unsigned long time;
unsigned long timeTemp; //time para temperatura

//inicia objeto para comunicacao arduino
A2a arduinoMaster;

OneWire oneWire(pinDataTemp);
DallasTemperature sensor(&oneWire);

void saveData(float data, int a, int b){ //data = dados, a e b são locais onde são salvos
    int dataInt = data * 100;
    arduinoMaster.varWireWrite(a, highByte(dataInt));
    arduinoMaster.varWireWrite(b, lowByte(dataInt));
}

void readTemp(){
    float tempC[nTempSensors];
    sensor.requestTemperatures();
    for (int i = 0;  i < nTempSensors;  i++) {
        tempC[i] = sensor.getTempCByIndex(i);
        if(i == 1 || i == 3){
            saveData((tempC[i - 1] + tempC[i])/2, i - 1 , i); //media
        }
    }
    timeTemp = millis();
}

//Função para definir um rotina shift-in, lê os dados do 74HC165
void read_shift_regs(){
    bool bitVal = 0; //variavel para que armazena bit a bit
    byte bytesVal[nICs];
    byte bytesValOut[nICs];
    for(int i = 0; i < nICs; i++){
        bytesVal[i] = 0;
        bytesValOut[i] = arduinoMaster.varWireRead(i);
    }

    //desloca todos os bits para o pino de dados para leitura
    digitalWrite(clockEnablePin165, HIGH);
    digitalWrite(ploadPin165, LOW);
    delayMicroseconds(readTime);
    digitalWrite(ploadPin165, HIGH);
    digitalWrite(clockEnablePin165, LOW);

    // Efetua a leitura de um bit da saida serial do 74HC165

    for(int IC = nICs - 1; IC >= 0; IC--){
        for (int j = 0; j < BYTES; j++){
            bitVal = digitalRead(dataPin165);

            //Realiza um shift left e armazena o bit correspondente em bytesVal
            bytesVal[IC] |= (bitVal << ((BYTES-1) - j));

            //compara se ouve mudanca para alterar o 595
            bytesValOut[IC] ^= (bitVal << ((BYTES-1) - j));

            //Lança um pulso de clock e desloca o próximo bit
            digitalWrite(clockPin165, HIGH);
            delayMicroseconds(readTime);
            digitalWrite(clockPin165, LOW);
        }
        pinValues[IC] = bytesVal[IC];

        //condição para somente alterar as saídas se soltar o botão
        if(pinValues[IC] != oldPinValues[IC] && !helpOutBool[IC]){
            time = millis();
            helpOut[IC] = bytesValOut[IC];
            helpOutBool[IC] = true;
            helpByte = helpAll = true;
            oldPinValues[IC] = pinValues[IC];
        }

        //condicao para impedir a alteração com o botao pressionado
        if(pinValues[IC] == 0 && helpOutBool[IC]){
            if(helpOut[IC] != arduinoMaster.varWireRead(IC)){
                arduinoMaster.varWireWrite(IC, helpOut[IC]);
                alterSlave = true;
            }
            helpOutBool[IC] = false;
        }

        //desliga as saidas após tempo pressionado
        if((millis() - time) > buttonTime && helpOutBool[IC] && helpByte){
            arduinoMaster.varWireWrite(IC, 0);
            helpOut[IC] = arduinoMaster.varWireRead(IC);
            helpByte = false;
            alterSlave = true;
        }

        //desliga todas as saidas após tempo pressionado
        if((millis() - time) > buttonTime * 2 && helpOutBool[IC] && helpAll){
            for(int i = 0; i < nICs; i++){
                arduinoMaster.varWireWrite(i, 0);
            }
            helpOut[IC] = arduinoMaster.varWireRead(IC);
            helpAll = false;
            alterSlave = true;
        }
    }
}

void alterOut(){
    //alterar saida do 595
    for (int i = 0; i < nICs; ++i){
        digitalWrite(latchPin595, LOW);
        shiftOut(dataPin595, clockPin595, LSBFIRST, arduinoMaster.varWireRead(i));
        digitalWrite(latchPin595, HIGH);
    }
}

void checkStatusMaster(){
    for (int i = 0; i < nICs; ++i){
        if(!alterSlave && (arduinoMaster.varWireRead(i) != arduinoMaster.varWireRead(i + 3))){
            arduinoMaster.varWireWrite(i, arduinoMaster.varWireRead(i + 3));
            alterMaster = true;
        }
    }
}

void receiveData() {
    arduinoMaster.receiveData(); 
}

void sendData() {
    arduinoMaster.sendData(); 
}

// Configuração do Programa
void setup(){

    // INICIA A COMUNICAÇÃO ENTRE ARDUINOS COMO SLAVE NO ENDEREÇO DEFINIDO
    arduinoMaster.begin(address);

    // FUNÇÕES PARA COMUNICAÇÃO
    arduinoMaster.onReceive(receiveData);
    arduinoMaster.onRequest(sendData);

    sensor.begin();
    nTempSensors = sensor.getDeviceCount();

    arduinoMaster.varWireWrite(boolArduinoMaster, false);
    
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
    for(int i = 0; i < nICs; i++){
        pinValues[i] = 0;
        oldPinValues[i] = 0;
        arduinoMaster.varWireWrite(i, 0);
        arduinoMaster.varWireWrite(i + 3, 0);
        helpOut[i] = 0;
        helpOutBool[i] = false;
    }

    alterOut();
}

//Função do loop principal
void loop(){
    //Lê todos as portas externas

    read_shift_regs();

    if(millis() - timeTemp > 10000){
        readTemp();
    }

    checkStatusMaster();
    
    //altera a saída se existir alguma mudança
    if(alterSlave){
        arduinoMaster.varWireWrite(boolArduinoMaster, true);
        alterOut();
        alterSlave = false;
    }

    //altera a saída se existir alguma mudança no Master (nuvem)
    if(alterMaster){
        alterOut();
        alterMaster = false;
    }

    delay(DELAY);
}
