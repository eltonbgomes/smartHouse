/********************************************************************************\
 * IOT Entradas e saídas do Arduino, utilizando Shift Register *
 * CI utilizado: 74HC165 com 74HC595                                            *
 * Por: Elton Barbosa Gomes                                                     *
 * Data: 31/12/2023                                                             *
 * Créditos: Baseado no playground.arduino.cc                                   *
\********************************************************************************/

// INCLUSÃO DE BIBLIOTECAS
#include <A2a.h>
#include "config.h"

// DEFINIÇÕES
#define address 0x08
#define boolMaster 8
#define boolFinishedMaster 9

// INSTANCIANDO OBJETOS
AdafruitIO_Feed *ci595_0 = io.feed("ci595_0");
AdafruitIO_Feed *ci595_1 = io.feed("ci595_1");
AdafruitIO_Feed *ci595_2 = io.feed("ci595_2");
AdafruitIO_Feed *tempExt = io.feed("tempExt");
AdafruitIO_Feed *tempInt = io.feed("tempInt");
AdafruitIO_Feed *lux = io.feed("lux");

A2a arduinoSlave;

// DECLARAÇÃO DE FUNÇÕES
bool getStatusSlaveIO();

void configuraMQTT();

void temperatureLux();

void retorno_ci595_0(AdafruitIO_Data *data); //funcao necessaria para cada feed do portal
void retorno_ci595_1(AdafruitIO_Data *data); //funcao necessaria para cada feed do portal
void retorno_ci595_2(AdafruitIO_Data *data); //funcao necessaria para cada feed do portal

long temperatureTime = 	millis();

void setup() {
	Serial.begin(9600);
	while (! Serial);

	configuraMQTT(); //conexao internet e adaFruit

	arduinoSlave.begin(0, 2);

	Serial.println("Atualizando valores");

	ci595_0->get();
	ci595_1->get();
	ci595_2->get();
	tempExt->get();
	tempInt->get();
	lux->get();

	io.run();
	
	temperatureLux();

	Serial.println("Fim Setup");
}

void loop() {
	io.run();

	if(getStatusSlaveIO()){
		arduinoSlave.varWireWrite(address, boolFinishedMaster, true);
		arduinoSlave.varWireWrite(address, boolMaster, false);

		if(arduinoSlave.varWireRead(address, 0) != arduinoSlave.varWireRead(address, 3)){
			ci595_0->save(arduinoSlave.varWireRead(address, 0)); //envia informações para AdaFruit
		}

		if(arduinoSlave.varWireRead(address, 1) != arduinoSlave.varWireRead(address, 4)){
			ci595_1->save(arduinoSlave.varWireRead(address, 1)); //envia informações para AdaFruit
		}

		if(arduinoSlave.varWireRead(address, 2) != arduinoSlave.varWireRead(address, 5)){
			ci595_2->save(arduinoSlave.varWireRead(address, 2)); //envia informações para AdaFruit
		}
		arduinoSlave.varWireWrite(address, boolFinishedMaster, false);
	}

	if(millis() - temperatureTime > 30000){
		temperatureLux();
	}
}

void temperatureLux(){
	byte byteHigh, byteLow;
	int count = 0;
	float data[3];
	int aux;
	
	for(int i = 10; i < 15; i += 2){
		byteHigh = arduinoSlave.varWireRead(address, i);
		byteLow = arduinoSlave.varWireRead(address, i + 1);
		int aux = byteHigh << 8 | byteLow;
		data[count] = aux;
		data[count] /= 100;
		count++;
	}
	tempExt->save(data[0]); //envia informações para AdaFruit
	tempInt->save(data[1]); //envia informações para AdaFruit
	lux->save(data[2]); //envia informações para AdaFruit
	temperatureTime = millis();
}

bool getStatusSlaveIO(){
	if(arduinoSlave.varWireRead(address, boolMaster)){
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
		arduinoSlave.varWireWrite(address, 3, byte(data->toInt())); //envia valor para arduino
	}
}

void retorno_ci595_1(AdafruitIO_Data *data){
	Serial.print("Controle Recebido <- ");  
	Serial.println(data->value()); //recebe valor do portal ADAFruit

	if(!getStatusSlaveIO()){
		arduinoSlave.varWireWrite(address, 4, byte(data->toInt())); //envia valor para arduino
	}
}

void retorno_ci595_2(AdafruitIO_Data *data){
	Serial.print("Controle Recebido <- ");  
	Serial.println(data->value()); //recebe valor do portal ADAFruit

	if(!getStatusSlaveIO()){
		arduinoSlave.varWireWrite(address, 5, byte(data->toInt())); //envia valor para arduino
	}
}
