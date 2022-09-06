/********************************************************************************\
 * Programa Expansão de Entradas e saídas do Arduino, utilizando Shift Register *
 * CI utilizado: 74HC165 com 74HC595                                            *
 * Por: Elton Barbosa Gomes                                                     *
 * Data: 05/09/2022                                                             *
 * Créditos: Baseado no playground.arduino.cc                                   *
\********************************************************************************/
//ligar A5 e A4 dos arduinos (utilização da comunicação I2C)

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

//Porta sensor temperatura
#define pinDataTemp 2

// DEFINIÇÃO DO PINO DA TRAVA
#define pinOutputBioSensor 3 //saida para ligar o sensor biometrico
#define pinInputButtonLock 4 //entrada para travar porta
#define pinOutputLockDoor 5 //saida para travar ou destravar a porta
#define pinInputLockSensor 6 // input do sensor fechadura

//sensor de luminosidade
#define pinAnalogLuxSensor A0

#define delayTime 1000

int lux = 0;

// INSTANCIANDO OBJETOS
SoftwareSerial mySerial(8, 9); //pin 8 = Tx do sensor, pin 9 Rx do sensor (colocar divisor tensao)

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

OneWire oneWire(pinDataTemp);
DallasTemperature sensor(&oneWire);
 
float tempC[nTempSensors];
unsigned long time;

A2a arduinoSlave;

int getFingerprintID() {
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
    digitalWrite(pinOutputLockDoor, HIGH);
    Serial.print("ID encontrado #"); Serial.print(finger.fingerID); 
    Serial.print(" com confiança de "); Serial.println(finger.confidence);
    Serial.println("Porta Destravada");
    delay(delayTime*5);
    digitalWrite(pinOutputBioSensor, LOW);
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
    lux = analogRead(pinAnalogLuxSensor);
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
    pinMode(pinOutputLockDoor, OUTPUT);
    digitalWrite(pinOutputLockDoor, LOW);
    pinMode(pinInputLockSensor, INPUT);
    pinMode(pinInputButtonLock, INPUT);
    pinMode(pinOutputBioSensor, OUTPUT);
    digitalWrite(pinOutputBioSensor, HIGH);

    //pino sensor de luminosidade
    pinMode(pinAnalogLuxSensor,INPUT);

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
    digitalWrite(pinOutputBioSensor, LOW);
}

void loop() {

    //condição para travar a porta
    if(digitalRead(pinOutputLockDoor) == HIGH && digitalRead(pinInputLockSensor) == HIGH && digitalRead(pinInputButtonLock) == HIGH){
        delay(delayTime*3);
        if(digitalRead(pinInputLockSensor) == HIGH){
            digitalWrite(pinOutputLockDoor, LOW);
            Serial.println("Porta Travada");
        }
    }

    //condição para destravar a porta
    if(digitalRead(pinOutputLockDoor) == LOW && digitalRead(pinInputButtonLock) == LOW){
        digitalWrite(pinOutputLockDoor, HIGH);
        Serial.println("Porta Destravada via Interruptor");
    }

    //condição para ligar o sensor biometrico
    if(digitalRead(pinOutputLockDoor) == LOW){
        if(digitalRead(pinOutputBioSensor) == LOW){
            digitalWrite(pinOutputBioSensor, HIGH);
        }
        getFingerprintID();
    }

    //condição para desligar o sensor biometrico
    if(digitalRead(pinOutputLockDoor) == HIGH && digitalRead(pinOutputBioSensor) == HIGH){
        digitalWrite(pinOutputBioSensor, LOW);
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
