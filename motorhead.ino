/*
 * FlySky IBus interface on an Arduino stm32f030.
 *  Connect FS-iA6B receiver to Serial1.
 */

#include <math.h> 
#include "FlySkyIBus.h"
#include "SparkFun_TB6612.h"

uint16_t pwmMin = 900;   //0x384
uint16_t pwmMid = 1500;  //0x5DC  
uint16_t pwmMax = 2098;  //0x832

int16_t  speedMin = -255;
int16_t  speedMax =  255;

uint16_t InputMax = 600;

struct motorDrive
{
  int16_t LeftD;
  int16_t RightD;  
};

struct InputChannel
{
  uint16_t Aileron;
  uint16_t Elevator;  
};

// Pins for all inputs, keep in mind the PWM defines must be on PWM pins
// the default pins listed are the ones used on the Redbot (ROB-12097) with
// the exception of STBY which the Redbot controls with a physical switch
#define AIN1 PA_1
#define BIN1 PA_3
#define AIN2 PA_0
#define BIN2 PA_4
#define PWMA PA_6
#define PWMB PA_7
#define STBY PA_2

// these constants are used to allow you to make your motor configuration 
// line up with function names like forward.  Value can be 1 or -1
const int offsetA = 1;
const int offsetB = 1;

// Initializing motors.  The library will allow you to initialize as many
// motors as you have memory for.  If you are using functions like forward
// that take 2 motors as arguements you can either write new functions or
// call the function more than once.
Motor motor1 = Motor(AIN1, AIN2, PWMA, offsetA, STBY);
Motor motor2 = Motor(BIN1, BIN2, PWMB, offsetB, STBY);

void setup() 
{
  Serial.begin(115200);
  IBus.begin(Serial);
}

void loop() 
{
  InputChannel ch;
  IBus.loop();
  ch.Aileron  = IBus.readChannel(2);
  ch.Elevator = IBus.readChannel(3);
  
  //Serial.print(ch.Aileron);
  //Serial.print("\t");
  //Serial.print(ch.Aileron);
  //Serial.print("\t");

  if(!ch.Aileron && !ch.Elevator)
    return;
    
  motorDrive locDrive = PulseToPWM(ch);
  
  //Serial.print(locDrive.LeftD);
  //Serial.print("\t");
  //Serial.print(locDrive.RightD);
  //Serial.print("\t");
  //Serial.println();
  //delay(500);
}

motorDrive PulseToPWM(InputChannel ch) 
{
  motorDrive Drive;
  ch.Aileron  = ch.Aileron  - pwmMid;
  ch.Elevator = ch.Elevator - pwmMid;
  if(!ch.Aileron && !ch.Elevator)
  {
    Drive.LeftD = Drive.RightD = 0;
    motor1.brake();
    motor2.brake();
    return Drive; 
  }
  Drive.LeftD  = ch.Aileron - ch.Elevator;
  Drive.RightD = ch.Aileron + ch.Elevator;
  float DriveScaler = max((float)1.0,max(abs((float)Drive.LeftD /InputMax),abs((float)Drive.RightD/InputMax)));
  Drive.LeftD = (int16_t)constrain((float)Drive.LeftD/DriveScaler,(-1*InputMax),InputMax);
  Drive.LeftD = map(Drive.LeftD, (-1*InputMax),InputMax, speedMin,speedMax);
  Drive.LeftD = constrain(Drive.LeftD, speedMin, speedMax);
  Drive.RightD = (int16_t)constrain((float)Drive.RightD/DriveScaler,(-1*InputMax),InputMax);
  Drive.RightD = map(Drive.RightD, (-1*InputMax),InputMax, speedMin,speedMax);
  Drive.RightD = constrain(Drive.RightD, speedMin, speedMax);
  motor1.drive(Drive.LeftD);
  motor2.drive(Drive.RightD);
  return Drive;
}
