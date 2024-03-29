/********************************************************************************\
 * Programa Expansão de Entradas e saídas do Arduino, utilizando Shift Register *
 * CI utilizado: 74HC165 com 74HC595                                            *
 * Por: Elton Barbosa Gomes                                                     *
 * Data: 28/09/2020                                                             *
 * Créditos: Baseado no playground.arduino.cc                                   *
\********************************************************************************/

/*Variaveis I2C ligar A4 e A5 entre arduinos
 *Variavel 0 -> valor da saida para o IC 0
 *Variavel 1 -> valor da saida para o IC 1
 *Variavel 2 -> valor da saida para o IC 2
 *Variavel 3
 *Variavel 4
 *Variavel 5
 *Variavel 6
 *Variavel 7
 *Variavel 8
 *Variavel 9 -> variavel usada para atualizar valores do master quando houver mudança no I2C
*/

#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <A2a.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define boolArduino 9        //variavel usada para atualizar valores do master quando houver mudança no I2C
//74hc165
// Definições de Labels
#define nICs  3              //Registra o número de CIs cascateados
#define BYTES 8
#define readTime 50          //Registra o tempo de que deverá ter o pulso para leitura e gravação, (milesegundos)
#define DELAY  100           //Registra o atraso de segurança entre leituras, (milesegundos)
#define address 0x08         //address usado na comunicacao I2C entre arduino
#define buttonTime 750       //tempo de intervalo segura botao

//numero de sensores de temperatura
#define nTempSensors 2

//Porta sensor temperatura
#define pinDataTemp 9

// DEFINIÇÃO DO PINO DA TRAVA
#define pinOutputBioSensor 10 //saida para ligar o sensor biometrico
#define pinInputButtonLock 11 //entrada para travar porta
#define pinOutputLockDoor 12 //saida para travar ou destravar a porta
#define pinInputLockSensor 13 // input do sensor fechadura

//sensor de luminosidade
#define pinAnalogLuxSensor A0

#define delayTime 1000

int lux = 0;

// INSTANCIANDO OBJETOS
SoftwareSerial mySerial(A1, A2); //pin A1 = Tx do sensor, pin A2 Rx do sensor (colocar divisor tensao)

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

OneWire oneWire(pinDataTemp);
DallasTemperature sensor(&oneWire);
 
float tempC[nTempSensors];
unsigned long timeSensor;

// Declaração de constantes globais 165
const int ploadPin165        = 6;    //Conecta ao pino 1 do 74HC165 (LH/LD - asynchronous parallel load input)(PL)
const int clockEnablePin165  = 5;    //Conecta ao pino 15 do 74HC165 (CE - Clock Enable Input)(CE)
const int dataPin165         = 8;   //Conecta ao pino 9 do 74HC165 (Q7 - serial output from the last stage)(Q7)
const int clockPin165        = 7;   //Conecta ao pino 2 do 74HC165 (CP - Clock Input)(CP)

//inicialização das variaveis onde serão armazenados os status
byte pinValues[nICs];
byte oldPinValues[nICs];
byte pinValuesOut[nICs];
byte helpOut[nICs]; // variavel usada para armazenar valores das saídas para serem alteradas na borda de descida
bool helpOutBool[nICs];

bool alter = false; //variavel para alterar as saidas

//auxiliares para converter bases
bool helpBin[BYTES];
bool bin[BYTES];

//auxiliares para impedir o acesso de condicionais quando ocorrer o desligamento por tempo
bool helpEsp = false;
bool helpByte = false;
bool helpAll = false;

// Declaração de constantes globais 595
const int clockPin595 = 2; //Pino conectado a SRCLK (pino 11 no 74HC595), registrador de deslocamento
const int latchPin595 = 3; //Pino conectado a RCLK (pino 12 no 74HC595), registrador de armazenamento
const int dataPin595  = 4; //Pino conectado a SER (pino 14 no 74HC595), entrada de dados serial

