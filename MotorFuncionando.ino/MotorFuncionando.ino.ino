/*
This is a test sketch for the Adafruit assembled Motor Shield for Arduino v2
It won't work with v1.x motor shields! Only for the v2's with built in PWM
control

For use with the Adafruit Motor Shield v2
----> http://www.adafruit.com/products/1438
*/

#include <Adafruit_MotorShield.h>
#include <SoftwareSerial.h>   

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61);

// Select which 'port' M1, M2, M3 or M4. In this case, M1
Adafruit_DCMotor *myMotor1 = AFMS.getMotor(1);
Adafruit_DCMotor *myMotor2 = AFMS.getMotor(2);
// You can also make another motor on port M2
//Adafruit_DCMotor *myOtherMotor = AFMS.getMotor(2);
SoftwareSerial BT(10,11);

//SENSOR 2
#define echoPin 2 
#define trigPin 4
//SENSOR 1
#define echoPin2 7 
#define trigPin2 6
//otros
#define salida 13
#define ledPin 13
#define NROBOT 1

//CALCULO DISTANCIAS CON LOS SENSORES
int sensor1(){
  long duracion;
  int distancia;  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duracion = pulseIn(echoPin, HIGH); //microsegundos
  distancia = duracion * 0.034 / 2;
  return distancia;
}

int sensor2(){
  long duracion;
  int distancia;  
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);
  duracion = pulseIn(echoPin2, HIGH); //microsegundos
  distancia = duracion * 0.034 / 2;
  return distancia;
}

unsigned char * mensaje;
unsigned char *buffer;

void setup() {
  BT.begin(9600);  //AT1:9600 AT2:38400  
  Serial.begin(9600); // set up Serial library at 9600 bps
  Serial.println("Adafruit Motorshield v2 - DC Motor test!");
  servo1.attach(10);
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT);
  pinMode(trigPin2, OUTPUT); 
  pinMode(echoPin2, INPUT);
  pinMode(salida, OUTPUT);
  pinMode(8, INPUT);
  digitalWrite(salida,HIGH);
  
  

  if (!AFMS.begin()) { // create with the default frequency 1.6KHz
  // if (!AFMS.begin(1000)) { // OR with a different frequency, say 1KHz
    Serial.println("Could not find Motor Shield. Check wiring.");
    while (1);
  }
  Serial.println("Motor Shield found.");

  // Set the speed to start, from 0 (off) to 255 (max speed)
  myMotor1->setSpeed(70);
  myMotor1->run(FORWARD);
  myMotor1->run(RELEASE);
  myMotor2->setSpeed(70);
  myMotor2->run(FORWARD);
  myMotor2->run(RELEASE);
}

int st_buffer = -1, n=0;
void recibir_mensaje(){
  unsigned char aux=0;
  if (BT.available()) {
    aux = BT.read();
    if (st_buffer==0 && aux=='}'){
       st_buffer=1;
       for (int i=0; i<n; i++)
        mensaje[i]=buffer[i];
       n=0;              
    }  

    if (st_buffer==0 && aux!='}'){
      buffer[n] = aux;
      n++;
    }
   if (st_buffer==-1 && aux=='{'){
       st_buffer=0;
    }  


  }
}
unsigned char c;

//ESTADOS
#define INICIO 0
#define G1 1 // avanza hacia grua 1
#define E1 2 // espera la carga de grua 1
#define G2 3 // avanza hacia grua 2
#define E2 4 // espera la descarga de grua 2
#define W 5 // warning de distancia

// ESTADOS INTERNOS
#define cargado 6
#define descargado 7
#define salida 13

int st_carrito = descargado;
int ESTADO = INICIO;
int DISTANCIA;
int DISTANCIA2;
int buttonState;

// BUFFER = [GRUA1,GRUA2,ROBOT1,ROBOT2]
//
// 1 = DISPONIBLE Y 0 = FIN
void loop() {

  DISTANCIA = sensor1();
  DISTANCIA2 = sensor2();
  buttonState = digitalRead(8);
  Serial.println(DISTANCIA);
  Serial.println(DISTANCIA2);
  Serial.println(buttonState);
  Serial.println(ESTADO);
  Serial.println("----------------");
  recibir_mensaje()  

  if (DISTANCIA<20 ){
       ESTADO = W;
  }

  if (DISTANCIA<20 && mensaje[0] == 1 && st_carrito == descargado){
      ESTADO = E1;       
  }
  if (DISTANCIA<20 && mensaje[0] == 0 && st_carrito == descargado){
      ESTADO = G2;
      st_carrito = cargado;       
  }

  if (DISTANCIA2<20 && mensaje[1] == 1 && st_carrito == cargado){
      ESTADO = E2;     
  }
  if (DISTANCIA2<20 && mensaje[1] == 0 && st_carrito == cargado){
      ESTADO = G1;
      st_carrito = descargado;       
  }

  if(ESTADO == E1 || ESTADO == E2 || ESTADO==W){
      myMotor1->run(RELEASE);
      myMotor2->run(RELEASE); 
  }  

  if(ESTADO == G2){
    myMotor1->run(BACKWARD);
    myMotor2->run(BACKWARD);
    myMotor1->setSpeed(70);
    myMotor2->setSpeed(70);
  }  

  if (ESTADO == G1) {
    myMotor1->run(FORWARD);
    myMotor2->run(FORWARD);
    myMotor1->setSpeed(70);
    myMotor2->setSpeed(70);
  }
  delay(10);
}