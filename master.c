/********************************************************************************\
 * Programa Expansão de Entradas e saídas do Arduino, utilizando Shift Register *
 * CI utilizado: 74HC165 com 74HC595                                            *
 * Por: Elton Barbosa Gomes                                                     *
 * Data: 05/09/2022                                                             *
 * Créditos: Baseado no playground.arduino.cc                                   *
\********************************************************************************/
//ligar A5 e A4 dos arduinos (utilização da comunicação I2C)

#include <A2a.h>


// DEFINIÇÃO DO ENDEREÇO DO SLAVE
#define address 8
#define address 9

#define delayTime 1500

A2a arduinoSlave;

void setup() {
    Serial.begin(9600);
    // INICIA A COMUNICAÇÃO ENTRE ARDUINOS
    arduinoSlave.begin();
}

void loop() {

    for(int i = 8; i < 10; i++){
        for(int j = 0; j < 4, j++){
            arduinoSlave.varWireWrite(i, j, HIGH);
            delay(delayTime);
        }
    }

    for(int i = 8; i < 10; i++){
        for(int j = 0; j < 4, j++){
            arduinoSlave.varWireWrite(i, j, LOW);
            delay(delayTime);
        }
    }
}
