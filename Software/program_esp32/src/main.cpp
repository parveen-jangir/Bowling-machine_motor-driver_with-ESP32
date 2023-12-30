#include <Arduino.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <BluetoothSerial.h>

String device_name = "ESP32-BT";  //Define name ESP32

LiquidCrystal_I2C lcd(0x3F, 16, 2);  // set the LCD address to 0x3F for a 16 chars and 2 line display

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run make menuconfig to and enable it
#endif

// Check Serial Port Profile
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;


char btdata = '\0';  //Assign data which receive from bluetooth

const int pwmPin1 = 12;  // PWM output pin for GPIO 12
const int pwmPin2 = 13;  // PWM output pin for GPIO 13

const int incButtonPin1 = 33;  // Push button for increasing duty cycle on GPIO 12
const int decButtonPin1 = 4;   // Push button for decreasing duty cycle on GPIO 12

const int incButtonPin2 = 16;  // Push button for increasing duty cycle on GPIO 13
const int decButtonPin2 = 17;  // Push button for decreasing duty cycle on GPIO 13

//........Button for default duty cycle.........//
const int preSetA = 26;  //for button A
const int preSetB = 32;  //for button B
const int preSetC = 25;  //for button C
const int preSetD = 27;  //for button D
const int preSetE = 14;  //for button E

//........contain default duty cycle values.......//
int dutyCycleA1;
int dutyCycleA2;
int dutyCycleB1;
int dutyCycleB2;
int dutyCycleC1;
int dutyCycleC2;
int dutyCycleD1;
int dutyCycleD2;
int dutyCycleE1;
int dutyCycleE2;

//........Define current and last state of button.......//
//for button A
int preSetAState = HIGH;
int lastPreSetAState = HIGH;
bool setupModeA = false;  //define state of setup mode of preset A
//for button B
int preSetBState = HIGH;
int lastPreSetBState = HIGH;
bool setupModeB = false;  //define state of setup mode of preset B
//for button C
int preSetCState = HIGH;
int lastPreSetCState = HIGH;
bool setupModeC = false;  //define state of setup mode of preset C
//for button D
int preSetDState = HIGH;
int lastPreSetDState = HIGH;
bool setupModeD = false;  //define state of setup mode of preset D
//for button E
int preSetEState = HIGH;
int lastPreSetEState = HIGH;
bool setupModeE = false;  //define state of setup mode of preset E

//.........Stating EEPROM.........//
unsigned long lastWriteTime = 0;
const int eepromWriteDelay = 5000;  // Delay in milliseconds

// EEPROM addresses for storing variable values
int eepromAddressDutyCycleA1 = 0;
int eepromAddressDutyCycleA2 = sizeof(int);  // Assuming int size is 2 bytes


int eepromAddressDutyCycleB1 = 2 * sizeof(int);
int eepromAddressDutyCycleB2 = 3 * sizeof(int);  // Assuming int size is 2 bytes

int eepromAddressDutyCycleC1 = 4 * sizeof(int);
int eepromAddressDutyCycleC2 = 5 * sizeof(int);  // Assuming int size is 2 bytes

int eepromAddressDutyCycleD1 = 6 * sizeof(int);
int eepromAddressDutyCycleD2 = 7 * sizeof(int);  // Assuming int size is 2 bytes

int eepromAddressDutyCycleE1 = 8 * sizeof(int);
int eepromAddressDutyCycleE2 = 9 * sizeof(int);  // Assuming int size is 2 bytes

int buzzer = 5;  //define buzzer pin

int dutyCycle1 = 0;  // Initial duty cycle for GPIO 12
int dutyCycle2 = 0;  // Initial duty cycle for GPIO 13

//define motor1 and motor2 for desplay data to LCD
int motor1 = -1;
int motor2 = -1;

//states of increasing and decreasing button of motor 1 and motor 2
int incButton1State = HIGH;
int decButton1State = HIGH;
int incButton2State = HIGH;
int decButton2State = HIGH;

//add debounce to preSet button
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

//defone mode for display bowl type
String mode = "Normal";
String lastMode;



