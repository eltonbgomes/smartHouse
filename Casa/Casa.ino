int clockPin = 2; //Pino conectado a SRCLK (pino 11 no 74HC595), registrador de deslocamento
int latchPin = 3; //Pino conectado a RCLK (pino 12 no 74HC595), registrador de armazenamento
int dataPin = 4; //Pino conectado a SER (pino 14 no 74HC595), entrada de dados serial

int a = 5;
int b = 8; //tamanho da matriz
int lampadasStatus[5][8] = {
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0}
};
int i,j;
int resto, dec;
int statusI = -1;
int statusJ = -1;
int tempo = 2500;

int pot(int num){ //função para calcular potencia de base 2;
  int n = 1;
  for(int i = 1; i <= num; i++){
    n *= 2;
  }
  return n;
}

void setup(){
  Serial.begin(9600);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
}

void loop(){
  for(i = 0; i < a; i++){ //acesso a matriz das lâmpadas
    dec = 0;
    for(j = 0; j < b; j++){
      if(statusI == i && statusJ == j){
        if(lampadasStatus[i][j] == 0){
          lampadasStatus[i][j] = 1;
        }else{
          lampadasStatus[i][j] = 0;
        }
      }
      resto = lampadasStatus[i][j] % 2; //verifica cada bit separadamente
      dec += resto * pot(7-j); //converte o bit em decinal
    }
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, dec);
    digitalWrite(latchPin, HIGH);
  }

  Serial.print("Digite a linha do endereco da lampada: ");
  do{
    statusI= Serial.read() - 48;
  }while(statusI < 0 || statusI >= a);
  Serial.println(statusI);

  Serial.print("Digite a coluna do endereco da lampada: ");
  do{
    statusJ = Serial.read() - 48;
  }while(statusJ < 0 || statusJ >= b);
  Serial.println(statusJ);
}
