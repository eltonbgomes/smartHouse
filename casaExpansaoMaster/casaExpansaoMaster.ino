#include <A2a.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// DEFINIÇÃO DO ENDEREÇO DO SLAVE
#define address 0x08


OneWire oneWire(9);
DallasTemperature sensor(&oneWire);
 
int nTempSensors = 0;
float tempC;

A2a arduinoSlave;

void setup() {
    // INICIA A COMUNICAÇÃO ENTRE ARDUINOS
    arduinoSlave.begin();

    sensor.begin(); 
    Serial.begin(9600);
    Serial.println("Localizando Dispositivos ...");
    Serial.print("Encontrados ");
    nTempSensors = sensor.getDeviceCount();
    Serial.print(nTempSensors, DEC);
    Serial.println(" dispositivos.");
    Serial.println("\n");
}

void loop() {
    Serial.print("\nStatus\n");
    for(int i = 0; i < 3; i++){
        Serial.print(arduinoSlave.varWireRead(address, i));
        Serial.print("\n");
    }

    sensor.requestTemperatures(); 
 
    for (int i = 0;  i < nTempSensors;  i++) {
        Serial.print("Sensor ");
        Serial.print(i+1);
        Serial.print(": ");
        tempC = sensor.getTempCByIndex(i);
        Serial.print(tempC);
        Serial.println("ºC");
    }
}
