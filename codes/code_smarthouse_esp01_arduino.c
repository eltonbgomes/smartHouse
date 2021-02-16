/* 
 * Programa para ESP-01
 */
#include <ESP8266WiFi.h>   
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <A2a.h>
#include "secrets.h"

#define endereco 8
#define tempoAtualizacao 1500

A2a arduinoSlave;
 
IPAddress server_addr(54, 39, 75, 7); // O IP DO SERVIDOR DA CLEVER CLOUD
char user[] = USER_MYSQL;              // Usuario MySQL
char password[] = PASS_MYSQL;        //   Senha MySQL
 
char ssid[] = WIFI_SSID;         //  Nome de rede Wifi
char pass[] = WIFI_PASS;     //            Senha Wi-Fi

WiFiClient client;             
MySQL_Connection conn(&client);
MySQL_Cursor* cursor;

byte pinValuesOut[nCIs];
byte statusBD[nCIs];

void recebeDadosSQL(){
    VerificaWiFi();
    if (conn.connect(server_addr, 3306, user, password)) {
        const char querySelect[] = "SELECT status FROM bdqyngbnbsudmj189t37.output";
        MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
        cur_mem->execute(querySelect);
        column_names *cols = cur_mem->get_columns();
        // Read the rows
        row_values *row = NULL;
        int i = 0;
        do {
            if (i < nCIs){
                row = cur_mem->get_next_row();
                if (row != NULL) {
                    for (int f = 0; f < cols->num_fields; f++) {
                        statusBD[i] = atoi(row->values[f]);
                    }
                }
            }
            i++;
        } while (row != NULL);
        delete cur_mem;
        enviaDadosArduino();
    }
    Serial.println("statusBD recebeDadosSQL");
    for (int i = 0; i < nCIs; i++){
        Serial.println(statusBD[i]);
    }
    Serial.println();
}

void enviaDadosOutSQL(int i) {
    VerificaWiFi();
    if (conn.connect(server_addr, 3306, user, password)) {
        char UPDATE_DATA[] = "UPDATE bdqyngbnbsudmj189t37.output SET status=%d where id_output=%d";
        char queryStatus[128];

        sprintf(queryStatus, UPDATE_DATA, pinValuesOut[i], i+1);

        MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);

        cur_mem->execute(queryStatus);

        delete cur_mem;

        Serial.println("query");
        Serial.println(queryStatus);
        Serial.println();
    }
    conn.close();
}

void recebeDadosArduino() {
    for (int i = 0; i < nCIs; i++){
        pinValuesOut[i] = arduinoSlave.varWireRead(endereco, i);
        if (pinValuesOut[i] != statusBD[i]){
            enviaDadosOutSQL(i);
            delay(1500);
        }
    }
    Serial.println("statusBD no recebeDadosArduino");
    for (int i = 0; i < nCIs; i++){
        Serial.println(statusBD[i]);
    }
    Serial.println();
}

void enviaDadosArduino(){
    for (int i = 0; i < nCIs; i++){
        arduinoSlave.varWireWrite(endereco, i, statusBD[i]);
    }
    Serial.println("statusBD enviaDadosArduino");
    for (int i = 0; i < nCIs; i++){
        Serial.println(statusBD[i]);
    }
    Serial.println();
}
 
void VerificaWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.disconnect();
        delay(1000);
        WiFi.begin(ssid, pass);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
        }
        Serial.println("WIFI OK");
    }else{
        Serial.println("FALHA WIFI");
    }
}

void setup() {
    Serial.begin(9600);
    while (! Serial);
    VerificaWiFi();
    arduinoSlave.begin(0, 2);
}
 
void loop() {
    recebeDadosSQL();

    recebeDadosArduino();

    delay(tempoAtualizacao);
}