//usado para contar o tempo do botao pressionado
unsigned long time;

//inicia objeto para comunicacao arduino
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
    timeSensor = millis();
}

void readStatusOut(){
    Serial.println("\nStatus");
    for(int i = 0; i < nICs; i++){
        Serial.println(arduinoMaster.varWireRead(i));
    }
}

void readLux(){
    lux = analogRead(pinAnalogLuxSensor);
    // print out the value you read:
    Serial.println("\nLuminosidade: ");
    Serial.println(lux);
}

//Função para definir um rotina shift-in, lê os dados do 74HC165
void read_shift_regs(){
    bool bitVal = 0; //variavel para que armazena bit a bit
    byte bytesVal[nICs];
    byte bytesValOut[nICs];
    for(int i = 0; i < nICs; i++){
        bytesVal[i] = 0;
        bytesValOut[i] = arduinoMaster.varWireRead(i);
    }

    //desloca todos os bits para o pino de dados para leitura
    digitalWrite(clockEnablePin165, HIGH);
    digitalWrite(ploadPin165, LOW);
    delayMicroseconds(readTime);
    digitalWrite(ploadPin165, HIGH);
    digitalWrite(clockEnablePin165, LOW);

    // Efetua a leitura de um bit da saida serial do 74HC165

    for(int IC = nICs - 1; IC >= 0; IC--){
        for (int j = 0; j < BYTES; j++){
            bitVal = digitalRead(dataPin165);

            //Realiza um shift left e armazena o bit correspondente em bytesVal
            bytesVal[IC] |= (bitVal << ((BYTES-1) - j));

            //compara se ouve mudanca para alterar o 595
            bytesValOut[IC] ^= (bitVal << ((BYTES-1) - j));

            //Lança um pulso de clock e desloca o próximo bit
            digitalWrite(clockPin165, HIGH);
            delayMicroseconds(readTime);
            digitalWrite(clockPin165, LOW);
        }
        pinValues[IC] = bytesVal[IC];

        //condição para somente alterar as saídas se soltar o botão
        if(pinValues[IC] != oldPinValues[IC] && !helpOutBool[IC]){
            time = millis();
            helpOut[IC] = bytesValOut[IC];
            helpOutBool[IC] = true;
            helpEsp = helpByte = helpAll = true;
            oldPinValues[IC] = pinValues[IC];
        }

        //condicao para impedir a alteração com o botao pressionado
        if(pinValues[IC] == 0 && helpOutBool[IC]){
            if(helpOut[IC] != arduinoMaster.varWireRead(IC)){
                arduinoMaster.varWireWrite(IC, helpOut[IC]);
                alter = true;
            }
            helpOutBool[IC] = false;
        }

        //condiçao para funcionar apenas com o botao 07 IC 0
        if((millis() - time) > buttonTime && helpOutBool[IC] && helpEsp && pinValues[IC] == 128 && IC == 0){
            convDecBin(IC); // converte o valor decimal para binario
            
            //desliga as saidas desejadas
            helpBin[7] = 0;
            helpBin[6] = 0;
            
            //inversao
            for(int i = 0; i < BYTES; i++){    
                bin[i] = helpBin[BYTES - i - 1];
            }
            arduinoMaster.varWireWrite(IC, convBinDec());
            helpOut[IC] = arduinoMaster.varWireRead(IC);
            helpEsp = false;
            alter = true;
        }

        //desliga as saidas após tempo pressionado
        if((millis() - time) > buttonTime * 2 && helpOutBool[IC] && helpByte){
            arduinoMaster.varWireWrite(IC, 0);
            helpOut[IC] = arduinoMaster.varWireRead(IC);
            helpByte = false;
            alter = true;
        }

        //desliga todas as saidas após tempo pressionado
        if((millis() - time) > buttonTime * 3 && helpOutBool[IC] && helpAll){
            for(int i = 0; i < nICs; i++){
                arduinoMaster.varWireWrite(i, 0);
            }
            helpOut[IC] = arduinoMaster.varWireRead(IC);
            helpAll = false;
            alter = true;
        }
    }
}

