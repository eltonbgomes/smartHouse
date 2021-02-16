/********************************************************************************\
 * Expansão de Entradas e saídas do Arduino, utilizando Shift Register          *
 * CI utilizado: 74HC165 e 74HC595                                              *
 * Autor: Elton Barbosa Gomes                                                   *
 * Data: 15/01/2020                                                             *
 * Créditos: Baseado no playground.arduino.cc                                   *
\********************************************************************************/
#include "DHT.h"
#include <A2a.h>
#include "secrets.h"

A2a arduinoMaster;

//74hc165
// Definições de constantes
#define BYTES 8
#define TempoDeslocamento 50  //Registra o tempo de que deverá ter o pulso para leitura e gravação, (milesegundos)
#define Atraso  100           //Registra o atraso de segurança entre leituras, (milesegundos)
#define endereco 8

#define DHTPIN A0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
unsigned long time = millis();

// Declaração de constantes globais 74HC165
const int ploadPin165        = 6;   //Conecta ao pino 1 do 74HC165 (LH/LD - asynchronous parallel load input)(PL)
const int clockEnablePin165  = 5;   //Conecta ao pino 15 do 74HC165 (CE - Clock Enable Input)(CE)
const int dataPin165         = 8;   //Conecta ao pino 9 do 74HC165 (Q7 - serial output from the last stage)(Q7)
const int clockPin165        = 7;   //Conecta ao pino 2 do 74HC165 (CP - Clock Input)(CP)

// Declaração de constantes globais 74HC595
const int clockPin595 = 2; //Pino conectado a SRCLK (pino 11 no 74HC595), registrador de deslocamento
const int latchPin595 = 3; //Pino conectado a RCLK (pino 12 no 74HC595), registrador de armazenamento
const int dataPin595 = 4; //Pino conectado a SER (pino 14 no 74HC595), entrada de dados serial

//inicialização dos vetores onde serão armazenados os bytes
byte pinValues[nCIs];
byte oldPinValues[nCIs];
byte pinValuesOut[nCIs];
byte oldPinValuesOut[nCIs];
byte statusBD[nCIs];
bool altera = false;
bool alteraSQL = false;

void receberDados() {
  arduinoMaster.receiveData(); 
}

void enviarDados() {
  arduinoMaster.sendData(); 
}

void recebeDadosESP(){
    for (int i = 0; i < nCIs; i++){
        statusBD[i] = arduinoMaster.varWireRead(i);
        if ((pinValuesOut[i] != statusBD[i]) && !altera){
            pinValuesOut[i] = statusBD[i];
            altera = true;
        }
        Serial.println("statusBD[i]");
        Serial.println(statusBD[i]);
    }
}

void enviaStatus(){
    for (int i = 0; i < nCIs; i++){
        arduinoMaster.varWireWrite(i, pinValuesOut[i]);
    }
    alteraSQL = false;
}

void enviaDHT() {

    char INSERT_DATA[] = "INSERT INTO bdqyngbnbsudmj189t37.temperatura (data, hora, temperatura, umidade, indice_calor) VALUES (CURDATE(), CURTIME(), %s, %s, %s)"; 
    char query[128];
 
    char queryDHT[200];
    char tempString[6];
    char umidString[6];
    char hicString[6];

    float temp = dht.readTemperature();
    float umid = dht.readHumidity();
    float hic = dht.computeHeatIndex(temp, umid, false);

    dtostrf(temp, 5, 2, tempString);
    dtostrf(umid, 5, 2, umidString);
    dtostrf(hic, 5, 2, hicString);

    sprintf(query, INSERT_DATA, tempString, umidString, hicString);

    Serial.println(query);

    enviaESP();
}

void enviaESP(){

}

//Função para leitura dos dados do 74HC165
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

    // Efetua a leitura dos bits da saida serial do 74HC165
    for(int i = 0; i < nCIs; i++){
        for (int j = 0; j < BYTES; j++){
            bitVal = digitalRead(dataPin165);

            //Realiza um shift left e armazena o bit correspondente em bytesVal
            bytesVal[nCIs-1-i] |= (bitVal << ((BYTES-1) - j));

            //compara se ouve mudanca de borda de subida para alterar o 595
            bytesValOut[nCIs-1-i] ^= (bitVal << ((BYTES-1) - j));

            //Lança um pulso de clock e desloca o próximo bit
            digitalWrite(clockPin165, HIGH);
            delayMicroseconds(TempoDeslocamento);
            digitalWrite(clockPin165, LOW);
        }
        pinValues[nCIs-1-i] = bytesVal[nCIs-1-i];

        //altera somente uma vez a saida com o botao pressionado
        if((pinValues[nCIs-1-i] != oldPinValues[nCIs-1-i]) && (pinValuesOut[nCIs-1-i] != bytesValOut[nCIs-1-i]) && !altera){
            pinValuesOut[nCIs-1-i] = bytesValOut[nCIs-1-i];
            altera = true;
            alteraSQL = true;
        }
    }
}

void alteraSaida(){
    //alterar saida do 595
    Serial.println("PINVALUES");
    for (int i = 0; i < nCIs; ++i){
        digitalWrite(latchPin595, LOW);
        shiftOut(dataPin595, clockPin595, LSBFIRST, pinValuesOut[i]);
        digitalWrite(latchPin595, HIGH);
        Serial.println("pinValuesOut[i]");
        Serial.println(pinValuesOut[i]);
    }
    altera = false;
}

// Configuração do Programa
void setup(){
    arduinoMaster.begin(endereco);
    arduinoMaster.onReceive(receberDados);
    arduinoMaster.onRequest(enviarDados);

    Serial.begin(9600);
    dht.begin();

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

    digitalWrite(latchPin595, HIGH);

    //inicializa com as saidas desligadas
    for(int i = 0; i < nCIs; i++){
        pinValues[i] = 0;
        oldPinValues[i] = 0;
        pinValuesOut[i] = 0;
        oldPinValuesOut[i] = 0;
        statusBD[i] = 0;
    }

    //altera as saidas para desligadas
    delay(3000);
    alteraSaida();
    enviaDHT();
}

//Função do loop principal
void loop(){
    //Lê todos as portas externas
    read_shift_regs();

    if(!alteraSQL){
        recebeDadosESP();  
    }

    //Se houver modificação no estado dos pinos, mostra o estado atual
    for(int i = 0; i < nCIs; i++){
        if(pinValues[i] != oldPinValues[i]){
            oldPinValues[i] = pinValues[i];
        }
    }
    //altera a saída se existir alguma mudança
    if (altera){
        for(int i = 0; i < nCIs; i++){
            if (oldPinValuesOut[i] != pinValuesOut[i]){
                alteraSaida();
                if (alteraSQL){
                    delayMicroseconds(Atraso);
                    enviaStatus();
                }
                oldPinValuesOut[i] = pinValuesOut[i];
                break;
            }
        }
    }

    if((millis() - time) > 10000){
        time = millis();
        enviaDHT();
    }
    delay(Atraso);
}