//.............................function for buzzer..................//
void buzz(int time) {
  digitalWrite(buzzer, HIGH);
  delay(time);
  digitalWrite(buzzer, LOW);
}

//.............................Set PWM..............................//
void setPWM() {
  analogWrite(pwmPin1, dutyCycle1);
  analogWrite(pwmPin2, dutyCycle2);
}

//............................Save duty cycle to EEPROM.............//
void saveDutyCycleA() {
  EEPROM.put(eepromAddressDutyCycleA1, dutyCycleA1);
  EEPROM.put(eepromAddressDutyCycleA2, dutyCycleA2);
  EEPROM.commit();  // Store the values in EEPROM
  delay(10);
}
void saveDutyCycleB() {
  EEPROM.put(eepromAddressDutyCycleB1, dutyCycleB1);
  EEPROM.put(eepromAddressDutyCycleB2, dutyCycleB2);
  EEPROM.commit();  // Store the values in EEPROM
  delay(10);
}
void saveDutyCycleC() {
  EEPROM.put(eepromAddressDutyCycleC1, dutyCycleC1);
  EEPROM.put(eepromAddressDutyCycleC2, dutyCycleC2);
  EEPROM.commit();  // Store the values in EEPROM
  delay(10);
}
void saveDutyCycleD() {
  EEPROM.put(eepromAddressDutyCycleD1, dutyCycleD1);
  EEPROM.put(eepromAddressDutyCycleD2, dutyCycleD2);
  EEPROM.commit();  // Store the values in EEPROM
  delay(10);
}
void saveDutyCycleE() {
  EEPROM.put(eepromAddressDutyCycleE1, dutyCycleE1);
  EEPROM.put(eepromAddressDutyCycleE2, dutyCycleE2);
  EEPROM.commit();  // Store the values in EEPROM
  delay(10);
}
//.............................Display data to LCD.................//
void displayData() {
  if (motor1 != dutyCycle1 || motor2 != dutyCycle2) {
    motor1 = dutyCycle1;
    motor2 = dutyCycle2;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("L_M");
    lcd.setCursor(13, 0);
    lcd.print("R_M");
    lcd.setCursor(0, 1);
    lcd.print(motor1);
    lcd.setCursor(13, 1);
    lcd.print(motor2);
    lcd.setCursor(5, 0);
    lcd.print(mode);
    if (SerialBT.connected()) {
      Serial.println("device is connected successfully");
      lcd.setCursor(8, 1);
      lcd.print("CONN");
    } else {
      Serial.println("not connected");
      lcd.setCursor(8, 1);
      lcd.print("NOT_CONN");
    }
  }
}

void adjustDutyCycleA() {
  while (setupModeA) {

    if (digitalRead(incButtonPin1) == HIGH && dutyCycleA1 < 255) {
      dutyCycleA1++;
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
    }

    if (digitalRead(decButtonPin1) == HIGH && dutyCycleA1 > 0) {
      dutyCycleA1--;
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
    }

    if (digitalRead(incButtonPin2) == HIGH && dutyCycleA2 < 255) {
      dutyCycleA2++;
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
    }

    if (digitalRead(decButtonPin2) == HIGH && dutyCycleA2 > 0) {
      dutyCycleA2--;
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
    }

    Serial.print("DutyCycleA1: ");
    Serial.print(dutyCycleA1);
    Serial.print(" | DutyCycleA2: ");
    Serial.println(dutyCycleA2);
    int setupLeft;
    int setupRight;
    if (setupLeft != dutyCycleA1 || setupRight != dutyCycleA2) {
      setupLeft = dutyCycleA1;
      setupRight = dutyCycleA2;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("L_M");
      lcd.setCursor(14, 0);
      lcd.print("R_M");
      lcd.setCursor(0, 1);
      lcd.print(setupLeft);
      lcd.setCursor(13, 1);
      lcd.print(setupRight);
      lcd.setCursor(5, 0);
      lcd.print("SETUP");
      lcd.setCursor(5, 1);
      lcd.print("R SPIN");
    }

    if (digitalRead(preSetA) == HIGH) {

      while (digitalRead(preSetA) == HIGH) {
        delay(10);
      }
      saveDutyCycleA();
      displayData();
      setupModeA = false;
    }
  }
}

