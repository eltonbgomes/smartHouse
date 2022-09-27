#include "Keyboard.h"

#define pinOn 8  //habilitar teclado
#define pinFood 9  //ham/mushrooms
#define pinCast 10 //SD/GFB
#define pinCastFast 11 //castFast
#define pinGamel 12 //castFast

#define delayHam 362000
#define delayMush 266000
#define delaySD 70000
#define delaySDNecklace 3615000
#define delayGFB 38000
#define delayRing 453000
#define delayNecklace 3600000
#define delayBoots 14403000
#define delayPotion 1198
#define delayFastSD 25112

#define rangeWait 2500
#define rangeFast 100

//auxiliares para contar o tempo de cada funcao
long timeFood = millis();
long timeCast = millis();
long timePotion = millis();
long timeRing = millis();
long timeBoots = millis();
long timeNecklace = millis();
long timeSDNecklace = millis();

int count = 0;

void setup() {
    delay(10000);
    Keyboard.begin();
    pinMode(pinFood, INPUT); //ham/mushrooms
    pinMode(pinCast, INPUT); //SD/GFB
    pinMode(pinCastFast, INPUT); //SD/GFB
    pinMode(pinGamel, INPUT); //SD/GFB
    pinMode(A0, OUTPUT);

    randomSeed(analogRead(A0)); //semente para gerar numeros aleatorios
}

void loop() {

    if (!digitalRead(pinOn) && digitalRead(pinGamel)) {
            delay(120);
            Keyboard.write(49);
            delay(112);
            Keyboard.write(50);
            delay(97);
            Keyboard.write(51);
            delay(86);
            Keyboard.write(52);
        }

    if(!digitalRead(pinOn) && !digitalRead(pinGamel)){
        if (!digitalRead(pinFood)) { //ham
            pressButton(delayHam, 49, timeFood, rangeWait);
        }
        else {
            pressButton(delayMush, 50, timeFood, rangeWait);
        }

        if (!digitalRead(pinCast)) {
            pressButton(delaySD, 51, timeCast, rangeWait);
        }
        else {
            pressButton(delayGFB, 52, timeCast, rangeWait);
        }

        if (digitalRead(pinCastFast)) {
            pressButton(delayPotion, 57, timePotion, rangeFast);
            if(count >= 10){
                delay(1012);
                Keyboard.write(51);
                count = 0;
            }
        }

        pressButton(delayRing, 53, timeRing, rangeWait);
        pressButton(delayNecklace, 54, timeNecklace, rangeWait);
        pressButton(delaySDNecklace, 55, timeSDNecklace, rangeWait);
        pressButton(delayBoots, 56, timeBoots, rangeWait);
    }
    delay(300);
}

void pressButton (long delayKind, int press, long time, long range){
    long r = random(delayKind - range, delayKind + range);
    long t = millis() - time;
    if(t > r){   //tempo para entrar no if em ms que varia conforme o range do random
        Keyboard.write(press);
        resetTime(press);
    }
}

void resetTime(int press){
    switch (press) {
        case 49:
            timeFood = millis();
            break;
        case 50:
            timeFood = millis();
            break;
        case 51:
            timeCast = millis();
            break;
        case 52:
            timeCast = millis();
            break;
        case 53:
            timeRing = millis();
            break;
        case 54:
            timeNecklace = millis();
            break;
        case 55:
            timeSDNecklace = millis();
            break;
        case 56:
            timeBoots = millis();
            break;
        case 57:
            timePotion = millis();
            count++;
            break;
        default:
        break;
    }
}
