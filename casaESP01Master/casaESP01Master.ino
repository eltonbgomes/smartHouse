/*
     CÓDIGO:  Q0589-Master
     AUTOR:   BrincandoComIdeias
     LINK:    https://www.youtube.com/brincandocomideias ; https://cursodearduino.net/ ; https://cursoderobotica.net
     COMPRE:  https://www.arducore.com.br/
     SKETCH:  Integrando Variaveis entre 2 Arduinos - Esp01 Master
     DATA:    26/01/2020
*/

// INCLUSÃO DE BIBLIOTECAS
#include <A2a.h>
#include "config.h"

// DEFINIÇÕES
#define endereco 0x08

// INSTANCIANDO OBJETOS
AdafruitIO_Feed *ci595_0 = io.feed("ci595_0");

A2a arduinoSlave;

// DECLARAÇÃO DE FUNÇÕES
void configuraMQTT();

void retorno_ci595_0(AdafruitIO_Data *data); //funcao necessaria para cada feed do portal

void setup() {
  Serial.begin(9600);
  while (! Serial);

  configuraMQTT(); //conexao internet e adaFruit

  arduinoSlave.begin(0, 2);

  Serial.println("Atualizando valor do Display de LED");

  ci595_0->get();

  io.run();
  
  Serial.println("Fim Setup");
}

void loop() {
  io.run();

  if(arduinoSlave.varWireRead(endereco, 9)){
        arduinoSlave.varWireWrite(endereco, 9, false); //envia valor para arduino
    ci595_0->save(arduinoSlave.varWireRead(endereco, 0)); //envia informações para AdaFruit
  }

}

// IMPLEMENTO DE FUNÇÕES
void configuraMQTT() {
  Serial.print("Conectando ao Adafruit IO");
  io.connect();

  ci595_0->onMessage(retorno_ci595_0);

  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println(io.statusText());
}

void retorno_ci595_0(AdafruitIO_Data *data){
  Serial.print("Controle Recebido <- ");  
  Serial.println(data->value()); //recebe valor do portal ADAFruit
  
  arduinoSlave.varWireWrite(endereco, 0, byte(data->toInt())); //envia valor para arduino
  arduinoSlave.varWireWrite(endereco, 8, true);
}
