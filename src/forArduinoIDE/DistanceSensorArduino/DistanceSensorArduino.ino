//https://2kuru.com/m5stack_atom_matrix_spec/
//https://www.mcucity.com/product/2926/hc-sr04-3-3v-5v-ultrasonic-distance-measuring-sensor-module-trig-echo-uarttx-rx-i2csdascl

#include <Arduino.h>
#include "tmDeltaTime.hpp"

#define INI_ATOMS3
//#define INI_ATOM
//#define INI_XIAO_RP2040

#ifdef INI_ATOMS3
#define TRIG_PIN (5)
#define ECHO_PIN (6)
#define MOTOR_PIN (8)
#define MOTOR2_PIN (7)
#define OUT_PIN (2)
#endif
#ifdef INI_ATOM
#define TRIG_PIN (25)
#define ECHO_PIN (21)
#define MOTOR_PIN (33)
#define MOTOR2_PIN (23)
#define OUT_PIN (26)
#endif
#ifdef INI_XIAO_RP2040
#include <Adafruit_NeoPixel.h>
#define TRIG_PIN (D1)
#define ECHO_PIN (D2)
#define MOTOR_PIN (D10)
#define MOTOR2_PIN (D9)
#define OUT_PIN (D3)
#define NUMPIXELS 1
#define NEO_PWR 11 //GPIO11
#define NEOPIX 12 //GPIO12
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIX, NEO_GRB + NEO_KHZ800);
#endif

#define TIMEOUT_US (100000)
#define MAX_DIST_MM (4000)

int32_t g_dist_mm;
int32_t g_mtTime;
bool g_mtIsOn;
TmDeltaTime* pTdt= new TmDeltaTime();

void updateDist(uint32_t deltaTime){
  digitalWrite(TRIG_PIN,HIGH);
  delayMicroseconds(100);
  digitalWrite(TRIG_PIN,LOW); // Trig pin outputs 10US high level pulse trigger signal

  unsigned long dist = pulseIn(ECHO_PIN,HIGH,TIMEOUT_US); // Count the received high time
  g_dist_mm = (dist==0) ? MAX_DIST_MM : (int32_t)(dist*340/2/1000); // Calculation distance
  Serial.println(g_dist_mm); // Serial port output distance signal
}

void updateMotorPin(bool isOn){
  digitalWrite(MOTOR_PIN,isOn);
  digitalWrite(MOTOR2_PIN,isOn);
  digitalWrite(OUT_PIN,isOn);
#ifdef INI_XIAO_RP2040
  if(isOn){
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  }else{
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  }
  pixels.show();
#endif
}

void updateMotor(uint32_t deltaTime){
  if(!g_mtIsOn){
    if(g_dist_mm>=MAX_DIST_MM){
      return;
    }
    if(g_dist_mm<g_mtTime){
      g_mtTime=g_dist_mm;
    }
    g_mtTime -= deltaTime;
    //Serial.println(g_mtIsOn?"++":"--");
    if(g_mtTime<0){
      g_mtIsOn=!g_mtIsOn;
      updateMotorPin(g_mtIsOn);
      g_mtTime+= ((g_dist_mm>1500)?150:75);
    }
  }else{
    g_mtTime -= deltaTime;
    //Serial.println(g_mtIsOn?"++":"--");
    if(g_mtTime<0){
      g_mtIsOn=!g_mtIsOn;
      updateMotorPin(g_mtIsOn);
      g_mtTime+=(g_dist_mm>>2);
    }
  }
}

void setup() {
#ifdef INI_XIAO_RP2040
  pixels.begin();
  pixels.setBrightness(20);
  pinMode(NEO_PWR,OUTPUT);  digitalWrite(NEO_PWR, HIGH);
  delay(200);
#endif

  Serial.begin(115200);
  Serial.println("Start");
  pinMode(ECHO_PIN,INPUT);
  pinMode(TRIG_PIN,OUTPUT);
  pinMode(MOTOR_PIN,OUTPUT);
  pinMode(MOTOR2_PIN,OUTPUT);
  pinMode(OUT_PIN,OUTPUT);
  digitalWrite(MOTOR_PIN,HIGH);
  g_dist_mm=0;
  g_mtTime=1000;
  g_mtIsOn=false;
  pTdt->Setup();
  pTdt->AddTrig(updateDist,100);
  pTdt->AddTrig(updateMotor,25);
}

void loop() {
  pTdt->Update();
  delay(5);
}