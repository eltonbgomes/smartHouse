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

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
unsigned long time = millis();

// Declaração de constantes globais 74HC165
const int ploadPin165        = 6;   //Conecta ao pino 1 do 74HC165 (LH/LD - asynchronous parallel load input)(PL)
const int clockEnablePin165  = 3;   //Conecta ao pino 15 do 74HC165 (CE - Clock Enable Input)(CE)
const int dataPin165         = 8;   //Conecta ao pino 9 do 74HC165 (Q7 - serial output from the last stage)(Q7)
const int clockPin165        = 7;   //Conecta ao pino 2 do 74HC165 (CP - Clock Input)(CP)

// Declaração de constantes globais 74HC595
const int clockPin595 = A0; //Pino conectado a SRCLK (pino 11 no 74HC595), registrador de deslocamento
const int latchPin595 = A1; //Pino conectado a RCLK (pino 12 no 74HC595), registrador de armazenamento
const int dataPin595 = A2; //Pino conectado a SER (pino 14 no 74HC595), entrada de dados serial

//inicialização dos vetores onde serão armazenados os bytes
byte pinValues[nCIs];
byte oldPinValues[nCIs];
byte pinValuesOut[nCIs];
byte oldPinValuesOut[nCIs];
byte statusBD[nCIs];
bool altera = false;
bool alteraSQL = false;

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
    conecta();
    char UPDATE_DATA[] = "UPDATE bdqyngbnbsudmj189t37.output SET status=%d where id_output=%d";
    char queryStatus[128];

    sprintf(queryStatus, UPDATE_DATA, pinValuesOut[i], i+1);

    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
    
    cur_mem->execute(queryStatus);

    delete cur_mem;
    alteraSQL = false;
}

void enviaDHT() {
    conecta();

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

    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);

    cur_mem->execute(queryDHT);

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
        if((pinValues[nCIs-1-i] != oldPinValues[nCIs-1-i]) && (pinValuesOut[nCIs-1-i] != bytesValOut[nCIs-1-i]) && !altera){
            pinValuesOut[nCIs-1-i] = bytesValOut[nCIs-1-i];
            altera = true;
            alteraSQL = true;
        }
    }
}

void alteraSaida(){
    //alterar saida do 595
    for (int i = 0; i < nCIs; ++i){
        digitalWrite(latchPin595, LOW);
        shiftOut(dataPin595, clockPin595, LSBFIRST, pinValuesOut[i]);
        digitalWrite(latchPin595, HIGH);
    }
    altera = false;
}

void comunicacao(){
    conecta();
    const char query[] = "SELECT status FROM bdqyngbnbsudmj189t37.output";
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
    cur_mem->execute(query);
    column_names *cols = cur_mem->get_columns();
    // Read the rows
    row_values *row = NULL;
    int i = 0;
    do {
        if (i < 3){
            row = cur_mem->get_next_row();
            if (row != NULL) {
                for (int f = 0; f < cols->num_fields; f++) {
                    statusBD[i] = atoi(row->values[f]);
                }
            }
            if ((pinValuesOut[i] != statusBD[i]) && !altera){
                pinValuesOut[i] = statusBD[i];
                altera = true;
            }
        }
        i++;
    } while (row != NULL);
    delete cur_mem;
}

// Configuração do Programa
void setup(){
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
    }

    //altera as saidas para desligadas
    alteraSaida();
    enviaDHT();
}

//Função do loop principal
void loop(){
    //Lê todos as portas externas
    read_shift_regs();

    if(!alteraSQL){
        comunicacao();
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
                    enviaStatus(i);
                }
                oldPinValuesOut[i] = pinValuesOut[i];
                break;
            }
        }
    }

    if((millis() - time) > 600000){
        time = millis();
        enviaDHT();
    }
    delay(Atraso);
}
