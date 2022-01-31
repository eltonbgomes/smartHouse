#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <A2a.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//variavel usada para atualizar valores do master quando houver mudança no I2C
#define boolArduino 9

//Registra o número de CIs cascateados
#define nICs 3
// DEFINIÇÃO DO ENDEREÇO DO SLAVE
#define address 0x08

//numero de sensores de temperatura
#define nTempSensors 2

// DEFINIÇÃO DO PINO DA TRAVA
#define pinBioSensor 8 //saida para ligar o sensor biometrico
#define pinLock 9 //saida para travar ou destravar a porta
#define pinLockSensor 10 // input do sensor fechadura

//sensor de luminosidade
#define pinLuxSensor A0

#define delayTime 1000

int lux = 0;

// INSTANCIANDO OBJETOS
SoftwareSerial mySerial(11, 12); //pin 11 = Tx do sensor, pin 12 Rx do sensor

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

OneWire oneWire(2);
DallasTemperature sensor(&oneWire);
 
float tempC[nTempSensors];
unsigned long time;

A2a arduinoSlave;

int getFingerprintIDez() {
    uint8_t p = finger.getImage();
    if (p != FINGERPRINT_OK)  return -1;

    p = finger.image2Tz();
    if (p != FINGERPRINT_OK)  return -1;

    p = finger.fingerFastSearch();
    if (p != FINGERPRINT_OK){
        Serial.println("Digital NAO reconhecida");
        return -1;
    }

    //Encontrou uma digital!
    digitalWrite(pinLock, HIGH);
    Serial.print("ID encontrado #"); Serial.print(finger.fingerID); 
    Serial.print(" com confiança de "); Serial.println(finger.confidence);
    Serial.println("Porta Destravada");
    delay(delayTime*3);
    digitalWrite(pinBioSensor, LOW);
    return finger.fingerID;
}

void readTemp(){
    sensor.requestTemperatures();
    for (int i = 0;  i < nTempSensors;  i++) {
        Serial.print("Sensor ");
        Serial.print(i+1);
        Serial.print(": ");
        tempC[i] = sensor.getTempCByIndex(i);
        Serial.print(tempC[i]);
        Serial.println("ºC");
    }
    readLux();
    time = millis();
}

void readStatusSlave(){
    Serial.println("\nStatus");
    for(int i = 0; i < nICs; i++){
        Serial.println(arduinoSlave.varWireRead(address, i));
    }
}

void readLux(){
    lux = analogRead(pinLuxSensor);
    // print out the value you read:
    Serial.println("\nLuminosidade: ");
    Serial.println(lux);
}

void setup() {
    Serial.begin(9600);
    // INICIA A COMUNICAÇÃO ENTRE ARDUINOS
    arduinoSlave.begin();
    sensor.begin();

    readTemp();

    //configuracao do sensor biometrico
    pinMode(pinLock, OUTPUT);
    digitalWrite(pinLock, LOW);
    pinMode(pinLockSensor, INPUT);
    pinMode(pinBioSensor, OUTPUT);
    digitalWrite(pinBioSensor, HIGH);

    //pino sensor de luminosidade
    pinMode(pinLuxSensor,INPUT);

    finger.begin(57600);

    bool checkFinger = false;
    while(!checkFinger){
        if(finger.verifyPassword()){
            Serial.println("Sensor biometrico encontrado!");
            checkFinger = true;
        }else{
            Serial.println("Sensor biometrico não encontrado! Verifique a conexão e reinicie o sistema");
            delay(delayTime*1.5);
        }
    }
    delay(delayTime);
    digitalWrite(pinBioSensor, LOW);
}

void loop() {

    //condição para travar a porta
    if(digitalRead(pinLockSensor) == HIGH && digitalRead(pinLock) == HIGH){
        delay(delayTime*3);
        if(digitalRead(pinLockSensor) == HIGH){
            digitalWrite(pinLock, LOW);
            Serial.println("Porta Travada");
        }
    }

    //condição para ligar o sensor biometrico
    if(digitalRead(pinLockSensor) == HIGH){
        digitalWrite(pinBioSensor, HIGH);
        getFingerprintIDez();
    }

    //condição para fazer a leitura de dados do slave
    if(arduinoSlave.varWireRead(address, boolArduino)){
        readStatusSlave();
        arduinoSlave.varWireWrite(address, boolArduino, false);
    }

    //fazer a leiura dos sensores
    if(millis() - time > 45000){
        readTemp();
    }
}
