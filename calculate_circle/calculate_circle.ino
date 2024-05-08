#include <TinyGPS++.h>
#include <SoftwareSerial.h>


#define RXpin 26
#define TXpin 25
TinyGPSPlus gps;
const int buzzer_pin = 2;
SoftwareSerial ss(RXpin, TXpin);   // RX, TX
double targetLat = 12.345678;      // ละติจูดของพื้นที่ที่คุณต้องการ
double targetLng = 98.765432;      // ลองจิจูดของพื้นที่ที่คุณต้องการ
const double radiusMeters = 1.0;   // รัศมีของพื้นที่วงกลมในเมตร
unsigned long previousMillis = 0;  // เก็บค่า millis() ของรอบก่อนหน้า
const long interval = 5000;        // ระยะเวลาในการตรวจสอบ (5 วินาที)

void setup() {
  pinMode(buzzer_pin, OUTPUT);
  Serial.begin(115200);
  ss.begin(9600);
}

void loop() {
  while (ss.available() > 0) {
    unsigned long currentMillis = millis();  // รับค่า millis() ปัจจุบัน
    // ตรวจสอบว่าเวลาที่ผ่านไปเท่ากับหรือเกินระยะเวลาที่กำหนด
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;  // อัปเดตค่า previousMillis ให้เป็นค่าปัจจุบัน
      if (gps.encode(ss.read())) {
        Serial.println("GPS Alert!!");
        Serial.print("Latitude: ");
        Serial.println(gps.location.lat(), 6);  // แสดงละติจูดที่เป็นทศนิยม
        Serial.print("Longitude: ");
        Serial.println(gps.location.lng(), 6);  // แสดงลองจิจูดที่เป็นทศนิยม
        Serial.println();
        if (gps.location.isValid()) {
          double distance = TinyGPSPlus::distanceBetween(gps.location.lat(), gps.location.lng(), targetLat, targetLng);
          //ถ้า GPS จับตำแหน่งได้
          Serial.print("Latitude: ");
          Serial.println(gps.location.lat(), 6);  // แสดงละติจูดที่เป็นทศนิยม
          Serial.print("Longitude: ");
          Serial.println(gps.location.lng(), 6);  // แสดงลองจิจูดที่เป็นทศนิยม
          Serial.println();                       // เพื่อเว้นบรรทัด
          ////////////
          if (distance > radiusMeters) {
            // GPS ออกจากพื้นที่ที่กำหนด
            digitalWrite(buzzer_pin, LOW);
            tone(buzzer_pin, 500);
            delay(800);
            noTone(buzzer_pin);
            digitalWrite(buzzer_pin, HIGH);
            delay(6000);


            Serial.println("GPS Alert!!");
            Serial.print("Latitude: ");
            Serial.println(gps.location.lat(), 6);  // แสดงละติจูดที่เป็นทศนิยม
            Serial.print("Longitude: ");
            Serial.println(gps.location.lng(), 6);  // แสดงลองจิจูดที่เป็นทศนิยม
            Serial.println();
            // ทำการแจ้งเตือนหรือการกระทำอื่น ๆ ที่คุณต้องการ
          }
        } else {
          Serial.print(F("Unable to detect GPS!!!"));
        }
      }
    }
  }
  // digitalWrite(buzzer_pin, LOW);
  // tone(buzzer_pin, 500);
  // delay(800);
  // noTone(buzzer_pin);
  // // digitalWrite(buzzer_pin, HIGH);
  // // delay(6000);
}
