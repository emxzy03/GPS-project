/***********************************************************
 *Project   : Gelato alert
************************************************************/
// Token Blynk
#define BLYNK_TEMPLATE_ID "TMPL65THUJaiU"
#define BLYNK_TEMPLATE_NAME "GPS Alert"
#define BLYNK_AUTH_TOKEN "-rhFOQaboAx-uPxgP6D_8weCRZ3gVn-X"
#define RXpin 26 
#define TXpin 25
#define LINE_NOTIFY

#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <WiFi.h>
#include <TridentTD_LineNotify.h>
#include <BlynkSimpleEsp32.h>

//connect wifi
const char* ssid = "Real Ez";
const char* pws = "a0870573599";

//Token Line
const String LINE_TOKEN = "3LVc6orZq7qKcssawRcHCB2ovmOKQhAboRDZvm5tr6g";


//variable 
const int buzzer_pin = 2;
const double targetLat = 13.281193;   // ละติจูดของพื้นที่ที่คุณต้องการ
const double targetLng = 100.924075;  // ลองจิจูดของพื้นที่ที่คุณต้องการ
const double radiusMeters = 65;      // รัศมีของพื้นที่วงกลมในเมตร
bool notify = false;
bool notifyOfInArea = false;
int GPSButton = 1;
// bool status = false;


SoftwareSerial mySerial(TXpin, RXpin);
TinyGPS gps;

void printFloat(double f, int digits = 2);

