#include <A2a.h>
#define endereco 0x08 // MODIFICAR O ENDEREÇO PARA CADA SLAVE

A2a arduinoMaster;

void receberDados() {
  arduinoMaster.receiveData(); 
}

void enviarDados() {
  arduinoMaster.sendData(); 
}

void setup() {
  arduinoMaster.begin(endereco);
  arduinoMaster.onReceive(receberDados);
  arduinoMaster.onRequest(enviarDados);
}

void loop() {
}
