/********************************************************************************\
 * Programa Expansão de Entradas e saídas do Arduino, utilizando Shift Register *
 * CI utilizado: 74HC165 com 74HC595                                            *
 * Por: Elton Barbosa Gomes                                                     *
 * Data: 28/09/2020                                                             *
 * Créditos: Baseado no playground.arduino.cc                                   *
\********************************************************************************/

#include "WiFiEsp.h" //INCLUSÃO DA BIBLIOTECA para ESP01
#include "SoftwareSerial.h"//INCLUSÃO DA BIBLIOTECA para ESP01

//74hc165
// Definições de Labels
    #define nCIs  1               //Registra o número de CIs cascateados
    #define BYTES 8
    #define TempoDeslocamento 50  //Registra o tempo de que deverá ter o pulso para leitura e gravação, (milesegundos)
    #define Atraso  100           //Registra o atraso de segurança entre leituras, (milesegundos)
    #define BYTES_VAL_T unsigned int  //Altera de int para long, se o a quantidade de CIs cascateados for maior que 2

//inicialização para ESP01
    SoftwareSerial Serial1(10, 9); //PINOS QUE EMULAM A SERIAL, ONDE O PINO 9 É O RX E O PINO 10 É O TX

    char ssid[] = "Musphelhein"; //VARIÁVEL QUE ARMAZENA O NOME DA REDE SEM FIO
    char pass[] = "8380786051";//VARIÁVEL QUE ARMAZENA A SENHA DA REDE SEM FIO

    int status = WL_IDLE_STATUS; //STATUS TEMPORÁRIO ATRIBUÍDO QUANDO O WIFI É INICIALIZADO E PERMANECE ATIVO
    //ATÉ QUE O NÚMERO DE TENTATIVAS EXPIRE (RESULTANDO EM WL_NO_SHIELD) OU QUE UMA CONEXÃO SEJA ESTABELECIDA
    //(RESULTANDO EM WL_CONNECTED)

    WiFiEspServer server(80); //CONEXÃO REALIZADA NA PORTA 80

    RingBuffer buf(8); //BUFFER PARA AUMENTAR A VELOCIDADE E REDUZIR A ALOCAÇÃO DE MEMÓRIA


// Declaração de variáveis globais 165
    const int ploadPin165        = 6;    //Conecta ao pino 1 do 74HC165 (LH/LD - asynchronous parallel load input)(PL)
    const int clockEnablePin165  = 5;    //Conecta ao pino 15 do 74HC165 (CE - Clock Enable Input)(CE)
    const int dataPin165         = 8;   //Conecta ao pino 9 do 74HC165 (Q7 - serial output from the last stage)(Q7)
    const int clockPin165        = 7;   //Conecta ao pino 2 do 74HC165 (CP - Clock Input)(CP)

//inicialização das variaveis onde serão armazenados os status
    BYTES_VAL_T pinValues[nCIs];
    BYTES_VAL_T oldPinValues[nCIs];
    BYTES_VAL_T pinValuesOut[nCIs];
    BYTES_VAL_T oldPinValuesOut[nCIs];


// Declaração de variáveis globais 595
    int clockPin595 = 2; //Pino conectado a SRCLK (pino 11 no 74HC595), registrador de deslocamento
    int latchPin595 = 3; //Pino conectado a RCLK (pino 12 no 74HC595), registrador de armazenamento
    int dataPin595 = 4; //Pino conectado a SER (pino 14 no 74HC595), entrada de dados serial


//Função para definir um rotina shift-in, lê os dados do 74HC165
void read_shift_regs(){
    bool bitVal=0; //variavel para que armazena bit a bit
    BYTES_VAL_T bytesVal[nCIs];
    BYTES_VAL_T bytesValOut[nCIs];
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

    // Efetua a leitura de um bit da saida serial do 74HC165

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

        //condição para somente alterar as saídas se soltar o botão
        if(pinValues[nCIs-1-i] != oldPinValues[nCIs-1-i]){
            pinValuesOut[nCIs-1-i] = bytesValOut[nCIs-1-i];
        }
    }
}

