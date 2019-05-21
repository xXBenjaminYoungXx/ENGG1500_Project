
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//***********Librarys**************//
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#include <SparkFun_APDS9960.h>//RGB sensor
#include <ENGG1500Lib.h>//Encoder stuff
#include <Servo.h> //makes the sonar magic happen
#include <Wire.h>

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//***********Servo variables**************//
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define SERVO 10
Servo servo;
int servoAngle = 0; //Servo intial position
int Time;
uint8_t ProxF = 30;
uint8_t ProxL = 30;
uint8_t ProxR = 30;

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//***********RGB variables**************//
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

SparkFun_APDS9960 apds = SparkFun_APDS9960();
uint8_t proximity_data = 0;
uint16_t ambient_light = 0;
uint16_t red_light = 0;
uint16_t green_light = 0;
uint16_t blue_light = 0;

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//***********IR reading variables**************//
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

float w1;
float w2;
float w3;
float w4;

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//***********PWM Controll Variables**************//
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int standardSpd = 70;
int speedDiff = 0;

float den;
float numer;
const int LPWM = 82;
const int RPWM = 85;
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//***********State machiene variables**************//
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int State = 1;
unsigned int time_ = 0;
/*
 * 0: End
 * 1: Follow line
 * 2: Dead End
 * 3: Hallway
 * 4: Gararge
 * 5: Scan
 */

void setup() {
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//***********RGB SETUP**************//
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    Serial.begin(9600);
    Serial.println(F("------------------------------------"));
    Serial.println(F("SparkFun APDS-9960 - ProximitySensor"));
    Serial.println(F("------------------------------------"));
  
    // Initialize APDS-9960 (configure I2C and initial values)
    if ( apds.init() ) {
      Serial.println(F("APDS-9960 initialization complete"));
    } else {
      Serial.println(F("Something went wrong during APDS-9960 init!"));
    }
  
    // Adjust the Proximity sensor gain
    if ( !apds.setProximityGain(PGAIN_2X) ) {
      Serial.println(F("Something went wrong trying to set PGAIN"));
    }
  
    // Start running the APDS-9960 proximity sensor (no interrupts)
    if ( apds.enableProximitySensor(false) ) {
      Serial.println(F("Proximity sensor is now running"));
    } else {
      Serial.println(F("Something went wrong during sensor init!"));
    }

    // Start running the APDS-9960 light sensor (no interrupts)
    if ( apds.enableLightSensor(false) ) {
      Serial.println(F("Light sensor is now running"));
    } else {
      Serial.println(F("Something went wrong during light sensor init!"));
    }
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//***********SERVO SETUP**************//
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    servo.attach(SERVO);
    servo.write(90);
     
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//***********IR SENSOR SETUP**************//
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    //IR sensors
    pinMode(A0, INPUT);//IR front
    pinMode(A1, INPUT);//IR front
    pinMode(A2, INPUT);//IR front
    pinMode(A3, INPUT);//IR front
     
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//***********MOTOR SETUP**************//
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
     
    //Motor Speeds
    pinMode(5,OUTPUT); //set ENA as an output
    pinMode(6,OUTPUT); //set ENB as an output
    //Motor directions
    pinMode(8,OUTPUT); //set IN1 as an output
    pinMode(9,OUTPUT); //set IN2 as an output
    pinMode(11,OUTPUT); //set IN3 as an output
    pinMode(12,OUTPUT); //set IN4 as an output
    
    delay(5000);
}

void loop() {
  if ( !apds.readProximity(proximity_data) ) {
    Serial.println("Error reading proximity value");
  }
  
  w1 = analogRead(A0);
  w2 = analogRead(A1);
  w3 = analogRead(A2);
  w4 = analogRead(A3);

  if((w1 <50 && w2 < 50 && w3 <50 && w4 < 50) && State != 0) {//We are on white, but we could just be on small gap, so wait a second to see if this is still the case
    
     time_ = millis();
     if(time_ + 1000 < millis()){//Second has passed 
          State = 5;
      }
  }

  if(proximity_data > 150 && State != 0){//Wall is seen need to scan
      State = 5;
  }

  if(State == 0){
     Stop();
  }
  
  if(State == 1){//Follow line and scan
    followLine();
  }

  if(State = 2){
    //Wall code
  }

  if(State = 3){
    Corridor();
  }

  if(State = 4){
    Garage();
  }

  if(State = 5){//Scan
        Halt();
        
        servo.write(0);
        delay(200);
        apds.readProximity(proximity_data);
        ProxR = proximity_data;
    
        servo.write(90);
        delay(200);
        apds.readProximity(proximity_data);
        ProxF = proximity_data;
    
        servo.write(180);
        delay(200);
        apds.readProximity(proximity_data);
        ProxL = proximity_data;
    
        servo.write(90);
    
        //Now to see what to do with results
        if(ProxF > 150 && ProxR <150 && ProxL < 150){//We are at dead end
          State = 2;
        }
    
        else if(ProxF < 150 && ProxR > 150 && ProxL > 150){//We are at coridor, converging / diverging
          State = 3;
        }
    
        else if(ProxF > 150 && ProxR > 150 && ProxL > 150){//We are at gararge
          State = 4;
        }
    
        else if(ProxF < 150 && ProxR < 150 && ProxL < 150){//We are on a white part of track, hopefully this never occurs
          State = 1;
        }
      }
}
