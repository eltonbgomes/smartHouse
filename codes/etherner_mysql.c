#include <Ethernet.h> //INCLUSÃO DE BIBLIOTECA
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include "secrets.h"

byte mac_addr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress server_addr(A,B,C,D); // O IP DO SERVIDOR DA CLEVER CLOUD
char user[] = SECRET_USER_MYSQL;  // Usuario MySQL
char password[] = SECRET_PASSWORD_MYSQL; //   Senha MySQL

char INSERT_DATA[] = "INSERT INTO bdqyngbnbsudmj189t37.temperatura (temperatura) VALUES (%s)"; 

EthernetClient client;
MySQL_Connection conn((Client *)&client);
MySQL_Cursor* cursor;

float temperature;

void EnviaDados(float temp) {
 
    char query[128];
    char temperatura[10];

    if (conn.connect(server_addr, 3306, user, password)) {
        delay(1000);
        MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
        // Save
        dtostrf(temp, 4, 1, temperatura);
        sprintf(query, INSERT_DATA, temperatura);
        // Execute the query
        cur_mem->execute(query);
        // Note: since there are no results, we do not need to read any data
        // Deleting the cursor also frees up memory used
        delete cur_mem;
        Serial.println(); Serial.println("Data recorded.");
        Serial.println("Dados.");
        Serial.println(query);
    }else{
        Serial.println(); Serial.println("Connection failed.");
    }
    conn.close();
}


void setup(){
    Serial.begin(9600);
    while (!Serial); // wait for serial port to connect
    Ethernet.begin(mac_addr);
}
void loop(){
    temperature = 131.9;
    EnviaDados(temperature);
    delay(2000);
}