void alteraSaida(){
    //alterar saida do 595
    for (int i = 0; i < nCIs; ++i){
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

//MÉTODO DE RESPOSTA A REQUISIÇÃO HTTP DO CLIENTE
void sendHttpResponse(WiFiEspClient client){
    client.println("HTTP/1.1 200 OK"); //ESCREVE PARA O CLIENTE A VERSÃO DO HTTP
    client.println("Content-Type: text/html"); //ESCREVE PARA O CLIENTE O TIPO DE CONTEÚDO(texto/html)
    client.println("");
    client.println("<!DOCTYPE HTML>"); //INFORMA AO NAVEGADOR A ESPECIFICAÇÃO DO HTML
    client.println("<html>"); //ABRE A TAG "html"
    client.println("<head>"); //ABRE A TAG "head"
    client.println("<link rel='icon' type='image/png' href='https://blogmasterwalkershop.com.br/arquivos/artigos/sub_wifi/icon_mws.png'/>"); //DEFINIÇÃO DO ICONE DA PÁGINA
    client.println("<link rel='stylesheet' type='text/css' href='https://blogmasterwalkershop.com.br/arquivos/artigos/sub_wifi/webpagecss.css' />"); //REFERENCIA AO ARQUIVO CSS (FOLHAS DE ESTILO)
    client.println("<title>MasterWalker Shop - Modulo WiFi ESP8266 para Arduino</title>"); //ESCREVE O TEXTO NA PÁGINA
    client.println("</head>"); //FECHA A TAG "head"

    //AS LINHAS ABAIXO CRIAM A PÁGINA HTML
    client.println("<body>"); //ABRE A TAG "body"
    client.println("<img alt='masterwalkershop' src='https://blogmasterwalkershop.com.br/arquivos/artigos/sub_wifi/logo_mws.png' height='156' width='700' />"); //LOGO DA MASTERWALKER SHOP
    client.println("<p style='line-height:2'><font>Modulo WiFi ESP8266 para Arduino</font></p>"); //ESCREVE O TEXTO NA PÁGINA
    client.println("<font>ESTADO ATUAL DO LED</font>"); //ESCREVE O TEXTO NA PÁGINA

    client.println("<hr />"); //TAG HTML QUE CRIA UMA LINHA NA PÁGINA
    client.println("<hr />"); //TAG HTML QUE CRIA UMA LINHA NA PÁGINA
    client.println("</body>"); //FECHA A TAG "body"
    client.println("</html>"); //FECHA A TAG "html"
    delay(1); //INTERVALO DE 1 MILISSEGUNDO
}

// Configuração do Programa
void setup()
{
    //habilita a comunicação via monitor serial
    Serial.begin(9600);

    Serial1.begin(9600); //INICIALIZA A SERIAL PARA O ESP8266
    WiFi.init(&Serial1); //INICIALIZA A COMUNICAÇÃO SERIAL COM O ESP8266
    WiFi.config(IPAddress(192,168,0,220)); //COLOQUE UMA FAIXA DE IP DISPONÍVEL DO SEU ROTEADOR

    //INÍCIO - VERIFICA SE O ESP8266 ESTÁ CONECTADO AO ARDUINO, CONECTA A REDE SEM FIO E INICIA O WEBSERVER
    if(WiFi.status() == WL_NO_SHIELD){
        while (true);
    }
    while(status != WL_CONNECTED){
        status = WiFi.begin(ssid, pass);
    }
    server.begin();
    //FIM - VERIFICA SE O ESP8266 ESTÁ CONECTADO AO ARDUINO, CONECTA A REDE SEM FIO E INICIA O WEBSERVER
    
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

    //Lê e mostra o estado dos pinos
    for(int i = 0; i < nCIs; i++){
        pinValues[i] = 0;
        oldPinValues[i] = 0;
        pinValuesOut[i] = 0;
        oldPinValuesOut[i] = 0;    
    }

    alteraSaida();
    display_pin_values();
}

//Função do loop principal
void loop(){
    WiFiEspClient client = server.available(); //ATENDE AS SOLICITAÇÕES DO CLIENTE

    if (client) { //SE CLIENTE TENTAR SE CONECTAR, FAZ
        buf.init(); //INICIALIZA O BUFFER
        while (client.connected()){ //ENQUANTO O CLIENTE ESTIVER CONECTADO, FAZ
            if(client.available()){ //SE EXISTIR REQUISIÇÃO DO CLIENTE, FAZ
                char c = client.read(); //LÊ A REQUISIÇÃO DO CLIENTE
                buf.push(c); //BUFFER ARMAZENA A REQUISIÇÃO

                //IDENTIFICA O FIM DA REQUISIÇÃO HTTP E ENVIA UMA RESPOSTA
                if(buf.endsWith("\r\n\r\n")) {
                    sendHttpResponse(client);
                    break;
                }
            }
        }
        client.stop(); //FINALIZA A REQUISIÇÃO HTTP E DESCONECTA O CLIENTE
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