void adjustDutyCycleB() {
  while (setupModeB) {

    if (digitalRead(incButtonPin1) == HIGH && dutyCycleB1 < 255) {
      dutyCycleB1++;
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
    }

    if (digitalRead(decButtonPin1) == HIGH && dutyCycleB1 > 0) {
      dutyCycleB1--;
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
    }

    if (digitalRead(incButtonPin2) == HIGH && dutyCycleB2 < 255) {
      dutyCycleB2++;
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
    }

    if (digitalRead(decButtonPin2) == HIGH && dutyCycleB2 > 0) {
      dutyCycleB2--;
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
    }

    Serial.print("DutyCycleB1: ");
    Serial.print(dutyCycleB1);
    Serial.print(" | DutyCycleB2: ");
    Serial.println(dutyCycleB2);
    int setupLeft;
    int setupRight;
    if (setupLeft != dutyCycleB1 || setupRight != dutyCycleB2) {
      setupLeft = dutyCycleB1;
      setupRight = dutyCycleB2;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("L_M");
      lcd.setCursor(14, 0);
      lcd.print("R_M");
      lcd.setCursor(0, 1);
      lcd.print(setupLeft);
      lcd.setCursor(13, 1);
      lcd.print(setupRight);
      lcd.setCursor(5, 0);
      lcd.print("SETUP");
      lcd.setCursor(5, 1);
      lcd.print("FAST");
    }

    if (digitalRead(preSetB) == HIGH) {

      while (digitalRead(preSetB) == HIGH) {
        delay(10);
      }
      saveDutyCycleB();
      displayData();
      setupModeB = false;
    }
  }
}

void adjustDutyCycleC() {
  while (setupModeC) {

    if (digitalRead(incButtonPin1) == HIGH && dutyCycleC1 < 255) {
      dutyCycleC1++;
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
    }

    if (digitalRead(decButtonPin1) == HIGH && dutyCycleC2 > 0) {
      dutyCycleC1--;
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
    }

    if (digitalRead(incButtonPin2) == HIGH && dutyCycleC2 < 255) {
      dutyCycleC2++;
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
    }

    if (digitalRead(decButtonPin2) == HIGH && dutyCycleC2 > 0) {
      dutyCycleC2--;
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
    }

    Serial.print("DutyCycleC1: ");
    Serial.print(dutyCycleC1);
    Serial.print(" | DutyCycleC2: ");
    Serial.println(dutyCycleC2);
    int setupLeft;
    int setupRight;
    if (setupLeft != dutyCycleC1 || setupRight != dutyCycleC2) {
      setupLeft = dutyCycleC1;
      setupRight = dutyCycleC2;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("L_M");
      lcd.setCursor(14, 0);
      lcd.print("R_M");
      lcd.setCursor(0, 1);
      lcd.print(setupLeft);
      lcd.setCursor(13, 1);
      lcd.print(setupRight);
      lcd.setCursor(5, 0);
      lcd.print("SETUP");
      lcd.setCursor(5, 1);
      lcd.print("MEDIUM");
    }

    if (digitalRead(preSetC) == HIGH) {

      while (digitalRead(preSetC) == HIGH) {
        delay(10);
      }
      saveDutyCycleC();
      displayData();
      setupModeC = false;
    }
  }
}