void setup() {
  pinMode(buzzer_pin, OUTPUT);
  Serial.begin(115200);
  mySerial.begin(9600);
  delay(200);
  digitalWrite(buzzer_pin, HIGH);

  //Connect Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pws);

  // Connect WiFi
  Serial.println("Start...");
  WiFi.begin(ssid, pws);
  Serial.println("Please wait to connect...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  //Line
  LINE.setToken(LINE_TOKEN);


}

void loop() {
  digitalWrite(buzzer_pin, HIGH);
  bool newdata = false;
  unsigned long start = millis();
  while (millis() - start < 5000) {
    if (GPSButton != 0) {
      if (mySerial.available()) {
        char c = mySerial.read();
        if (gps.encode(c)) {
          newdata = true;
          notify = true;
        }
      }
    } else if (GPSButton == 0) {  
      Blynk.virtualWrite(V7, 0);
      Blynk.virtualWrite(V8, 0);
      Blynk.virtualWrite(V4, String("NO GPS"));
    }
  }

  if (newdata) {
    Serial.println("Acquired Data: ");
    Serial.println("-------------------------------------------");
    gpsdata(gps);
    Serial.println("\n-------------------------------------------");
    Serial.println();
  }
  Blynk.run();
  
}
BLYNK_WRITE(V0) {
  int value = param.asInt();
  GPSButton = value;
}

float getDistance(float flat1, float flon1, float flat2, float flon2) {

  // Variables
  float dist_calc = 0;
  float dist_calc2 = 0;
  float diflat = 0;
  float diflon = 0;

  // Calculations
  diflat = radians(flat2 - flat1);
  flat1 = radians(flat1);
  flat2 = radians(flat2);
  diflon = radians((flon2) - (flon1));

  dist_calc = (sin(diflat / 2.0) * sin(diflat / 2.0));
  dist_calc2 = cos(flat1);
  dist_calc2 *= cos(flat2);
  dist_calc2 *= sin(diflon / 2.0);
  dist_calc2 *= sin(diflon / 2.0);
  dist_calc += dist_calc2;

  dist_calc = (2 * atan2(sqrt(dist_calc), sqrt(1.0 - dist_calc)));

  dist_calc *= 6371000.0;  //Converting to meters

  return dist_calc;
}

void gpsdata(TinyGPS& gps) {
  long latitude, longitude;
  float flatitude, flongitude;
  unsigned long date, time, chars, age;
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned short sentences, failed;
  String sms_data;

  // double distance = TinyGPS ::distance_between(flatitude, flongitude, targetLat, targetLng);
  gps.f_get_position(&flatitude, &flongitude);
  double distance = getDistance(flatitude, flongitude, targetLat, targetLng);
  if (distance > radiusMeters) {
    // Buzzer sound
    digitalWrite(buzzer_pin, LOW);
    tone(buzzer_pin, 320);
    delay(1700);
    noTone(buzzer_pin);
    digitalWrite(buzzer_pin, HIGH);
    delay(1200);

    //latitude and longitude in 10^-5deg
    gps.get_position(&latitude, &longitude, &age);
    Serial.print("Lat/Long(float): ");
    Serial.print(latitude);
    Serial.print(", ");
    Serial.print(longitude);
    Serial.print("\n");
    Serial.println("-----------ผู้สูงอายุอยู่นอกระยะ!!---------------");

    //Blynk Text Status
    Blynk.virtualWrite(V4, String("Out of area"));
    // status = true;

    //sms
    sms_data = "ผู้สูงอายุอยู่นอกระยะ!! พิกัด: ";
    sms_data += "http://maps.google.com/maps?q=";
    sms_data += String(flatitude, 6) + "," + String(flongitude, 6);
    if (notify) {
      LINE.notify(sms_data);
      notifyOfInArea = true;  //ถ้าผู้ป่วนออกนอกพื้นที่ การแจ้งเตือนเมื่อผู้ป่วยออกนอกพื้นที่เป็นจริง
    }

  } else {
    gps.get_position(&latitude, &longitude, &age);
    Serial.print("Lat/Long(float): ");
    Serial.print(latitude);
    Serial.print(", ");
    Serial.print(longitude);
    Serial.print("\n");
    /////
    digitalWrite(buzzer_pin, HIGH);
    Serial.print("ผู้สูงอายุอยู่ในระยะ!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

    //Blynk Text Status
    Blynk.virtualWrite(V4, String("Within area"));
    // status = false;

    //sms
    sms_data = "ผู้สูงอายุอยู่ในระยะ!! พิกัด: ";
    sms_data += "http://maps.google.com/maps?q=";
    sms_data += String(flatitude, 6) + "," + String(flongitude, 6);
    if (notify && notifyOfInArea) {  //ถ้าผู้ป่วยออกนอกพื้นที่ และ การแจ้งเตือนออกนอกพื้นที่เป็นจริง
      LINE.notify(sms_data);
      notifyOfInArea = false;  // การแจ้งเตือนออกนอกพื้นที่เป็นเท็จ
    }
  }

  // Blynk
  gps.f_get_position(&flatitude, &flongitude);
  gps.get_position(&latitude, &longitude);
  Blynk.virtualWrite(V7, String(flatitude, 6));
  Blynk.virtualWrite(V8, String(flongitude, 6));


  //latitude and longitude in float
  Serial.print("Lat/Long(float): ");
  printFloat(flatitude, 6);
  Serial.print(", ");
  printFloat(flongitude, 6);
  Serial.print("\n");



  //get statistics
  gps.stats(&chars, &sentences, &failed);
  Serial.print("Stats: characters: ");
  Serial.print(chars);
  Serial.print(" sentences: ");
  Serial.print(sentences);
  Serial.print(" failed checksum: ");
  Serial.print(failed);
}

void printFloat(double number, int digits) {
  // Handle negative numbers
  if (number < 0.0) {
    Serial.print('-');
    number = -number;
  }

  // Round correctly so that print(1.999, 2) prints as "2.00"
  double rounding = 0.5;
  for (uint8_t i = 0; i < digits; ++i)
    rounding /= 10.0;

  number += rounding;

  // Extract the integer part of the number and print it
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;
  Serial.print(int_part);

  // Print the decimal point, but only if there are digits beyond
  if (digits > 0)
    Serial.print(".");

  // Extract digits from the remainder one at a time
  while (digits-- > 0) {
    remainder *= 10.0;
    int toPrint = int(remainder);
    Serial.print(toPrint);
    remainder -= toPrint;
  }
}