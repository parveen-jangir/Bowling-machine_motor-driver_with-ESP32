/*pannel button pattern and connected GPIO pins

Swing_Inc(IO4)    Ball Feedeer(IO2)   R_Swing(IO14)  Speed_Inc(IO33)

                  RST(IO13)           FWD(IO27)

Swing_Dec(IO15)   Mode(IO12)          L_Swing(IO26)  Speed_Dec(IO25)

 */

#include <Arduino.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <BluetoothSerial.h>

// bowl feeder connected pin to esp32
const uint8_t Motor3 = 18;
const uint8_t In1 = 5;
const uint8_t In2 = 17;

//***************GPIO Layout*************//
const uint8_t Speed_Inc = 33;
const uint8_t Speed_Dec = 25;
const uint8_t R_swing = 14;
const uint8_t L_swing = 26;
const uint8_t Fwd = 27;
const uint8_t Mode = 12;
const uint8_t RST = 13;
const uint8_t ball_F = 2;
const uint8_t Swing_Inc = 4;
const uint8_t Swing_Dec = 15;

// RPM and direction of bowl Feeder
uint8_t motor_F;
bool dir1;
bool dir2;

// These are the states of bowl Feedeer push button
bool ball_FState = HIGH;
bool lastBall_FState = HIGH;
char ball_FMode;
bool ball_FCMode;
unsigned long last_Ftime = 0;
int F_time;

//****** BLUETOOTH CODE **********

String device_name = "ESP32 Sports AMI"; // ESP32_MOTOR_CONTROLLER

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run make menuconfig to and enable it
#endif

BluetoothSerial SerialBT;

char btdata = '0'; // VARIABLE FOR BLUETOOTH DATA INPUT VALUE

// add debounce to preSet button
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

const uint8_t buzzer = 16;

//.............................function for buzzer..................//
void buzz(int time)
{
  digitalWrite(buzzer, HIGH);
  delay(time);
  digitalWrite(buzzer, LOW);
}

//.................Function for Display for ball Feeder..............//
void update_FDisp()
{
  Serial.println(ball_FMode);
}

//.................Function for ball Feeder.........................//
void ball_FSetting(bool setup_F)
{
  // if (setup_F == false)
  // {
  //   digitalWrite(In1, dir1);
  //   digitalWrite(In2, dir2);
  //   return;
  // }
  while (setup_F)
  {
    if (digitalRead(Swing_Inc) == HIGH && ball_FMode != 'C')
    {
      buzz(200);
      ball_FMode += 1;
      update_FDisp();
    }
    if (digitalRead(Swing_Dec) == HIGH && ball_FMode != 'A')
    {
      buzz(200);
      ball_FMode -= 1;
      update_FDisp();
    }

    if (digitalRead(Speed_Inc) == HIGH)
    {
      motor_F += 15;
    }
    if (digitalRead(Speed_Dec) == HIGH)
    {
      motor_F -= 15;
    }
    if(digitalRead(ball_F) == HIGH){
      buzz(200);
      break;
    }
  }
  if (ball_FMode == 'A')
  {
    ball_FCMode = false;
    dir1 = 1;
    dir2 = 0;
    digitalWrite(In1, dir1);
    digitalWrite(In2, dir2);
  }
  else if (ball_FMode == 'B')
  { 
    ball_FCMode = false;
    dir1 = 0;
    dir2 = 1;
    digitalWrite(In1, dir1);
    digitalWrite(In2, dir2);
  }
  else if (ball_FMode == 'C')
  {
    ball_FCMode = true;
  }
}

void setup()
{
  Serial.begin(115200);
  analogWrite(motor_F, 0);
  ball_FSetting(false);
}

void loop()
{
  ball_FState = digitalRead(ball_F);

  if (ball_FState != lastBall_FState || btdata == 'p')
  {
    lastBall_FState = ball_FState;

    if (ball_FState == HIGH || btdata == 'p')
    {
      buzz(200);
      if(analogRead(Motor3) != 0){
        analogWrite(Motor3, 0);
      }
      else
        analogWrite(Motor3, motor_F);
     
      lastDebounceTime = millis();
    }
  }
  if (ball_FState == HIGH && millis() - lastDebounceTime >= 3000)
  {
    buzz(1000);
    while (digitalRead(ball_F) == HIGH)
    {
      delay(10);
    }
    ball_FSetting(true);
  }

  if(ball_FCMode && millis() - last_Ftime >= F_time){

  }
}