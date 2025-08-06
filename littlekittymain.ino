#include <ArduinoJson.h>

#define I1 4
#define I2 5
#define E1 6
#define I3 7
#define I4 8
#define E2 9

#define V 6
#define R 7
#define G 8
#define B 9

#define Encoder1 2
#define Encoder2 3

#define R_weel 0.067
#define R_enc 0.0135
#define Rot_enc 116
#define RPM 16.6667
#define spd2pwm 1593.75
#define Kp 1
#define Ki 0
#define Kd 0

#define start_color 0x34248C
#define NO_conn_color 0xAB2748

bool conn = false;

float VR = 0;
float VL = 0;
float spd1 = 0, spd2 = 0;

float integral, pInput;


unsigned long tmr=0;
unsigned long enc1_tmr=0, enc2_tmr=0;
unsigned long pid_tmr=0;


void setup() {
  // MOTOR
  pinMode(I1, OUTPUT);
  pinMode(I2, OUTPUT);
  pinMode(E1, OUTPUT);
  pinMode(I3, OUTPUT);
  pinMode(I4, OUTPUT);
  pinMode(E2, OUTPUT);
  //LEDtrip
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  //Encoders
    pinMode(Encoder1, INPUT);
    pinMode(Encoder2, INPUT);

    pinMode(13,OUTPUT);

    attachInterrupt(digitalPinToInterrupt(Encoder1),Enc_counter_1,RISING);
    attachInterrupt(digitalPinToInterrupt(Encoder2),Enc_counter_2,RISING);

  //off
  set_spd(0,0);
  digitalWrite(R, HIGH);
  digitalWrite(G, HIGH);
  digitalWrite(B, HIGH);
  
  //start
  setColor(start_color, 128);
  delay(1000);

  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!conn) setColor(NO_conn_color, 128);
  else setColor(0, 128);

  if (Serial.available()) { 
    //String inData = Serial.readStringUntil('\n');
    //Serial.println(inData);
    conn=true;
    tmr = millis();
    JsonDocument recv;
    DeserializationError err = deserializeJson(recv, Serial);
    
    if (err == DeserializationError::Ok) { 
      VR = recv["Motor"]["Vr"];
      VL = recv["Motor"]["Vl"];
      Serial.read();
    }
  }
  else if (millis()-tmr>=100){
    conn=false;
    VR = 0;
    VL = 0;
  }

  set_spd(VR,VL);
  
  
  

  JsonDocument transm;
  transm["enc"]["Right"] = spd2;
  transm["enc"]["Left"] = spd1;
  String buffer;
  serializeJsonPretty(transm, buffer);
  Serial.println(buffer);
  //delay(10);

  del_spd(&spd1,&enc1_tmr);
  del_spd(&spd2,&enc2_tmr);
}







void Enc_counter_1(){
  spd1 = 1./(micros()-enc1_tmr)*1000000./Rot_enc/2/3.141593*R_weel;
  enc1_tmr = micros();
}
void Enc_counter_2(){
  spd2 = 1./(micros()-enc2_tmr)*1000000./Rot_enc/2/3.141593*R_weel;
  enc2_tmr = micros();
}

void setColor(uint32_t color, uint8_t H) {
  analogWrite(V, H);
  analogWrite(R, (color >> 16) & 0xff);
  analogWrite(G, (color >> 8)  & 0xff);
  analogWrite(B, color & 0xff);
}


void PID(float setpoint, float input, uint8_t pin) {
  int speed = setpoint*spd2pwm;;
  analogWrite(pin, abs(speed));
  

}

void del_spd(float* spd, unsigned long* timer){
  if (micros() - *timer>100) {
    *spd=0;
    *timer=micros();
  } 
}

void set_spd(float speed1 , float speed2){
  if (speed1>0.01) {
    digitalWrite(I1,HIGH);
    digitalWrite(I2,LOW);
  }
  else if (speed1<-0.01) {
    digitalWrite(I1,LOW);
    digitalWrite(I2,HIGH);
  }
  else {
    digitalWrite(I1,LOW);
    digitalWrite(I2,LOW);
  }
  
  if (speed2>0.01) {
    digitalWrite(I3,HIGH);
    digitalWrite(I4,LOW);
  }
  else if (speed2<-0.01) {
    digitalWrite(I3,LOW);
    digitalWrite(I4,HIGH);
  }
  else {
    digitalWrite(I3,LOW);
    digitalWrite(I4,LOW);
  }

  PID(speed1, spd1, E1);
  PID(speed2, spd2, E2);
  pid_tmr = millis();
}