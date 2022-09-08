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

#define delayTime 1500

A2a arduinoSlave;

void setup() {
    Serial.begin(9600);
    // INICIA A COMUNICAÇÃO ENTRE ARDUINOS
    arduinoSlave.begin();

    Serial.println("INICIALIZANDO");
    for(int i = 8; i < 10; i++){
        for(int j = 10; j < 14; j++){
            arduinoSlave.pinWireMode(i, j, OUTPUT);
        }
    }
}

void loop() {

    Serial.println("LIGANDO");
    for(int i = 8; i < 10; i++){
        for(int j = 10; j < 14; j++){
            arduinoSlave.digitalWireWrite(i, j, HIGH);
            delay(delayTime);
        }
    }

    Serial.println("DESLIGANDO");
    for(int i = 8; i < 10; i++){
        for(int j = 10; j < 14; j++){
            arduinoSlave.digitalWireWrite(i, j, LOW);
            delay(delayTime);
        }
    }
}
