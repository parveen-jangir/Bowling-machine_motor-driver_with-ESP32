/*pannel button pattern and connected GPIO pins

swSwing_Inc(IO4)    Ball Feedeer(IO2)   R_Swing(IO14)  Speed_Inc(IO33)

                  RST(IO13)           FWD(IO27)

swSwing_Dec(IO15)   Mode(IO12)          L_Swing(IO26)  swSpeed_Dec(IO25)

 */
#include <Arduino.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <BluetoothSerial.h>
// #include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2); // set the LCD address to 0x3F for a 16 chars and 2 line display

// bowl feeder connected pin to esp32
const uint8_t Motor_3 = 18;
// const uint8_t Motor_1;
// const uint8_t Motor_2;
const uint8_t In_1 = 5;
const uint8_t In_2 = 17;

//***************GPIO Layout*************//
const uint8_t swSpeed_Inc = 33;
const uint8_t swSpeed_Dec = 25;
const uint8_t swR_swing = 14;
const uint8_t swL_swing = 26;
const uint8_t swFwd = 27;
const uint8_t swMode = 12;
const uint8_t swRST = 13;
const uint8_t swball_F = 2;
const uint8_t swSwing_Inc = 4;
const uint8_t swSwing_Dec = 15;

// RPM and direction of bowl Feeder
uint8_t ball_F = 255;
bool dir_1;
bool dir_2;

// present dutycycle of all three motors
uint8_t duty_M1;
uint8_t duty_M2;
uint8_t duty_M3;

// These are the states of bowl Feedeer push button
bool ball_FState = HIGH;
bool lastBall_FState = HIGH;
char ball_FMode = 'A';
bool ball_FCMode = false;
unsigned long last_Ftime = 0;
int F_time;

//****** BLUETOOTH CODE **********

String device_name = "ESP32 Pakhandi"; // ESP32_MOTOR_CONTROLLER

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
  while (setup_F)
  {
    if (digitalRead(swSwing_Inc) == HIGH && ball_FMode != 'C')
    {
      buzz(200);
      ball_FMode += 1;
      update_FDisp();
    }
    if (digitalRead(swSwing_Dec) == HIGH && ball_FMode != 'A')
    {
      buzz(200);
      ball_FMode -= 1;
      update_FDisp();
    }

    if (digitalRead(swSpeed_Inc) == HIGH)
    {
      ball_F += 15;
    }
    if (digitalRead(swSpeed_Dec) == HIGH)
    {
      ball_F -= 15;
    }
    if (digitalRead(swball_F) == HIGH)
    {
      buzz(200);
      break;
    }
  }

  if (ball_FMode == 'A')
  {
    ball_FCMode = false;
    dir_1 = 1;
    dir_2 = 0;
    digitalWrite(In_1, dir_1);
    digitalWrite(In_2, dir_2);
  }
  else if (ball_FMode == 'B')
  {
    ball_FCMode = false;
    dir_1 = 0;
    dir_2 = 1;
    digitalWrite(In_1, dir_1);
    digitalWrite(In_2, dir_2);
  }
  else if (ball_FMode == 'C')
  {
    ball_FCMode = true;
  }
}

void setup()
{
  pinMode(Motor_3, OUTPUT);
  pinMode(In_1, OUTPUT);
  pinMode(In_2, OUTPUT);
  pinMode(swSpeed_Dec, INPUT);
  pinMode(swSpeed_Inc, INPUT);
  pinMode(swSwing_Dec, INPUT);
  pinMode(swSwing_Inc, INPUT);
  pinMode(swball_F, INPUT);

  Serial.begin(115200);
  SerialBT.begin(device_name);
  analogWrite(Motor_3, ball_F);
  duty_M3 = ball_F;
  ball_FSetting(false);
  // inilialize LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();

  // starting Display
  lcd.setCursor(4, 0);
  lcd.print("WELCOME");
  lcd.setCursor(4, 1);
  lcd.print("PLAYER!");
  delay(2000);
}

void loop()
{
  delay(200);
  btdata = '\0';
  if (SerialBT.available())
  {
    btdata = SerialBT.read();
    Serial.print(" Bluetooth input :- ");
    Serial.println(btdata);
  }
  ball_FState = digitalRead(swball_F);

  if (ball_FState != lastBall_FState || btdata == 'p')
  {
    lastBall_FState = ball_FState;

    if (ball_FState == HIGH || btdata == 'p')
    {
      buzz(200);
      if (duty_M3 != 0)
      {
        analogWrite(Motor_3, 0);
        duty_M3 = 0;
        Serial.println("H1");
      }
      else
      {
      analogWrite(Motor_3, ball_F);
      duty_M3 = ball_F;
    }
    lastDebounceTime = millis();
  }
}
if ((ball_FState == HIGH && millis() - lastDebounceTime >= 3000) || btdata == 'q')
{
  Serial.println("H2");
  buzz(1000);
  while (digitalRead(swball_F) == HIGH)
  {
    delay(10);
  }
  ball_FSetting(true);
}

if (ball_FCMode && millis() - last_Ftime >= F_time)
{
  Serial.println("H1");
  bool F_flag = NULL;
  if (F_flag == true)
  {
    Serial.println("H3");
    digitalWrite(In_1, HIGH);
    digitalWrite(In_2, LOW);
    F_flag = false;
  }
  else
    digitalWrite(In_1, LOW);
  Serial.println("H4");
  digitalWrite(In_2, HIGH);
  F_flag = true;
}
}