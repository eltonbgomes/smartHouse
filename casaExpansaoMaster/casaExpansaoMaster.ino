#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <PushButton.h>
#include <A2a.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// DEFINIÇÃO DO ENDEREÇO DO SLAVE
#define address 0x08

//botao para cadastrar digital ADM
#define pinButton 10

// DEFINIÇÃO DO PINO DA TRAVA
#define pinLock 13

// INSTANCIANDO OBJETOS
SoftwareSerial mySerial(11, 12); //pin 11 = Tx do sensor, pin 12 Rx do sensor

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
PushButton button(pinButton);

// DECLARAÇÃO DAS VARIÁVEIS E FUNCOES
uint8_t numID = 1;
bool save = false;

OneWire oneWire(9);
DallasTemperature sensor(&oneWire);
 
int nTempSensors = 0;
float tempC;

A2a arduinoSlave;

uint8_t saveModeID(uint8_t saveID) {

    int p = -1;
    Serial.print("Esperando uma leitura válida para salvar #");
    Serial.println(saveID);
    delay(2000);
    while (p != FINGERPRINT_OK) {
        p = finger.getImage();
        switch (p) {
            case FINGERPRINT_OK:
                Serial.println("Leitura concluída");
                break;
            case FINGERPRINT_NOFINGER:
                Serial.println(".");
                delay(200);
                break;
            case FINGERPRINT_PACKETRECIEVEERR:
                Serial.println("Erro comunicação");
                break;
            case FINGERPRINT_IMAGEFAIL:
                Serial.println("Erro de leitura");
                break;
            default:
                Serial.println("Erro desconhecido");
                break;
        }
    }

    p = finger.image2Tz(1);
    switch (p) {
        case FINGERPRINT_OK:
            Serial.println("Leitura convertida");
            break;
        case FINGERPRINT_IMAGEMESS:
            Serial.println("Leitura suja");
            return p;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Erro de comunicação");
            return p;
        case FINGERPRINT_FEATUREFAIL:
            Serial.println("Não foi possível encontrar propriedade da digital");
            return p;
        case FINGERPRINT_INVALIDIMAGE:
            Serial.println("Não foi possível encontrar propriedade da digital");
            return p;
        default:
            Serial.println("Erro desconhecido");
            return p;
    }
  
    Serial.println("Remova o dedo");
    delay(2000);
    p = 0;
    while (p != FINGERPRINT_NOFINGER) {
        p = finger.getImage();
    }
    Serial.print("ID ");
    Serial.println(saveID);
    p = -1;
    Serial.println("Coloque o Mesmo dedo novamente");
    while (p != FINGERPRINT_OK) {
        p = finger.getImage();
        switch (p) {
            case FINGERPRINT_OK:
                Serial.println("Leitura concluída");
                break;
            case FINGERPRINT_NOFINGER:
                Serial.print(".");
                delay(200);
                break;
            case FINGERPRINT_PACKETRECIEVEERR:
                Serial.println("Erro de comunicação");
                break;
            case FINGERPRINT_IMAGEFAIL:
                Serial.println("Erro de Leitura");
                break;
            default:
                Serial.println("Erro desconhecido");
                break;
        }
    }

    // OK successo!

    p = finger.image2Tz(2);
    switch (p) {
        case FINGERPRINT_OK:
            Serial.println("Leitura convertida");
            break;
        case FINGERPRINT_IMAGEMESS:
            Serial.println("Leitura suja");
            return p;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Erro de comunicação");
            return p;
        case FINGERPRINT_FEATUREFAIL:
            Serial.println("Não foi possível encontrar as propriedades da digital");
            return p;
        case FINGERPRINT_INVALIDIMAGE:
            Serial.println("Não foi possível encontrar as propriedades da digital");
            return p;
        default:
            Serial.println("Erro desconhecido");
            return p;
    }
  
    // OK convertido!
    Serial.print("Criando modelo para #");
    Serial.println(saveID);
  
    p = finger.createModel();
    if (p == FINGERPRINT_OK) {
        Serial.println("As digitais batem!");
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
        Serial.println("Erro de comunicação");
        return p;
    } else if (p == FINGERPRINT_ENROLLMISMATCH) {
        Serial.println("As digitais não batem");
        return p;
    } else {
        Serial.println("Erro desconhecido");
        return p;
    }
  
    Serial.print("ID "); Serial.println(saveID);
    p = finger.storeModel(saveID);
    if (p == FINGERPRINT_OK) {
        Serial.println("Armazenado!");
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
        Serial.println("Erro de comunicação");
        return p;
    } else if (p == FINGERPRINT_BADLOCATION) {
        Serial.println("Não foi possível save neste local da memória");
        return p;
    } else if (p == FINGERPRINT_FLASHERR) {
        Serial.println("Erro durante escrita na memória flash");
        return p;
    } else {
        Serial.println("Erro desconhecido");
        return p;
    }
}

int getFingerprintIDez() {
    uint8_t p = finger.getImage();
    if (p != FINGERPRINT_OK)  return -1;

    p = finger.image2Tz();
    if (p != FINGERPRINT_OK)  return -1;

    p = finger.fingerFastSearch();
    if (p != FINGERPRINT_OK)  return -1;

    //Encontrou uma digital!
    if (finger.fingerID == 0) {
        Serial.print("Modo Administrador!");

        numID++;
        saveModeID(numID);
        return 0; 
    }else{

        digitalWrite(pinLock, LOW);
        Serial.print("ID encontrado #"); Serial.print(finger.fingerID); 
        Serial.print(" com confiança de "); Serial.println(finger.confidence);
        delay(500);
        digitalWrite(pinLock, HIGH);
        return finger.fingerID;
    }
}

void setup() {

    //configuracao do sensor biometrico
    pinMode(pinLock, OUTPUT);
    digitalWrite(pinLock, HIGH);

    finger.begin(57600);

    bool checkFinger = false;
    while(!checkFinger){
        if(finger.verifyPassword()){
            Serial.println("Sensor biometrico encontrado!");
            checkFinger = true;
        }else{
            Serial.println("Sensor biometrico não encontrado! Verifique a conexão e reinicie o sistema");
            delay(1500);
        }
    }

    // INICIA A COMUNICAÇÃO ENTRE ARDUINOS
    arduinoSlave.begin();

    sensor.begin(); 
    Serial.begin(9600);
    Serial.println("Localizando Dispositivos ...");
    Serial.print("Encontrados ");
    nTempSensors = sensor.getDeviceCount();
    Serial.print(nTempSensors, DEC);
    Serial.println(" dispositivos.");
    Serial.println("\n");
}

void loop() {
    Serial.print("\nStatus\n");  
    for(int i = 0; i < 3; i++){
            Serial.print(arduinoSlave.varWireRead(address, i));
            Serial.print("\n");
    }
    sensor.requestTemperatures(); 
 
    for (int i = 0;  i < nTempSensors;  i++) {
        Serial.print("Sensor ");
        Serial.print(i+1);
        Serial.print(": ");
        tempC = sensor.getTempCByIndex(i);
        Serial.print(tempC);
        Serial.println("ºC");
    }

    button.button_loop();

    if ( button.pressed() ){
        save = true;
    }

    if(save){
        saveModeID(0);
        save = false;
    }

    getFingerprintIDez();
}
