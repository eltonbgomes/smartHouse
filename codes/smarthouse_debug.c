/********************************************************************************\
 * Expansão de Entradas e saídas do Arduino, utilizando Shift Register          *
 * CI utilizado: 74HC165 e 74HC595                                              *
 * Autor: Elton Barbosa Gomes                                                   *
 * Data: 15/01/2020                                                             *
 * Créditos: Baseado no playground.arduino.cc                                   *
\********************************************************************************/
#include <Ethernet.h> //INCLUSÃO DE BIBLIOTECA
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include "DHT.h"
#include "secrets.h"

byte mac_addr[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

IPAddress server_addr(A,B,C,D); // O IP DO SERVIDOR DA CLEVER CLOUD
char user[] = SECRET_USER_MYSQL;  // Usuario MySQL
char password[] = SECRET_PASSWORD_MYSQL; //   Senha MySQL

EthernetClient client;
MySQL_Connection conn((Client *)&client);
MySQL_Cursor* cursor;

//74hc165
// Definições de constantes
#define nCIs  3               //Registra o número de CIs cascateados
#define BYTES 8
#define TempoDeslocamento 50  //Registra o tempo de que deverá ter o pulso para leitura e gravação, (milesegundos)
#define Atraso  100           //Registra o atraso de segurança entre leituras, (milesegundos)

#define DHTPIN A0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
unsigned long time = millis();

// Declaração de constantes globais 74HC165
const int ploadPin165        = 30;   //Conecta ao pino 1 do 74HC165 (LH/LD - asynchronous parallel load input)(PL)
const int clockEnablePin165  = 28;   //Conecta ao pino 15 do 74HC165 (CE - Clock Enable Input)(CE)
const int dataPin165         = 34;   //Conecta ao pino 9 do 74HC165 (Q7 - serial output from the last stage)(Q7)
const int clockPin165        = 32;   //Conecta ao pino 2 do 74HC165 (CP - Clock Input)(CP)

// Declaração de constantes globais 74HC595
const int clockPin595 = 22; //Pino conectado a SRCLK (pino 11 no 74HC595), registrador de deslocamento
const int latchPin595 = 24; //Pino conectado a RCLK (pino 12 no 74HC595), registrador de armazenamento
const int dataPin595 = 26; //Pino conectado a SER (pino 14 no 74HC595), entrada de dados serial

//inicialização dos vetores onde serão armazenados os bytes
byte pinValues[nCIs];
byte oldPinValues[nCIs];
byte pinValuesOut[nCIs];
byte oldPinValuesOut[nCIs];

void conecta(){
    if(!conn.connect(server_addr, 3306, user, password)){
        Serial.println("Falha na Conexão.");
    }
}

void desconecta(){
    conn.close();
    Serial.println("Conexão Encerrada.");
}

void enviaStatus(int i){
    char UPDATE_DATA[] = "UPDATE bdqyngbnbsudmj189t37.output SET status=%d where id_output=%d";

    char status[4];
    char indice[1];
    char queryStatus[128];

    Serial.println("pinvalues ANTES");
    Serial.println(pinValuesOut[i]);

    dtostrf(pinValuesOut[i], 3, 0, status);
    dtostrf(i + 1, 1, 0, indice);
    
    Serial.println("pinvalues");
    Serial.println(pinValuesOut[i]);
    Serial.println(status);
    Serial.println(indice);

    sprintf(queryStatus, UPDATE_DATA, status, indice);
    conecta();
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
    
    cur_mem->execute(queryStatus);
    desconecta();
    Serial.println(queryStatus);

    delete cur_mem;
}

void enviaDHT() {

    char INSERT_DATA[] = "INSERT INTO bdqyngbnbsudmj189t37.temperatura (data, hora, temperatura, umidade, indice_calor) VALUES (CURDATE(), CURTIME(), %s, %s, %s)"; 
 
    char queryDHT[128];
    char tempString[6];
    char umidString[6];
    char hicString[6];

    float temp = dht.readTemperature();
    float umid = dht.readHumidity();
    float hic = dht.computeHeatIndex(temp, umid, false);

    dtostrf(temp, 5, 2, tempString);
    dtostrf(umid, 5, 2, umidString);
    dtostrf(hic, 5, 2, hicString);

    sprintf(queryDHT, INSERT_DATA, tempString, umidString, hicString);

    conecta();
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
    
    Serial.println(queryDHT);
    cur_mem->execute(queryDHT);
    desconecta();

    delete cur_mem;
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
        if(pinValues[nCIs-1-i] != oldPinValues[nCIs-1-i]){
            pinValuesOut[nCIs-1-i] = bytesValOut[nCIs-1-i];
        }
    }
}

void alteraSaida(){
    //alterar saida do 595

    Serial.println("Valor de PINVALUESOUT no momento de ir para o CI");
    for (int i = 0; i < nCIs; ++i){
        Serial.println(pinValuesOut[i]);
        digitalWrite(latchPin595, LOW);
        shiftOut(dataPin595, clockPin595, LSBFIRST, pinValuesOut[i]);
        digitalWrite(latchPin595, HIGH);
    }
}

//Mostra os dados recebidos
void display_pin_values(){
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

void comunicacao(){
    
}

// Configuração do Programa
void setup(){
    Serial.begin(9600);
    dht.begin();
    Ethernet.begin(mac_addr);

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

    //inicializa com as saidas desligadas
    for(int i = 0; i < nCIs; i++){
        pinValues[i] = 0;
        oldPinValues[i] = 0;
        pinValuesOut[i] = 0;
        oldPinValuesOut[i] = 0;    
    }

    //altera as saidas para desligadas
    alteraSaida();
    //mostra no monitor
    display_pin_values();
    enviaDHT();
}

//Função do loop principal
void loop(){
    comunicacao();//faz a comunicacao caso ocorra solicitacao
    //Lê todos as portas externas
    read_shift_regs();
    
    //Se houver modificação no estado dos pinos, mostra o estado atual
    for(int i = 0; i < nCIs; i++){
        if(pinValues[i] != oldPinValues[i]){
            Serial.print("*Alteracao detectada*\r\n");
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
            delayMicroseconds(Atraso);
            enviaStatus(i);
            oldPinValuesOut[i] = pinValuesOut[i];
        }
    }

    if((millis() - time) > 1800000){
        time = millis();
        enviaDHT();
    }

    delay(Atraso);
}
