/*
     CÓDIGO:  Q0577
     AUTOR:   BrincandoComIdeias
     LINK:    https://www.youtube.com/brincandocomideias ; https://cursodearduino.net/ ; https://cursoderobotica.net
     COMPRE:  https://www.arducore.com.br/
     SKETCH:  20 Arduinos I2C
     DATA:    10/12/2019
*/

// INCLUSÃO DE BIBLIOTECAS
#include <A2a.h> // DOWNLOAD DA BIBLIOTECA http://bit.ly/2KF6kej

// DEFINIÇÕES DE PINOS
#define pinLED   7

// INSTANCIANDO OBJETOS
A2a arduinoSlave;

// DECLARAÇÃO DE VARIÁVEIS
void efeitoLEDBuiltIn();

// DECLARAÇÃO DE VARIÁVEIS
byte endereco[] = {0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27};
byte ordemBuiltIn[] = {0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x24, 0x25, 0x26, 0x27, 0x23, 0x22, 0x21, 0x20, 0x19, 0x18, 0x17, 0x16};

void setup() {
  arduinoSlave.begin();
  Serial.begin(9600);

  for (byte nS = 0 ; nS < 20 ; nS++) {
    arduinoSlave.pinWireMode(endereco[nS], pinLED, OUTPUT);
    arduinoSlave.pinWireMode(endereco[nS], LED_BUILTIN, OUTPUT);
    delay(10);
  }

  for (byte nS = 0 ; nS < 20 ; nS++) {
    arduinoSlave.digitalWireWrite(endereco[nS], pinLED, LOW);
    arduinoSlave.digitalWireWrite(endereco[nS], LED_BUILTIN, LOW);
    delay(1);
  }

  Serial.println("Fim Setup");
}

void loop() {
  // EFEITO 1
  for (byte nS = 0 ; nS < 20 ; nS++) {
    arduinoSlave.digitalWireWrite(endereco[nS], pinLED, HIGH);
    efeitoLEDBuiltIn();
    delay(30);
  }
  for (byte nS = 0 ; nS < 20 ; nS++) {
    arduinoSlave.digitalWireWrite(endereco[nS], pinLED, LOW);
    efeitoLEDBuiltIn();
    delay(30);
  }
  // FIM EFEITO 1

  // EFEITO 2
  for (byte nS = 0 ; nS < 20 ; nS++) {
    arduinoSlave.digitalWireWrite(endereco[nS], pinLED, HIGH);
    efeitoLEDBuiltIn();
    delay(10);
  }
  for (byte nS = 20 ; nS > 0 ; nS--) {
    arduinoSlave.digitalWireWrite(endereco[nS - 1], pinLED, LOW);
    efeitoLEDBuiltIn();
    delay(10);
  }
  // FIM EFEITO 2
}

// IMPLEMENTO DE FUNÇÕES
void efeitoLEDBuiltIn() {
  static byte enderecoSlave = 0;
  static bool sentido = true;
  static byte passos = 0;

  if (passos < 3) {
     passos++;
     return;
  } else {
     passos = 0;
  } 

  arduinoSlave.digitalWireWrite(ordemBuiltIn[enderecoSlave], LED_BUILTIN, LOW);

  if (sentido) {
    if (enderecoSlave < 19) {
        enderecoSlave++;
    } else {
        enderecoSlave--;
        sentido = false;
    }
  } else {
    if (enderecoSlave > 0) {
        enderecoSlave--;
    } else {
        enderecoSlave++;
        sentido = true;
    }
  }

  arduinoSlave.digitalWireWrite(ordemBuiltIn[enderecoSlave], LED_BUILTIN, HIGH);
}
