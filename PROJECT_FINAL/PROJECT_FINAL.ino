#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

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

const char* host = "den1.mysql2.gear.host";
char user[] = "sentinel";              // MySQL user login username
char password[] = "Sc2r0I-~CgE9";        // MySQL user login password

// WiFi card example
char ssid[] = "FMI-403";    // your SSID
char pass[] = "";       // your SSID Password

WiFiClient client;           // Use this for WiFi instead of EthernetClient
MySQL_Connection conn((Client *)&client);

String content= "";
char contentC[128];

char masterCard[13] = "E0 70 06 A8";
bool isLocked = true;

char QUERY_POP[] = "SELECT rfid FROM sentinel.trusted";
char query[128];

char INSERT_SQL[] = "INSERT INTO sentinel.trusted(rfid) VALUES (\"%s\")";
char INSERT_LOG[] = "INSERT INTO sentinel.log(datetime, rfid) VALUES (NOW(), \"%s\")";

// PushingBox scenario DeviceId code and API
String deviceId = "vBECF1260DE1D1A7";
const char* logServer = "api.pushingbox.com";

void ConnectToDB(){
  Serial.begin(115200);
  while (!Serial); // wait for serial port to connect. Needed for Leonardo only

  // Begin WiFi section
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connecting...");
  if (conn.connect(host, 3306, user, password)) {
    Serial.println("Connected!");
    delay(1000);
  }
  else
    Serial.println("Connection failed.");
}

void InitRFID()
{
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

void ReadCard()
{
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    contentC[0] = '\0';
    delay(50);
    return;
  }
 
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    contentC[0] = '\0';
    delay(50);
    return;
  }
 
  // Dump debug info about the card; PICC_HaltA() is automatically called

  contentC[0] = '\0';
  content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     /*Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);*/
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();

  content = content.substring(1);
  content.toCharArray(contentC, 128);
}

bool IsMasterCard(char* card)
{
  return (strcmp(card,masterCard) == 0);
}

bool IsTrustedCard(char* rfid)
{
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  sprintf(query, QUERY_POP, 9000000);
  cur_mem->execute(query);
  column_names *cols = cur_mem->get_columns();
  row_values *row = NULL;
  do {
   row = cur_mem->get_next_row();
   if (row != NULL) {
      int f=0;
      while(row->values[f] != NULL) {
        if(strcmp(rfid, row->values[f]) == 0)
        {
          return true;
        }
        f++;
       }
    }
}while (row != NULL);
return false;
}

void InsertTrustedCard(char* rfid)
{
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);

  sprintf(query, INSERT_SQL, rfid);
  cur_mem->execute(query);
  
  Serial.print("Done");
}

void LogCard(char* rfid)
{
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);

  sprintf(query, INSERT_LOG, rfid);
  cur_mem->execute(query);
  
  Serial.print("Done");
}

void Alarm()
{
  String message = "Someone has broken into your house!";
  Serial.println("- connecting to pushing server: " + String(logServer));
  if (client.connect(logServer, 80)) {
    Serial.println("- succesfully connected");
    
    String postStr = "devid=";
    postStr += String(deviceId);
    postStr += "&message_parameter=";
    postStr += String(message);
    postStr += "\r\n\r\n";
    
    Serial.println("- sending data...");
    
    client.print("POST /pushingbox HTTP/1.1\n");
    client.print("Host: api.pushingbox.com\n");
    client.print("Connection: close\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  }
  Serial.println("- stopping the client");
}

void Lock()
{
  Serial.println("LOCKED");
  isLocked = true;
}

void Unlock()
{
  Serial.println("UNLOCKED");
  isLocked = false;
}

void setup() {
  // put your setup code here, to run once:
  ConnectToDB();
  InitRFID();
}

void loop() {
  // put your main code here, to run repeatedly:
  ReadCard();
  if(strlen(contentC) != 0)
  {
    if(IsMasterCard(contentC))   {
       while(strlen(contentC) == 0 || IsMasterCard(contentC)){
        ReadCard();
       }
       InsertTrustedCard(contentC);
    }
    else{
      LogCard(contentC);
      Serial.println("CARD LOGGED");
      if(IsTrustedCard(contentC)){
         Unlock();
         delay(5000);
         Lock();
      }
      else{
        Alarm();
        Serial.println("ALARM");
      }
     }
  }
  
}