void adjustDutyCycleD() {
  while (setupModeD) {

    if (digitalRead(incButtonPin1) == HIGH && dutyCycleD1 < 255) {
      dutyCycleD1++;
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
    }

    if (digitalRead(decButtonPin1) == HIGH && dutyCycleD1 > 0) {
      dutyCycleD1--;
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
    }

    if (digitalRead(incButtonPin2) == HIGH && dutyCycleD2 < 255) {
      dutyCycleD2++;
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
    }

    if (digitalRead(decButtonPin2) == HIGH && dutyCycleD2 > 0) {
      dutyCycleD2--;
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
    }

    Serial.print("DutyCycleD1: ");
    Serial.print(dutyCycleD1);
    Serial.print(" | DutyCycleD2: ");
    Serial.println(dutyCycleD2);
    int setupLeft;
    int setupRight;
    if (setupLeft != dutyCycleD1 || setupRight != dutyCycleD2) {
      setupLeft = dutyCycleD1;
      setupRight = dutyCycleD2;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("L_M");
      lcd.setCursor(14, 0);
      lcd.print("R_M");
      lcd.setCursor(0, 1);
      lcd.print(setupLeft);
      lcd.setCursor(13, 1);
      lcd.print(setupRight);
      lcd.setCursor(5, 0);
      lcd.print("SETUP");
      lcd.setCursor(5, 1);
      lcd.print("SLOW");
    }

    if (digitalRead(preSetD) == HIGH) {

      while (digitalRead(preSetD) == HIGH) {
        delay(10);
      }
      saveDutyCycleD();
      displayData();
      setupModeD = false;
    }
  }
}

void adjustDutyCycleE() {
  while (setupModeE) {

    if (digitalRead(incButtonPin1) == HIGH && dutyCycleE1 < 255) {
      dutyCycleE1++;
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
    }

    if (digitalRead(decButtonPin1) == HIGH && dutyCycleE1 > 0) {
      dutyCycleE1--;
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
    }

    if (digitalRead(incButtonPin2) == HIGH && dutyCycleE2 < 255) {
      dutyCycleE2++;
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
    }

    if (digitalRead(decButtonPin2) == HIGH && dutyCycleE2 > 0) {
      dutyCycleE2--;
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
    }

    Serial.print("DutyCycleE1: ");
    Serial.print(dutyCycleE1);
    Serial.print(" | DutyCycleE2: ");
    Serial.println(dutyCycleE2);
    int setupLeft;
    int setupRight;
    if (setupLeft != dutyCycleE1 || setupRight != dutyCycleE2) {
      setupLeft = dutyCycleE1;
      setupRight = dutyCycleE2;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("L");
      lcd.setCursor(14, 0);
      lcd.print("R");
      lcd.setCursor(0, 1);
      lcd.print(setupLeft);
      lcd.setCursor(13, 1);
      lcd.print(setupRight);
      lcd.setCursor(5, 0);
      lcd.print("SETUP");
      lcd.setCursor(5, 1);
      lcd.print("L SPIN");
    }

    if (digitalRead(preSetE) == HIGH) {

      while (digitalRead(preSetE) == HIGH) {
        delay(10);
      }
      saveDutyCycleE();
      displayData();
      setupModeE = false;
    }
  }
}


