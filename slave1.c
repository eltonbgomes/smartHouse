#include <A2a.h>

// DEFINIÇÃO DO ENDEREÇO DO SLAVE
#define address 8

#define delayTime 1000
 
unsigned long time;

A2a arduinoMaster;

void receiveData(){
    arduinoMaster.receiveData();
}

void sendData(){
    arduinoMaster.sendData();
}

void setup(){
    arduinoMaster.begin(address);
    arduinoMaster.onReceive(receiveData);
    arduinoMaster.onRequest(sendData);

    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(13, OUTPUT);
}

void loop(){
    for(int i = 0; i < 4; i++){
        if(arduinoMaster.varWireRead(i)){
            digitalWrite(i + 10, HIGH);
        }else{
            digitalWrite(i + 10, LOW);
        }
    }
}