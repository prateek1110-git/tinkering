#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#define SS_PIN D4
#define RST_PIN D3
#define RELAY 3 //connect the relay to number 3 pin
#define BUZZER D0 // connect the buzzer to 2 pin
#define LED1 D1//for blue 
#define LED2 D2//for red
#define ACCESS_DELAY 2000
#define DENIED_DELAY 1000
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.

int count1=0;
int count2=0;


const char *ssid = "VivoT1";
const char *password = "Krish@123";

WiFiClient client;


#define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "4rsuMpzMwnz0XqSlZIKvJ-7KNtQ4zv5gwSYSCDyowFiXYKgtCbs5VvHM3lI_VVgNsBczrVutuAB4HDJn0i51UQ=="
#define INFLUXDB_ORG "7a864e1609899af8"
#define INFLUXDB_BUCKET "Attendance System"


#define TZ_INFO "UTC5.5"


InfluxDBClient dbclient(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);


Point sensor("esp");




// void connectToWiFi() {
//    Serial.println();
//    Serial.println();
//    Serial.print("Connecting to WiFi");
//    Serial.println("...");
//    WiFi.begin(ssid, password);
//    int retries = 0;
//     while ((WiFi.status() != WL_CONNECTED) && (retries < 20)) {
//       retries++;
//       delay(500);
//       Serial.print(".");
//     }
//     delay(1000);
//     if (retries > 20) {
//         Serial.println(F("WiFi connection FAILED"));
//     }
//     if (WiFi.status() == WL_CONNECTED) {
//         Serial.println(F("WiFi connected!"));
//         Serial.println("IP address: ");
//         Serial.println(WiFi.localIP());
//         Serial.println(F("Setup ready"));
//     }
//     else{
//       Serial.println(F("WiFi connection FAILED"));
//     }
    
// }





void setup() {
  Serial.begin(9600); // Initiate a serial communication
 // connectToWiFi();
  SPI.begin(); // Initiate SPI bus
  mfrc522.PCD_Init(); // Initiate MFRC522
  pinMode(RELAY, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  noTone(BUZZER);
  digitalWrite(RELAY, HIGH);


  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
 

  // Check server connection
  // if (dbclient.validateConnection()) {
  //   Serial.print("Connected to InfluxDB: ");
  //   Serial.println(dbclient.getServerUrl());
  // } else {
  //   Serial.print("InfluxDB connection failed: ");
  //   Serial.println(dbclient.getLastErrorMessage());
  // }

  Serial.println("Put your card to the reader for scanning …");
  Serial.println();
}

void loop() {
  sensor.clearFields(); 

  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  // Show UID on serial monitor
  Serial.print("UID tag: ");
  String name,status,USERid;

  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message: ");
  content.toUpperCase();
  if (content.substring(1) == "D0 3E 92 25") { // enter your own card number after copying it from serial monitor
    Serial.println("Authorized access");
    tone(BUZZER, 500);
    tone(LED1,300);
    delay(DENIED_DELAY);
    noTone(BUZZER);
    noTone(LED1);
    Serial.println();
    Serial.println();
    name = "user1";
    USERid = "381122";
    count1++;
    if(count1%2==0){
      status = "OUT";
    }
    else{
      status = "IN";
    }
    delay(500);
    digitalWrite(RELAY, LOW);
    delay(ACCESS_DELAY);
    digitalWrite(RELAY, HIGH);
  }
  else if (content.substring(1) == "06 16 55 D3") { // enter your own card number after copying it from serial monitor
    Serial.println("Authorized access");
    tone(BUZZER, 500);
    tone(LED1,300);
    delay(DENIED_DELAY);
    noTone(BUZZER);
    noTone(LED1);
    Serial.println();
    name = "user2";
    USERid = "802122";
    count1++;
    if(count1%2==0){
      status = "OUT";
    }
    else{
      status = "IN";
    }
    delay(500);
    digitalWrite(RELAY, LOW);
    delay(ACCESS_DELAY);
    digitalWrite(RELAY, HIGH);
  } else {
    Serial.println("Access denied");
    tone(BUZZER, 3000);
    tone(LED2, 3000);
    delay(2000);
    delay(DENIED_DELAY);
    noTone(BUZZER);
    noTone(LED2);
  }

  
    if (dbclient.validateConnection()) {
      Serial.println("Connected to InfluxDB");
    } else {
      Serial.println("InfluxDB connection failed: ");
    }

    
    sensor.addField("name",name);
    sensor.addField("USERid", USERid);
    sensor.addField("status", status);                             // Adding count and signal strength fields


    if (!dbclient.writePoint(sensor)) {
      Serial.print("InfluxDB write failed: ");
      Serial.println(dbclient.getLastErrorMessage());                   // Write the data point to database
    }


}
