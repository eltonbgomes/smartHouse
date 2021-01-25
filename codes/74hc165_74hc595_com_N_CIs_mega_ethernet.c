/********************************************************************************\
 * Expansão de Entradas e saídas do Arduino, utilizando Shift Register          *
 * CI utilizado: 74HC165 e 74HC595                                              *
 * Autor: Elton Barbosa Gomes                                                     *
 * Data: 15/01/2020                                                             *
 * Créditos: Baseado no playground.arduino.cc                                   *
\********************************************************************************/

#include <SPI.h> //INCLUSÃO DE BIBLIOTECA
#include <Ethernet.h> //INCLUSÃO DE BIBLIOTECA

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //ATRIBUIÇÃO DE ENDEREÇO MAC AO ETHERNET SHIELD W5100
byte ip[] = { 192, 168, 0, 110 }; //COLOQUE UMA FAIXA DE IP DISPONÍVEL DO SEU ROTEADOR. EX: 192.168.1.110  **** ISSO VARIA, NO MEU CASO É: 192.168.0.175
byte gateway[] = {192, 168, 0, 1}; //GATEWAY DE CONEXÃO (ALTERE PARA O GATEWAY DO SEU ROTEADOR)
byte subnet[] = {255, 255, 255, 0}; //MASCARA DE REDE (ALTERE PARA A SUA MÁSCARA DE REDE)
EthernetServer server(80); //PORTA EM QUE A CONEXÃO SERÁ FEITA

const int ledPin = 50; //PINO DIGITAL UTILIZADO PELO LED
String readString = String(30); //VARIÁVEL PARA BUSCAR DADOS NO ENDEREÇO (URL)
String indice_out = String(30); //VARIÁVEL PARA BUSCAR DADOS NO ENDEREÇO (URL)
int indice_out_int;
int value_out; //VARIÁVEL PARA BUSCAR DADOS NO ENDEREÇO (URL)

String indice_c_out = String(30); //VARIÁVEL PARA BUSCAR DADOS NO ENDEREÇO (URL)
int indice_c_out_int;
int value_c_out; //VARIÁVEL PARA BUSCAR DADOS NO ENDEREÇO (URL)

byte byte_out;


int status = 0; //DECLARAÇÃO DE VARIÁVEL DO TIPO INTEIRA(SERÁ RESPONSÁVEL POR VERIFICAR O STATUS ATUAL DO LED)
int led = -1;

int controle = 0;







//74hc165
// Definições de constantes
#define nCIs  2               //Registra o número de CIs cascateados
#define BYTES 8
#define TempoDeslocamento 50  //Registra o tempo de que deverá ter o pulso para leitura e gravação, (milesegundos)
#define Atraso  100           //Registra o atraso de segurança entre leituras, (milesegundos)

// Declaração de constantes globais 74HC165
const int ploadPin165        = 30;   //Conecta ao pino 1 do 74HC165 (LH/LD - asynchronous parallel load input)(PL)
const int clockEnablePin165  = 28;   //Conecta ao pino 15 do 74HC165 (CE - Clock Enable Input)(CE)
const int dataPin165         = 34;   //Conecta ao pino 9 do 74HC165 (Q7 - serial output from the last stage)(Q7)
const int clockPin165        = 32;   //Conecta ao pino 2 do 74HC165 (CP - Clock Input)(CP)

// Declaração de constantes globais 74HC595
const int clockPin595 = 22; //Pino conectado a SRCLK (pino 11 no 74HC595), registrador de deslocamento
const int latchPin595 = 24; //Pino conectado a RCLK (pino 12 no 74HC595), registrador de armazenamento
const int dataPin595 = 26; //Pino conectado a SER (pino 14 no 74HC595), entrada de dados serial

//inicialização dos vetores onde serão armazenados os bytes
byte pinValues[nCIs];
byte oldPinValues[nCIs];
byte pinValuesOut[nCIs];
byte oldPinValuesOut[nCIs];

