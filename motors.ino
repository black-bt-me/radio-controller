#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// تعريف أطراف المحرك مع L298N
#define MOTOR_PIN1 32  // IN1
#define MOTOR_PIN2 33  // IN2
#define MOTOR_PWM 25   // ENA (PWM)

// NRF24L01 Pins
#define CE_PIN 22
#define CSN_PIN 21

// عنوان الاتصال بين المرسل والمستقبل
const uint64_t pipeIn = 0xE8E8F0F0E1LL;

// هيكل لتخزين البيانات المستلمة
struct Signal {
  byte ch1;
  byte ch2;
  byte ch3;
  byte ch4;
  byte ch5;
  byte ch6;
};

Signal data;

// إنشاء كائن للراديو
RF24 radio(CE_PIN, CSN_PIN);

unsigned long lastRecvTime = 0;

void ResetData() {
  data.ch1 = 127;
  data.ch2 = 127;
  data.ch3 = 0;  // throttle
  data.ch4 = 127;
  data.ch5 = 0;
  data.ch6 = 0;
}

void setup() {
  Serial.begin(115200);

  // إعداد أطراف المحرك
  pinMode(MOTOR_PIN1, OUTPUT);
  pinMode(MOTOR_PIN2, OUTPUT);
  ledcAttachPin(MOTOR_PWM, 0); // PWM على القناة 0
  ledcSetup(0, 1000, 8);        // تردد 1 كHz، 8-bit

  // إعداد الراديو
  radio.begin();
  radio.openReadingPipe(1, pipeIn);
  radio.setChannel(110);
  radio.setAutoAck(true);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.startListening();

  ResetData();
}

void recvData() {
  while (radio.available()) {
    radio.read(&data, sizeof(Signal));
    lastRecvTime = millis();
  }
}

void loop() {
  recvData();

  // إذا لم يتم استقبال بيانات لأكثر من ثانية، إعادة ضبط
  if (millis() - lastRecvTime > 1000) {
    ResetData();
  }

  // مثال: استخدام ch3 كتحكم في السرعة (throttle)
  int speed = map(data.ch3, 0, 255, 0, 255); // سرعة PWM

  // ch1 للتحكم في الاتجاه (يمين/يسار أو أمام/خلف)
  if (data.ch1 < 100) {
    // للخلف
    digitalWrite(MOTOR_PIN1, LOW);
    digitalWrite(MOTOR_PIN2, HIGH);
  } else if (data.ch1 > 150) {
    // للأمام
    digitalWrite(MOTOR_PIN1, HIGH);
    digitalWrite(MOTOR_PIN2, LOW);
  } else {
    // توقف
    digitalWrite(MOTOR_PIN1, LOW);
    digitalWrite(MOTOR_PIN2, LOW);
  }

  // إرسال سرعة المحرك
  ledcWrite(0, speed);
}

