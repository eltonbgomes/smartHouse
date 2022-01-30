#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <A2a.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// DEFINIÇÃO DO ENDEREÇO DO SLAVE
#define address 0x08

// DEFINIÇÃO DO PINO DA TRAVA
#define pinLock 9
#define pinLockSensor 10

// INSTANCIANDO OBJETOS
SoftwareSerial mySerial(11, 12); //pin 11 = Tx do sensor, pin 12 Rx do sensor

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

OneWire oneWire(2);
DallasTemperature sensor(&oneWire);
 
int nTempSensors = 0;
float tempC;

A2a arduinoSlave;

int getFingerprintIDez() {
    uint8_t p = finger.getImage();
    if (p != FINGERPRINT_OK)  return -1;

    p = finger.image2Tz();
    if (p != FINGERPRINT_OK)  return -1;

    p = finger.fingerFastSearch();
    if (p != FINGERPRINT_OK)  return -1;

    //Encontrou uma digital!
    digitalWrite(pinLock, HIGH);
    Serial.print("ID encontrado #"); Serial.print(finger.fingerID); 
    Serial.print(" com confiança de "); Serial.println(finger.confidence);
    Serial.println("Porta Destravada");
    delay(3000);
    return finger.fingerID;
}

void setup() {
    Serial.begin(9600);

    //configuracao do sensor biometrico
    pinMode(pinLock, OUTPUT);
    pinMode(pinLockSensor, INPUT);
    digitalWrite(pinLock, LOW);

    finger.begin(57600);

    bool checkFinger = false;
    while(!checkFinger){
        if(finger.verifyPassword()){
            Serial.println("Sensor biometrico encontrado!");
            checkFinger = true;
        }else{
            Serial.println("Sensor biometrico não encontrado! Verifique a conexão e reinicie o sistema");
            delay(1500);
        }
    }

    // INICIA A COMUNICAÇÃO ENTRE ARDUINOS
    arduinoSlave.begin();

    sensor.begin(); 
    Serial.println("Localizando Dispositivos ...");
    Serial.print("Encontrados ");
    nTempSensors = sensor.getDeviceCount();
    Serial.print(nTempSensors, DEC);
    Serial.println(" dispositivos.");
    Serial.println("\n");
}

void loop() {

    if(digitalRead(pinLockSensor) == HIGH && digitalRead(pinLock) == HIGH){
        digitalWrite(pinLock, LOW);
        Serial.println("Porta Travada");
    }

    getFingerprintIDez();

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