//Função para leitura dos dados do 74HC165
void read_shift_regs(){
    bool bitVal=0; //variavel para que armazena bit a bit
    byte bytesVal[nCIs];
    byte bytesValOut[nCIs];
    for(int i = 0; i < nCIs; i++){
        bytesVal[i] = 0;
        bytesValOut[i] = pinValuesOut[i];
    }

    //desloca todos os bits para o pino de dados para leitura
    digitalWrite(clockEnablePin165, HIGH);
    digitalWrite(ploadPin165, LOW);
    delayMicroseconds(TempoDeslocamento);
    digitalWrite(ploadPin165, HIGH);
    digitalWrite(clockEnablePin165, LOW);

    // Efetua a leitura dos bits da saida serial do 74HC165
    for(int i = 0; i < nCIs; i++){
        for (int j = 0; j < BYTES; j++){
            bitVal = digitalRead(dataPin165);

            //Realiza um shift left e armazena o bit correspondente em bytesVal
            bytesVal[nCIs-1-i] |= (bitVal << ((BYTES-1) - j));

            //compara se ouve mudanca de borda de subida para alterar o 595
            bytesValOut[nCIs-1-i] ^= (bitVal << ((BYTES-1) - j));

            //Lança um pulso de clock e desloca o próximo bit
            digitalWrite(clockPin165, HIGH);
            delayMicroseconds(TempoDeslocamento);
            digitalWrite(clockPin165, LOW);
        }
        pinValues[nCIs-1-i] = bytesVal[nCIs-1-i];

        //altera somente uma vez a saida com o botao pressionado
        if(pinValues[nCIs-1-i] != oldPinValues[nCIs-1-i]){
            pinValuesOut[nCIs-1-i] = bytesValOut[nCIs-1-i];
        }
    }
}

void alteraSaida(){
    //alterar saida do 595

    Serial.println("Valor de PINVALUESOUT no momento de ir para o CI");
    for (int i = 0; i < nCIs; ++i){
        Serial.println(pinValuesOut[i]);
        digitalWrite(latchPin595, LOW);
        shiftOut(dataPin595, clockPin595, LSBFIRST, pinValuesOut[i]);
        digitalWrite(latchPin595, HIGH);
    }
}

//Mostra os dados recebidos
void display_pin_values()
{
    Serial.print("Estado das entradas:\r\n");

    for(int i = 0; i < nCIs; i++){
        for(int j = 0; j < BYTES; j++){
            Serial.print("  Pin0-");
            Serial.print(i);
            Serial.print(j);
            Serial.print(": ");

            if((pinValues[i] >> j) & 1)
                Serial.print("ALTO");
            else
                Serial.print("BAIXO");

            Serial.print("\r\n");
        }
    }

    Serial.print("\r\n");
}

// Configuração do Programa
void setup(){

    Ethernet.begin(mac, ip, gateway, subnet); //PASSA OS PARÂMETROS PARA A FUNÇÃO QUE VAI FAZER A CONEXÃO COM A REDE
    server.begin(); //INICIA O SERVIDOR PARA RECEBER DADOS NA PORTA 80
    pinMode(ledPin, OUTPUT); //DEFINE O PINO COMO SAÍDA
    digitalWrite(ledPin, LOW); //LED INICIA DESLIGADO


    //habilita a comunicação via monitor serial
    Serial.begin(9600);
    
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

    //inicializa com as saidas desligadas
    for(int i = 0; i < nCIs; i++){
        pinValues[i] = 0;
        oldPinValues[i] = 0;
        pinValuesOut[i] = 0;
        oldPinValuesOut[i] = 0;    
    }

    //altera as saidas para desligadas
    alteraSaida();
    //mostra no monitor
    display_pin_values();
}

