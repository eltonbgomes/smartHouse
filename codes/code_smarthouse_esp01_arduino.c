/* 
 * Programa para ESP-01
 */
#include <ESP8266WiFi.h>          
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include "secrets.h"
 
IPAddress server_addr(54, 39, 75, 7); // O IP DO SERVIDOR DA CLEVER CLOUD
char user[] = USER_MYSQL;              // Usuario MySQL
char password[] = PASS_MYSQL;        //   Senha MySQL
 
char ssid[] = WIFI_SSID;         //  Nome de rede Wifi
char pass[] = WIFI_PASS;     //            Senha Wi-Fi

WiFiClient client;             
MySQL_Connection conn(&client);
MySQL_Cursor* cursor;

byte statusBD[nCIs];

char query[200];

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
}

void enviaDadosSQL() {
 
    VerificaWiFi();
    if (conn.connect(server_addr, 3306, user, password)) {
        delay(1000);
        MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
        // Save
        // Execute the query
        cur_mem->execute(query);
        // Note: since there are no results, we do not need to read any data
        // Deleting the cursor also frees up memory used
        delete cur_mem;
    }
    conn.close();
}

void aguardaDadosArduino() {
    while (!(Serial.available() > 0)) {}
}

void recebeDadosArduino() {
    byte atual = 255;
    byte i = 0;

    if (Serial.available() > 0) {
        while (atual != 10) {
            if (Serial.available() > 0) {
                atual = Serial.read();
                query[i] = (char)atual;
                i++;
            }
        }
        enviaDadosSQL();
    }
}

void enviaDadosArduino(){
    for (int i = 0; i < nCIs; i++){
        Serial.print(statusBD[i]);
        Serial.print("|");
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
    }
}

void setup() {
    Serial.begin(9600);
    VerificaWiFi();
}
 
void loop() {
    recebeDadosSQL();
    aguardaDadosArduino();
    recebeDadosArduino();

    delay(100);
}