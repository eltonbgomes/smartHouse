const int pinoPOT = A0; //PINO ANALÓGICO UTILIZADO PELO POTENCIÔMETRO
float luminosidadeLED = 0; //VARIÁVEL QUE ARMAZENA O VALOR DA LUMINOSIDADE QUE SERÁ APLICADA AO LED
float luminosidadeLED0 = 0; //VARIÁVEL QUE ARMAZENA O VALOR DA LUMINOSIDADE QUE SERÁ APLICADA AO LED

void setup(){
   Serial.begin(9600);
   pinMode(pinoPOT, INPUT); //DEFINE O PINO COMO ENTRADA
}

void loop(){
  luminosidadeLED0 = map(analogRead(pinoPOT), 0, 1023, 0, 255); //EXECUTA A FUNÇÃO "map" DE ACORDO COM OS PARÂMETROS PASSADOS
  luminosidadeLED = analogRead(pinoPOT); //EXECUTA A FUNÇÃO "map" DE ACORDO COM OS PARÂMETROS PASSADOS
  Serial.print(luminosidadeLED0);
  Serial.print("\n");
  Serial.print(luminosidadeLED);
  Serial.print("\n");
  delay(1500);
}