void convDecBin (int IC){
    for(int i = 0; i < BYTES; i++){
        helpBin[i] = 0; 
    }
    int decimal = arduinoMaster.varWireRead(IC);;
    int i;
    for(i = 0; (decimal > 1); i++){
        helpBin[i] = decimal % 2;
        decimal /= 2; 
    }
    helpBin[i] = decimal;
}

int convBinDec(){
    float dec = 0;
    for(int i = 0; i < BYTES; i++){
        dec += bin[i]*pow(2, 7 - i);
    }
    return (int)(dec + 1);
}

void alterOut(){
    //alterar saida do 595
    for (int i = 0; i < nICs; ++i){
        digitalWrite(latchPin595, LOW);
        shiftOut(dataPin595, clockPin595, LSBFIRST, arduinoMaster.varWireRead(i));
        digitalWrite(latchPin595, HIGH);
    }
}

void receiveData() {
  arduinoMaster.receiveData(); 
}

void sendData() {
  arduinoMaster.sendData(); 
}

// Configuração do Programa
void setup(){
    Serial.begin(9600);

    // INICIA A COMUNICAÇÃO ENTRE ARDUINOS COMO SLAVE NO ENDEREÇO DEFINIDO
    arduinoMaster.begin(address);

    //sensor temperatura
    sensor.begin();

    //leitura sensores
    readTemp();

    // FUNÇÕES PARA COMUNICAÇÃO
    arduinoMaster.onReceive(receiveData);
    arduinoMaster.onRequest(sendData);

    arduinoMaster.varWireWrite(boolArduino, false);

    //configuracao do sensor biometrico
    pinMode(pinOutputLockDoor, OUTPUT);
    digitalWrite(pinOutputLockDoor, LOW);
    pinMode(pinInputLockSensor, INPUT);
    pinMode(pinInputButtonLock, INPUT);
    pinMode(pinOutputBioSensor, OUTPUT);
    digitalWrite(pinOutputBioSensor, HIGH);

    //pino sensor de luminosidade
    pinMode(pinAnalogLuxSensor,INPUT);
    
    //Inicializa e configura os pinos do 165
    pinMode(ploadPin165, OUTPUT);
    pinMode(clockEnablePin165, OUTPUT);
    pinMode(clockPin165, OUTPUT);
    pinMode(dataPin165, INPUT);

    digitalWrite(clockPin165, HIGH);
    digitalWrite(ploadPin165, HIGH);

    //incializa pinos do 595
    pinMode(latchPin595, OUTPUT);
    pinMode(clockPin595, OUTPUT);
    pinMode(dataPin595, OUTPUT);

    //reseta o estado dos pinos
    for(int i = 0; i < nICs; i++){
        pinValues[i] = 0;
        oldPinValues[i] = 0;
        arduinoMaster.varWireWrite(i, 0);
        helpOut[i] = 0;
        helpOutBool[i] = false;
    }

    alterOut();

    //inicialização sensor biometrico
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
    digitalWrite(pinOutputBioSensor, LOW);
}

//Função do loop principal
void loop(){
    //Lê todos as portas externas

    read_shift_regs();
    
    //altera a saída se existir alguma mudança
    if(alter){
        arduinoMaster.varWireWrite(boolArduino, true);
        alterOut();
        alter = false;
    }

    //condição para travar a porta
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
    }

    //condição para fazer a leitura de dados do slave
    if(arduinoMaster.varWireRead(boolArduino)){
        readStatusOut();
        arduinoMaster.varWireWrite(boolArduino, false);
    }

    //fazer a leiura dos sensores
    if(millis() - timeSensor > 45000){
        readTemp();
    }

    delay(DELAY);
}
