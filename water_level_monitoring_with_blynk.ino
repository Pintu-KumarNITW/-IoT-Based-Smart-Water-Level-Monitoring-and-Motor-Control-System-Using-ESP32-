#define BLYNK_TEMPLATE_ID "TMPL3hIPJgLEw"
#define BLYNK_TEMPLATE_NAME "Water Level Monitoring System"
#define BLYNK_AUTH_TOKEN "-wXTcW14CJUlO5DY8aNTmKQ0Ks2pSqOs"

#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LED_LOW 5    // LED for very low water level
#define LED_HIGH 4   // LED for high water level
#define TRIG 12 
#define ECHO 13
#define RELAY 14
#define BUZZER 18    // Buzzer via BC547 transistor

// Tank max height in cm
int MaxLevel = 12;

// Define high/low threshold levels
int LevelLow = (MaxLevel * 80) / 100;      // 80%
int LevelVeryLow = (MaxLevel * 20) / 100;  // 20%

LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "RSM";          
char pass[] = "hotspotledu";  

bool buzzerAlertedLow = false;
bool buzzerAlertedHigh = false;

// === Function to beep 3 times ===
void beepThreeTimes() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(300);
    digitalWrite(BUZZER, LOW);
    delay(300);
  }
}

// === Ultrasonic reading + logic ===
void ultrasonic() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(4);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long t = pulseIn(ECHO, HIGH);
  int distance = t / 29 / 2; 

  Serial.print("Distance: ");
  Serial.println(distance);

  int blynkDistance = (distance - MaxLevel) * -1;
  if (distance <= MaxLevel)
    Blynk.virtualWrite(V0, blynkDistance);
  else
    Blynk.virtualWrite(V0, 0);

  lcd.setCursor(0, 0);
  lcd.print("WLevel:");

  // === Very Low Water ===
  if (distance >= LevelLow) {
    lcd.setCursor(8, 0);
    lcd.print("Very Low ");
    digitalWrite(LED_LOW, HIGH);
    digitalWrite(LED_HIGH, LOW);

    if (!buzzerAlertedLow) {
      beepThreeTimes();
      buzzerAlertedLow = true;
      buzzerAlertedHigh = false;
    }
  }
  // === High Water ===
  else if (distance <= LevelVeryLow) {
    lcd.setCursor(8, 0);
    lcd.print("High     ");
    digitalWrite(LED_LOW, LOW);
    digitalWrite(LED_HIGH, HIGH);

    if (!buzzerAlertedHigh) {
      beepThreeTimes();
      buzzerAlertedHigh = true;
      buzzerAlertedLow = false;
    }
  }
  // === Normal Water ===
  else {
    lcd.setCursor(8, 0);
    lcd.print("Normal   ");
    digitalWrite(LED_LOW, LOW);
    digitalWrite(LED_HIGH, LOW);
  }
}

// === Motor control from Blynk ===
BLYNK_WRITE(V1) {
  bool RelayState = param.asInt();
  if (RelayState == 1) {
    digitalWrite(RELAY, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Motor: ON ");
  } else {
    digitalWrite(RELAY, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("Motor: OFF");
  }
}

// === setup() ===
void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  lcd.init();
  lcd.backlight();

  pinMode(LED_LOW, OUTPUT);
  pinMode(LED_HIGH, OUTPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(RELAY, HIGH);
  digitalWrite(BUZZER, LOW);

  lcd.setCursor(0, 0);
  lcd.print("System Loading..");
  delay(2000);
  lcd.clear();
}

// === loop() ===
void loop() {
  ultrasonic();
  Blynk.run();
  delay(1000);
}