void setup() {
  Serial.begin(115200);  // Initialize serial communication
  SerialBT.begin(device_name);

  //inilialize LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();

  //Define pin of buzzer
  pinMode(buzzer, OUTPUT);

  //define PWM pin
  pinMode(pwmPin1, OUTPUT);
  pinMode(pwmPin2, OUTPUT);

  //define inc. and dec. button of motor 1
  pinMode(incButtonPin1, INPUT_PULLDOWN);
  pinMode(decButtonPin1, INPUT_PULLDOWN);

  //define inc. and dec. button of motor 2
  pinMode(incButtonPin2, INPUT_PULLDOWN);
  pinMode(decButtonPin2, INPUT_PULLDOWN);

  //inilialize PWM of motor 1 and motor 2 with 0%
  analogWrite(pwmPin1, 0);  // Duty cycle of 0%
  analogWrite(pwmPin2, 0);  // Duty cycle of 0%

  //define preset button pin mode
  pinMode(preSetA, INPUT_PULLDOWN);
  pinMode(preSetB, INPUT_PULLDOWN);
  pinMode(preSetC, INPUT_PULLDOWN);
  pinMode(preSetD, INPUT_PULLDOWN);
  pinMode(preSetE, INPUT_PULLDOWN);

  // Read variable values from EEPROM
  EEPROM.begin(512);  // Initialize EEPROM
  EEPROM.get(eepromAddressDutyCycleA1, dutyCycleA1);
  EEPROM.get(eepromAddressDutyCycleA2, dutyCycleA2);
  EEPROM.get(eepromAddressDutyCycleB1, dutyCycleB1);
  EEPROM.get(eepromAddressDutyCycleB2, dutyCycleB2);
  EEPROM.get(eepromAddressDutyCycleC1, dutyCycleC1);
  EEPROM.get(eepromAddressDutyCycleC2, dutyCycleC2);
  EEPROM.get(eepromAddressDutyCycleD1, dutyCycleD1);
  EEPROM.get(eepromAddressDutyCycleD2, dutyCycleD2);
  EEPROM.get(eepromAddressDutyCycleE1, dutyCycleE1);
  EEPROM.get(eepromAddressDutyCycleE2, dutyCycleE2);

  //reset all data
  if (digitalRead(preSetA) == HIGH && digitalRead(preSetB) == HIGH && digitalRead(preSetD) == HIGH && digitalRead(preSetE) == HIGH) {
    dutyCycleA1 = 0;
    dutyCycleA2 = 0;
    dutyCycleB1 = 0;
    dutyCycleB2 = 0;
    dutyCycleC1 = 0;
    dutyCycleC2 = 0;
    dutyCycleD1 = 0;
    dutyCycleD2 = 0;
    dutyCycleE1 = 0;
    dutyCycleE2 = 0;

    dutyCycle1 = 0;
    dutyCycle2 = 0;
    analogWrite(pwmPin1, dutyCycle1);
    analogWrite(pwmPin2, dutyCycle2);

    saveDutyCycleA();
    saveDutyCycleB();
    saveDutyCycleC();
    saveDutyCycleD();
    saveDutyCycleE();

    lcd.clear();
    lcd.print("System Reset ....");

    for (int i = 0; i < 2; i++) {
      delay(1000);
      digitalWrite(buzzer, HIGH);
      delay(400);
      digitalWrite(buzzer, LOW);
    }
    lcd.clear();
    lcd.print("!....Finish....!");
    delay(500);
  }

  //starting Display
  lcd.setCursor(4, 0);
  lcd.print("WELCOME");
  lcd.setCursor(4, 1);
  lcd.print("PLAYER!");
  delay(2000);
}

