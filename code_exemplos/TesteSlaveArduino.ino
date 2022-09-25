#include <A2a.h>
#define endereco 0x08

#define pinSensor A1
#define pinLED1 3
#define pinLED2 4
#define pinLED3 5
#define pinLED4 6
#define pinLED5 7
#define pinLED6 8
#define pinLED7 9
#define pinLED8 10
#define pinLED9 11
#define pinLED10 12

A2a arduinoMaster;

byte displayLEDAnt;

void setup() {
  arduinoMaster.begin(endereco);
  arduinoMaster.onReceive(receberDados);
  arduinoMaster.onRequest(enviarDados);

  pinMode(pinLED1, OUTPUT);
  pinMode(pinLED2, OUTPUT);
  pinMode(pinLED3, OUTPUT);
  pinMode(pinLED4, OUTPUT);
  pinMode(pinLED5, OUTPUT);
  pinMode(pinLED6, OUTPUT);
  pinMode(pinLED7, OUTPUT);
  pinMode(pinLED8, OUTPUT);
  pinMode(pinLED9, OUTPUT);
  pinMode(pinLED10, OUTPUT);

  Serial.begin(9600);
}

void loop() {

   int valorSensor = analogRead(pinSensor);
   arduinoMaster.varWireWrite(0, highByte(valorSensor));
   arduinoMaster.varWireWrite(1, lowByte(valorSensor));
   Serial.println(valorSensor);

   byte displayLED = arduinoMaster.varWireRead(2);
   if (displayLED != displayLEDAnt) {
      digitalWrite(pinLED1, displayLED >= 1);
      digitalWrite(pinLED2, displayLED >= 2);
      digitalWrite(pinLED3, displayLED >= 3);
      digitalWrite(pinLED4, displayLED >= 4);
      digitalWrite(pinLED5, displayLED >= 5);
      digitalWrite(pinLED6, displayLED >= 6);
      digitalWrite(pinLED7, displayLED >= 7);
      digitalWrite(pinLED8, displayLED >= 8);
      digitalWrite(pinLED9, displayLED >= 9);
      digitalWrite(pinLED10, displayLED >= 10);
      displayLEDAnt = displayLED; 
   }

   delay(20);
}

void receberDados() {
  arduinoMaster.receiveData(); 
}

void enviarDados() {
  arduinoMaster.sendData(); 
}
