/*
=====================================================================
Práctica 1. Módulo 4. Control de velocidad

Presenta:
Zaida Irais López Mendieta - A01708755
Francisco Alejandro Velázquez Ledesma - A01709475
Julio David Reséndiz Cruz - A01709375

=====================================================================
*/

float kp=1;
float ki=10.5;
float kd=0.025;
float y[2] = {0,0}; //salida y[0] salida actual, y[1] anterior
float e[3] = {0,0,0}; //e[0] error actual, e[1] error anterior
float K1, K2, K3;

//Definir pines de entrada y salida
#define IN_1 26 //Entrada 1 del puente H
#define IN_2 27 //Entrada 2 del puente H
#define EN 14   //ENABLE del puente H

#define EN_A 18 //Canal A del encoder
#define EN_B 19 //Canal B del encoder

//Configuracion del PWM
#define PWM_CH 0
#define FREQ 5000
#define RES 8

//tiempo de muestreo
#define DT 10000

//Caracteristicas del motor
#define REDUCTOR 34
#define CPR 12

//Variables del programa
double pos = 0;
double pos_ant = 0;
double timer = 0;
float vel = 0;
int duty_cycle = 100;

//Función que se ejecuta cada vez que hay un cambio (FALLING) en el canal A del encoder
void IRAM_ATTR int_callback(){
  if (digitalRead(EN_B)==0){
    pos = pos + 1;
  } else {
    pos = pos - 1;
  }
}

void setup() {
  //Inicializar puerto serial
  Serial.begin(115200);
  
  // Definir entradas y salidas
  pinMode (IN_1, OUTPUT);
  pinMode (IN_2, OUTPUT);
  pinMode (EN, OUTPUT);

  pinMode (EN_A, INPUT);
  pinMode (EN_B, INPUT);

  //Definimos la interrupción la cual se ejecutara en cada flanco de bajada del canal A del encoder
  attachInterrupt(EN_A, int_callback, FALLING);

  //Configurar el canal de PWM
  ledcSetup(PWM_CH, FREQ, RES);
  ledcAttachPin(EN, PWM_CH);

  //****

  K1 = kp + (DT/1000000.0)*ki + kd/(DT/1000000.0);
  K2 = - kp - 2.0*kd/(DT/1000000.0);
  K3 = kd/(DT/1000000.0);

}

float referencia = 90;

void loop() {
  //Revisar si ha transcurrido un periodo de tiempo
  if ((micros()-timer)>DT){
    timer = micros();
    //Estimadando la velocidad del motor en rpm
    vel = (((pos - pos_ant)/DT)/(CPR*REDUCTOR))*(1000000.0*60.0);
    pos_ant = pos;

    e[0] = referencia - vel;
    y[0] = K1*e[0] + K2*e[1] + K3*e[2] + y[1];
    //limitar la salida
    if(y[0] > 255) y[0] = 255;
    if(y[0] < -255) y[0] = -255;

    e[2] = e[1];
    e[1] = e[0]; 
    y[1] = y[0];

    if (y[0] >= 0){
      digitalWrite(IN_2, HIGH);
      digitalWrite(IN_1, LOW);
      ledcWrite(PWM_CH, y[0]);
    }
    else{
      digitalWrite(IN_2, LOW);
      digitalWrite(IN_1, HIGH);
      ledcWrite(PWM_CH, -y[0]);
    }

    Serial.print(referencia);
    Serial.print("   ");
    Serial.print(vel);
    Serial.print("   ");
    Serial.print(e[0]);
    Serial.print("   ");
    Serial.println(y[0]);
      
  }

}

String mensaje;
int indexa = 0;
void serialEvent()
{
  char ch = Serial.read();
  mensaje.concat(ch);
  if(mensaje.charAt(indexa) == '\n' && mensaje.charAt(indexa-1) == '\r')
  {
      indexa = 0;
      referencia = mensaje.toFloat();
      mensaje = "";
  }
else{
      indexa++;
     }
 }