void loop() {

  btdata = '\0';  //delete data of baluetooth variable in every loop

  if (SerialBT.available()) {
    btdata = SerialBT.read();
    Serial.print(" Bluetooth input :- ");
    Serial.println(btdata);
  }

  //..........................Motor controls button (increse and decresase).............................//

  int incButton1State = digitalRead(incButtonPin1);
  int decButton1State = digitalRead(decButtonPin1);
  int incButton2State = digitalRead(incButtonPin2);
  int decButton2State = digitalRead(decButtonPin2);


  if (((incButton1State == HIGH) || btdata == 'l') && (dutyCycle1 < 251)) {
    delay(50);
    dutyCycle1 += 5;
    analogWrite(pwmPin1, dutyCycle1);
    buzz(120);
  }

  if (((decButton1State == HIGH) || btdata == 'n') && (dutyCycle1 > 4)) {
    delay(50);
    dutyCycle1 -= 5;
    analogWrite(pwmPin1, dutyCycle1);
    buzz(120);
  }

  if (((incButton2State == HIGH) || btdata == 'm') && (dutyCycle2 < 251)) {
    delay(50);
    dutyCycle2 += 5;
    analogWrite(pwmPin2, dutyCycle2);
    buzz(120);
  }

  if (((decButton2State == HIGH) || btdata == 'o') && (dutyCycle2 > 4)) {
    delay(50);
    dutyCycle2 -= 5;
    analogWrite(pwmPin2, dutyCycle2);
    buzz(120);
  }
  //.................Define PreSet buttons (A,B,C,D).....................//
  preSetAState = digitalRead(preSetA);
  preSetBState = digitalRead(preSetB);
  preSetCState = digitalRead(preSetC);
  preSetDState = digitalRead(preSetD);
  preSetEState = digitalRead(preSetE);

  //..........................allocate  preset values........................//
  if ((preSetAState != lastPreSetAState) || btdata == 'd') {
    lastPreSetAState = preSetAState;

    if (preSetAState == HIGH || btdata == 'd') {
      buzz(220);
      dutyCycle1 = dutyCycleA1;
      dutyCycle2 = dutyCycleA2;
      mode = "R_SWING";
      setPWM();
      lastDebounceTime = millis();
    }
  }

  if (preSetBState != lastPreSetBState || btdata == 'b') {
    lastPreSetBState = preSetBState;

    if (preSetBState == HIGH || btdata == 'b') {
      buzz(200);
      dutyCycle1 = dutyCycleB1;
      dutyCycle2 = dutyCycleB2;
      mode = "FAST";
      setPWM();
      lastDebounceTime = millis();
    }
  }

  if (preSetCState != lastPreSetCState || btdata == 'a') {
    lastPreSetCState = preSetCState;

    if (preSetCState == HIGH || btdata == 'a') {
      buzz(200);
      dutyCycle1 = dutyCycleC1;
      dutyCycle2 = dutyCycleC2;
      mode = "MEDIUM";
      setPWM();
      lastDebounceTime = millis();
    }
  }

  if (preSetDState != lastPreSetDState || btdata == 'c') {
    lastPreSetDState = preSetDState;

    if (preSetDState == HIGH || btdata == 'c') {
      buzz(200);
      dutyCycle1 = dutyCycleD1;
      dutyCycle2 = dutyCycleD2;
      mode = "SLOW";
      setPWM();
      lastDebounceTime = millis();
    }
  }

  if (preSetEState != lastPreSetEState || btdata == 'e') {
    lastPreSetEState = preSetEState;

    if (preSetEState == HIGH || btdata == 'e') {
      buzz(200);
      dutyCycle1 = dutyCycleE1;
      dutyCycle2 = dutyCycleE2;
      mode = "L_SWING";
      setPWM();
      lastDebounceTime = millis();
    }
  }


  //.............................Setup PreSets........................//
  if (preSetAState == HIGH && millis() - lastDebounceTime >= 3000) {
    buzz(1000);
    setupModeA = true;
    while (digitalRead(preSetA) == HIGH) {
      delay(10);
    }
    adjustDutyCycleA();
  }

  if (preSetBState == HIGH && millis() - lastDebounceTime >= 3000) {
    buzz(1000);
    setupModeB = true;
    while (digitalRead(preSetB) == HIGH) {
      delay(10);
    }
    adjustDutyCycleB();
  }

  if (preSetCState == HIGH && millis() - lastDebounceTime >= 3000) {
    buzz(1000);
    setupModeC = true;
    while (digitalRead(preSetC) == HIGH) {
      delay(10);
    }
    adjustDutyCycleC();
  }

  if (preSetDState == HIGH && millis() - lastDebounceTime >= 3000) {
    buzz(1000);
    setupModeD = true;
    while (digitalRead(preSetD) == HIGH) {
      delay(10);
    }
    adjustDutyCycleD();
  }

  if (preSetEState == HIGH && millis() - lastDebounceTime >= 3000) {
    buzz(1000);
    setupModeE = true;
    while (digitalRead(preSetE) == HIGH) {
      delay(10);
    }
    adjustDutyCycleE();
  }


  if (SerialBT.connected()) {
    Serial.println("device is connected successfully");
  } else {
    Serial.println("not connected");
  }
  //restart ESP32 when receive 'r'
  if (btdata == 'r') {
    buzz(1200);
    delay(1000);
    ESP.restart();
  }
  
  if (btdata == 'j') {
    buzz(150);
    dutyCycle1 = 0;
    dutyCycle2 = 0;
    setPWM();
  }

  if (btdata == 'i') {
    buzz(150);
    dutyCycle1 += 12;
    dutyCycle2 += 12;
    setPWM();
  }

  if (btdata == 'k') {
    buzz(150);
    dutyCycle1 -= 12;
    dutyCycle2 -= 12;
    setPWM();
  }



  // Print values to Serial Monitor
  Serial.print("Duty Cycle (GPIO 12): ");
  Serial.print(dutyCycle1);
  Serial.print("\t");

  Serial.print("Duty Cycle (GPIO 13): ");
  Serial.println(dutyCycle2);
  displayData();
  delay(10);  // Add a delay for readability

  mode = "CUSTOM";
}