#include <Ethernet.h> //INCLUSÃO DE BIBLIOTECA
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include "DHT.h"
#include "secrets.h"

byte mac_addr[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

IPAddress server_addr(A,B,C,D); // O IP DO SERVIDOR DA CLEVER CLOUD
char user[] = SECRET_USER_MYSQL;  // Usuario MySQL
char password[] = SECRET_PASSWORD_MYSQL; //   Senha MySQL

char INSERT_DATA[] = "INSERT INTO bdqyngbnbsudmj189t37.temperatura (temperatura, umidade) VALUES (%s, %s)"; 

EthernetClient client;
MySQL_Connection conn((Client *)&client);
MySQL_Cursor* cursor;

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void conecta(){
    while(!conn.connect(server_addr, 3306, user, password)){
        Serial.println("Falha na Conexão.");
    };
}

void desconecta(){
    conn.close();
    Serial.println("Conexão Encerrada.");
}

void enviaDHT() {
 
    char query[128];
    char tempString[10];
    char umidString[10];

    float umid = dht.readHumidity();
    float temp = dht.readTemperature();

    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);

    dtostrf(temp, 4, 2, tempString);
    dtostrf(umid, 4, 2, umidString);
    Serial.println(tempString);
    sprintf(query, INSERT_DATA, tempString, umidString);
    
    Serial.println(query);
    cur_mem->execute(query);

    delete cur_mem;
}

void setup(){
    Serial.begin(9600);
    dht.begin();
    while (!Serial); // wait for serial port to connect
    Ethernet.begin(mac_addr);
    Serial.println("Conectando...");
}
void loop(){
    conecta();
    enviaDHT();
    delay(5000);
    desconecta();
}