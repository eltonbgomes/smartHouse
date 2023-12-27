/********************************************************************************\
 * Programa Expansão de Entradas e saídas do Arduino, utilizando Shift Register *
 * CI utilizado: 74HC165 com 74HC595                                            *
 * Por: Elton Barbosa Gomes                                                     *
 * Data: 05/09/2022                                                             *
 * Créditos: Baseado no playground.arduino.cc                                   *
\********************************************************************************/

/*Variaveis I2C ligar A4 e A5 entre arduinos
 *Variavel 0 -> temperatura externa em °C
 *Variavel 1 -> temperatura interna em °C
 *Variavel 2 -> luminosidade em percentual - (0 escuro) - (100 claro)
 *Variavel 3 -> status porta (false=aberta) (true=fechada)
 *Variavel 4 -> status trava porta (false=travado) (true=destravado)
 *Variavel 5
 *Variavel 6
 *Variavel 7
 *Variavel 8
 *Variavel 9
*/

#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <A2a.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// DEFINIÇÃO DO ENDEREÇO DO SLAVE
#define address 0x09

//numero de sensores de temperatura
#define nTempSensors 4

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

#define address 0x09         //address usado na comunicacao I2C entre arduino

int lux = 0;

// INSTANCIANDO OBJETOS
SoftwareSerial mySerial(8, 9); //pin 8 = Tx do sensor, pin 9 Rx do sensor (colocar divisor tensao)

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

OneWire oneWire(pinDataTemp);
DallasTemperature sensor(&oneWire);
 
float tempC[nTempSensors];
unsigned long time;

A2a arduinoMaster;

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

void readLux(){
    lux = analogRead(pinAnalogLuxSensor);
    // print out the value you read:
    int pLux = map(lux, 0, 1023, 0, 100);
    Serial.println("\nLuminosidade: ");
    Serial.println(lux);
    Serial.println("\nLuminosidade com MAP: ");
    Serial.println(pLux);
}

void receiveData() {
    arduinoMaster.receiveData(); 
}

void sendData() {
    arduinoMaster.sendData();
}

void setup() {
    Serial.begin(9600);
    // INICIA A COMUNICAÇÃO ENTRE ARDUINOS
    arduinoMaster.begin(address);

    // FUNÇÕES PARA COMUNICAÇÃO
    arduinoMaster.onReceive(receiveData);
    arduinoMaster.onRequest(sendData);

    sensor.begin();

    readTemp();

/*     //configuracao do sensor biometrico
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
    digitalWrite(pinOutputBioSensor, LOW); */
}

void loop() {

/*     //condição para travar a porta
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
    } */

    //fazer a leiura dos sensores
    if(millis() - time > 2000){
        readTemp();
    }
}
