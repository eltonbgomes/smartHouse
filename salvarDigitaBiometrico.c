#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <PushButton.h>

//botao para cadastrar digital ADM
#define pinButton 13

// DEFINIÇÃO DO PINO DA TRAVA
#define pinLock 5
#define pinLockSensor 6

// INSTANCIANDO OBJETOS
SoftwareSerial mySerial(8, 9); //pin 8 = Tx do sensor, pin 9 Rx do sensor

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
PushButton button(pinButton);

// DECLARAÇÃO DAS VARIÁVEIS E FUNCOES
uint8_t numID = 10;
bool save = false;

uint8_t read(int p, int ID){
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
                Serial.println("Erro de comunicação");
                break;
            case FINGERPRINT_IMAGEFAIL:
                Serial.println("Erro de leitura");
                break;
            default:
                Serial.println("Erro desconhecido");
                break;
        }
    }

    p = finger.image2Tz(ID);
    switch (p) {
        case FINGERPRINT_OK:
            Serial.println("Leitura convertida");
            return 0;
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
}

uint8_t saveModeID(uint8_t saveID) {

    int p = -1;
    Serial.print("Esperando uma leitura válida para salvar #");
    Serial.println(saveID);
    delay(2000);

    uint8_t helpRead = read(p, 1);
    if (helpRead != 0){
        return helpRead;
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
    
    helpRead = read(p, 2);
    if (helpRead != 0){
        return helpRead;
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
        numID--;
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
    digitalWrite(pinLock, HIGH);
    Serial.print("ID encontrado #"); Serial.print(finger.fingerID); 
    Serial.print(" com confiança de "); Serial.println(finger.confidence);
    Serial.println("Porta Destravada");
    delay(3000);
    return finger.fingerID;
}

void setup() {

    Serial.begin(9600);
    //configuracao do sensor biometrico
    pinMode(pinLock, OUTPUT);
    pinMode(pinLockSensor, INPUT);
    digitalWrite(pinLock, LOW);

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
}

void loop() {

    if(digitalRead(pinLockSensor) == HIGH && digitalRead(pinLock) == HIGH){
        digitalWrite(pinLock, LOW);
        Serial.println("Porta Travada");
    }

    button.button_loop();

    if ( button.pressed() ){
        save = true;
    }

    if(save){
        saveModeID(numID);
        numID++;
        save = false;
    }

    getFingerprintIDez();
}
