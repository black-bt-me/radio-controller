#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9,8 ); // CE, CSN
const byte address[6] = "00001";

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(108);
  radio.openWritingPipe(address);
  radio.openReadingPipe(1, address);
  radio.stopListening();
}

void loop() {
  const char text[] = "H";
  if (radio.write(&text, sizeof(text))) {
    Serial.println("تم الإرسال");
    
    // انتقل إلى الاستماع للرد
    radio.startListening();
    unsigned long startTime = millis();
    while (millis() - startTime < 200) {
      if (radio.available()) {
        char reply[32] = "";
        radio.read(&reply, sizeof(reply));
        Serial.print("تم الرد: ");
        Serial.println(reply);
        break;
      }
    }
    radio.stopListening();
  } else {
    Serial.println("فشل الإرسال");
  }

  delay(1000);
}