//Função do loop principal
void loop(){

    EthernetClient client = server.available(); //CRIA UMA CONEXÃO COM O CLIENTE
    if (client) { // SE EXISTE CLIENTE, FAZ
        while (client.connected()) {//ENQUANTO EXISTIR CLIENTE CONECTADO, FAZ
            if (client.available()) { //SE O CLIENTE ESTÁ HABILITADO, FAZ
                char c = client.read(); //LÊ CARACTERE A CARACTERE DA REQUISIÇÃO HTTP
                if (readString.length() < 100){ //SE O ARRAY FOR MENOR QUE 100, FAZ
                    readString += c; // "readstring" VAI RECEBER OS CARACTERES LIDO
                }

                if (c == '\n') { //SE ENCONTRAR "\n" É O FINAL DO CABEÇALHO DA REQUISIÇÃO HTTP
                    if (readString.indexOf("?") <0){ //SE ENCONTRAR O CARACTER "?", FAZ
                    }else{ //SENÃO,FAZ
                        if(readString.indexOf("l") >0){ //SE ENCONTRAR O PARÂMETRO "ledParam=1", FAZ
                            controle = 1;
                            indice_out = readString.indexOf("l");
                            indice_c_out = readString.indexOf("c");
                            Serial.println("indice_out");
                            Serial.println(indice_out);

                            indice_out_int = indice_out.toInt();
                            indice_c_out_int = indice_c_out.toInt();
                            Serial.println("indice_out_int");
                            Serial.println(indice_out_int);

                            value_out = readString.charAt(indice_out_int + 2) - 48;
                            value_c_out = readString.charAt(indice_c_out_int + 2) - 48;
                            Serial.println("value_out");
                            Serial.println(value_out);
                            Serial.println("value_c_out");
                            Serial.println(value_c_out);

                            byte_out = 1;
                            for (int i = 0; i < value_out; i++){
                                byte_out *= 2;
                            }
                        
                            Serial.println("byte_out");
                            Serial.println(byte_out);
                            for(int i = 0; i < nCIs; i++){
                                if (i==value_c_out){
                                    pinValuesOut[i] ^= byte_out;
                                    break;
                                }
                                
                            }  
                        }
                    }

                    client.println("HTTP/1.1 200 OK"); //ESCREVE PARA O CLIENTE A VERSÃO DO HTTP
                    client.println("Content-Type: text/html"); //ESCREVE PARA O CLIENTE O TIPO DE CONTEÚDO(texto/html)
                    client.println("");
                    client.println("<!DOCTYPE HTML>"); //INFORMA AO NAVEGADOR A ESPECIFICAÇÃO DO HTML
                    client.println("<html>"); //ABRE A TAG "html"
                    client.println("<head>"); //ABRE A TAG "head"
                    client.println("<title>SMART HOUSE</title>"); //ESCREVE O TEXTO NA PÁGINA
                    client.println("</head>"); //FECHA A TAG "head"
                    client.println("<body style=background-color:#ADD8E6>"); //DEFINE A COR DE FUNDO DA PÁGINA
                    client.println("<center><font color='blue'><h1>SMART HOUSE</font></center></h1>"); //ESCREVE "MASTERWALKER SHOP" EM COR AZUL NA PÁGINA
                    client.println(readString);
                    client.println("<hr/>"); //TAG HTML QUE CRIA UMA LINHA HORIZONTAL NA PÁGINA
                    client.println(indice_out);
                    client.println("<h1><center>CONTROLE DE LED</center></h1>"); //ESCREVE "CONTROLE DE LED" NA PÁGINA
                    if (status == 1){ //SE VARIÁVEL FOR IGUAL A 1, FAZ
                        //A LINHA ABAIXO CRIA UM FORMULÁRIO CONTENDO UMA ENTRADA INVISÍVEL(hidden) COM O PARÂMETRO DA URL E CRIA UM BOTÃO APAGAR (CASO O LED ESTEJA LIGADO)
                        client.println("<center><form method=get name=LED><input type=hidden name=l value=0 /><input type=submit value=APAGAR></form></cente");
                    }else{ //SENÃO, FAZ
                        //A LINHA ABAIXO CRIA UM FORMULÁRIO CONTENDO UMA ENTRADA INVISÍVEL(hidden) COM O PARÂMETRO DA URL E CRIA UM BOTÃO ACENDER (CASO O LED ESTEJA DESLIGADO)
                        client.println("<center><form method=get name=LED><input type=hidden name=l value=1 /><input type=submit value=ACENDER></form></cente");
                    }
                    if (status == 1){ //SE VARIÁVEL FOR IGUAL A 1, FAZ
                        client.println("<center><font color='green' size='5'>LIGADO</font></center>"); //ESCREVE "LIGADO" EM COR VERDE NA PÁGINA
                    }else{ //SENÃO, FAZ
                        client.println("<center><font color='red' size='5'>DESLIGADO</font></center>"); //ESCREVE "DESLIGADO" EM COR VERMELHA NA PÁGINA
                    }
                    client.println("<hr/>"); //TAG HTML QUE CRIA UMA LINHA HORIZONTAL NA PÁGINA
                    client.println("<p>valor capturado</p>"); //TAG HTML QUE CRIA UMA LINHA HORIZONTAL NA PÁGINA
                    client.println(value_out);
                    client.println("<hr/>"); //TAG HTML QUE CRIA UMA LINHA HORIZONTAL NA PÁGINA
                    client.println("<p>valor capturado em byte</p>"); //TAG HTML QUE CRIA UMA LINHA HORIZONTAL NA PÁGINA
                    client.println(byte_out);
                    client.println("<hr/>"); //TAG HTML QUE CRIA UMA LINHA HORIZONTAL NA PÁGINA
                    client.println("<p>valor da saida</p>"); //TAG HTML QUE CRIA UMA LINHA HORIZONTAL NA PÁGINA
                    client.println(pinValuesOut[0]);
                    client.println("<hr/>"); //TAG HTML QUE CRIA UMA LINHA HORIZONTAL NA PÁGINA
                    client.println("<p>valor do controle</p>"); //TAG HTML QUE CRIA UMA LINHA HORIZONTAL NA PÁGINA
                    client.println(controle);
                    client.println("<hr/>"); //TAG HTML QUE CRIA UMA LINHA HORIZONTAL NA PÁGINA
                    client.println("<p>readString</p>"); //TAG HTML QUE CRIA UMA LINHA HORIZONTAL NA PÁGINA
                    client.println(readString);
                    client.println("<hr/>"); //TAG HTML QUE CRIA UMA LINHA HORIZONTAL NA PÁGINA
                    client.println("<p>indice_capturado</p>"); //TAG HTML QUE CRIA UMA LINHA HORIZONTAL NA PÁGINA
                    client.println(indice_out);
                    client.println("<hr/>"); //TAG HTML QUE CRIA UMA LINHA HORIZONTAL NA PÁGINA
                    client.println("<a type= 'button' href='http://127.0.0.1/smarthouse/index.php'>VOLTAR</a>"); //TAG HTML QUE CRIA UMA LINHA HORIZONTAL NA PÁGINA
                    client.println("<meta http-equiv='Refresh' content='0; url=http://127.0.0.1/smarthouse/index.php' />"); //ESCREVE O TEXTO NA PÁGINA
                    client.println("</body>"); //FECHA A TAG "body"
                    client.println("</html>"); //FECHA A TAG "html"
                    readString=""; //A VARIÁVEL É REINICIALIZADA
                    client.stop(); //FINALIZA A REQUISIÇÃO HTTP E DESCONECTA O CLIENTE
                }
            }
        }
    }




    //Lê todos as portas externas
    read_shift_regs();
    
    //Se houver modificação no estado dos pinos, mostra o estado atual
    for(int i = 0; i < nCIs; i++){
        if(pinValues[i] != oldPinValues[i]){
            Serial.print("*Alteracao detectada*\r\n");
            display_pin_values();
            Serial.print("Valor das entradas em decimal:\n");
            Serial.print(i);
            Serial.print("\t");
            Serial.print(pinValues[i]);
            oldPinValues[i] = pinValues[i];
        }
    }
    //altera a saída se existir alguma mudança
    for(int i = 0; i < nCIs; i++){
        if (oldPinValuesOut[i] != pinValuesOut[i]){
            alteraSaida();
            oldPinValuesOut[i] = pinValuesOut[i];
        }
    }

    delay(Atraso);
}
