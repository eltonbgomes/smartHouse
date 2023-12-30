/*
     CÓDIGO:  Q0589-Master
     AUTOR:   Elton Barbosa Gomes
     LINK:    https://www.youtube.com/brincandocomideias ; https://cursodearduino.net/ ; https://cursoderobotica.net
     COMPRE:  https://www.arducore.com.br/
	 SKETCH:  Integrando Variaveis entre 2 Arduinos - Esp01 Master
     DATA:    26/01/2020
*/

// INCLUSÃO DE BIBLIOTECAS
#include <A2a.h>
#include "config.h"

// DEFINIÇÕES
#define enderecoIO 0x08
#define enderecoSensors 0x09

// INSTANCIANDO OBJETOS
AdafruitIO_Feed *ci595_0 = io.feed("ci595_0");
AdafruitIO_Feed *ci595_1 = io.feed("ci595_1");
AdafruitIO_Feed *ci595_2 = io.feed("ci595_2");

A2a arduinoSlave;

// DECLARAÇÃO DE FUNÇÕES
bool getStatusSlaveIO();

void configuraMQTT();

void retorno_ci595_0(AdafruitIO_Data *data); //funcao necessaria para cada feed do portal
void retorno_ci595_1(AdafruitIO_Data *data); //funcao necessaria para cada feed do portal
void retorno_ci595_2(AdafruitIO_Data *data); //funcao necessaria para cada feed do portal

void setup() {
	Serial.begin(9600);
	while (! Serial);

	configuraMQTT(); //conexao internet e adaFruit

	arduinoSlave.begin(0, 2);

	Serial.println("Atualizando valor do Display de LED");

	ci595_0->get();
	ci595_1->get();
	ci595_2->get();

	io.run();

	Serial.println("Fim Setup");
}

void loop() {
	io.run();

	if(getStatusSlaveIO()){
		arduinoSlave.varWireWrite(enderecoIO, 9, false);

		if(arduinoSlave.varWireRead(enderecoIO, 0) != arduinoSlave.varWireRead(enderecoIO, 3)){
			ci595_0->save(arduinoSlave.varWireRead(enderecoIO, 0)); //envia informações para AdaFruit
		}

		if(arduinoSlave.varWireRead(enderecoIO, 1) != arduinoSlave.varWireRead(enderecoIO, 4)){
			ci595_1->save(arduinoSlave.varWireRead(enderecoIO, 1)); //envia informações para AdaFruit
		}

		if(arduinoSlave.varWireRead(enderecoIO, 2) != arduinoSlave.varWireRead(enderecoIO, 5)){
			ci595_2->save(arduinoSlave.varWireRead(enderecoIO, 2)); //envia informações para AdaFruit
		}
	}
}

bool getStatusSlaveIO(){
	if(arduinoSlave.varWireRead(enderecoIO, 9)){
		return true;
	}else{
		return false;
	}
}

// IMPLEMENTO DE FUNÇÕES
void configuraMQTT() {
	Serial.print("Conectando ao Adafruit IO");
	io.connect();

	ci595_0->onMessage(retorno_ci595_0);
	ci595_1->onMessage(retorno_ci595_1);
	ci595_2->onMessage(retorno_ci595_2);

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

	if(!getStatusSlaveIO()){
		arduinoSlave.varWireWrite(enderecoIO, 3, byte(data->toInt())); //envia valor para arduino
	}
}

void retorno_ci595_1(AdafruitIO_Data *data){
	Serial.print("Controle Recebido <- ");  
	Serial.println(data->value()); //recebe valor do portal ADAFruit

	if(!getStatusSlaveIO()){
		arduinoSlave.varWireWrite(enderecoIO, 4, byte(data->toInt())); //envia valor para arduino
	}
}

void retorno_ci595_2(AdafruitIO_Data *data){
	Serial.print("Controle Recebido <- ");  
	Serial.println(data->value()); //recebe valor do portal ADAFruit

	if(!getStatusSlaveIO()){
		arduinoSlave.varWireWrite(enderecoIO, 5, byte(data->toInt())); //envia valor para arduino
	}
}
