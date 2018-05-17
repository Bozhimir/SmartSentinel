#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>

#include <ESP8266WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <Arduino.h>
#include <string.h>
 
#define RST_PIN         D3         // Configurable, see typical pin layout above
#define SS_PIN          D0        // Configurable, see typical pin layout above
 
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

const char* host = "sql7.freesqldatabase.com";
char user[] = "sql7236757";              // MySQL user login username
char password[] = "EHsWrKxJuw";        // MySQL user login password
String content= "";
char contentC[128];
char INSERT_SQL[] = "INSERT INTO sql7236757.LOG(datetime, rfid) VALUES (NOW(), \"%s\")";
char query[128];

// WiFi card example
char ssid[] = "FMI-403";    // your SSID
char pass[] = "";       // your SSID Password

WiFiClient client;           // Use this for WiFi instead of EthernetClient
MySQL_Connection conn((Client *)&client);
 
void setup() {
  Serial.begin(115200);   // Initialize serial communications with the PC
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  // Begin WiFi section
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Connecting to Wi-Fi...");
  }

  Serial.println("Connecting to DB...");
  if (conn.connect(host, 3306, user, password)) {
    Serial.println("Connected to DB!");
    delay(100);
  }
  else
    Serial.println("Connection to DB failed.");
}
 
void loop() {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(50);
    return;
  }
 
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }
 
  // Dump debug info about the card; PICC_HaltA() is automatically called

  
  content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();

  content = content.substring(1);
  content.toCharArray(contentC, 128);
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);

  sprintf(query, INSERT_SQL, contentC);
  cur_mem->execute(query);
  
  Serial.print("Done");
  
  
  delay(10);
}
