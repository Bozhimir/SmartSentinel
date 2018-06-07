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

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET LED_BUILTIN  //4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define RST_PIN         D3         // Configurable, see typical pin layout above
#define SS_PIN          D0        // Configurable, see typical pin layout above
 
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

const char* host = "den1.mysql2.gear.host";
char user[] = "sentinel";              // MySQL user login username
char password[] = "Sc2r0I-~CgE9";        // MySQL user login password

// WiFi card example
char ssid[] = "AndroidAP";    // your SSID
char pass[] = "aaaaaaaa";       // your SSID Password

WiFiClient client;           // Use this for WiFi instead of EthernetClient
MySQL_Connection conn((Client *)&client);

String content= "";
char contentC[128];

char masterCard[13] = "E0 70 06 A8";
bool isLocked = true;

char QUERY_POP[] = "SELECT rfid FROM sentinel.trusted";
char query[128];

char INSERT_SQL[] = "INSERT INTO sentinel.trusted(rfid) VALUES (\"%s\")";
char DELETE_SQL[] = "DELETE FROM sentinel.trusted WHERE rfid = %s";
char INSERT_LOG[] = "INSERT INTO sentinel.log(datetime, rfid) VALUES (NOW(), \"%s\")";

// PushingBox scenario DeviceId code and API
String deviceId = "vBECF1260DE1D1A7";
const char* logServer = "api.pushingbox.com";

void ConnectToDB(){
  display.clearDisplay();
  display.setTextSize(2.5);
  display.setCursor(0,0);
  display.println("Connecting");
  display.setTextSize(2.5);
  display.display();
  Serial.begin(115200);
  while (!Serial); // wait for serial port to connect. Needed for Leonardo only

  // Begin WiFi section
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  if (conn.connect(host, 3306, user, password)) {
    Serial.println("Connected!");
    delay(1000);
  }
  else
    Serial.println("Connection failed.");
  Lock();
}

void InitRFID()
{
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
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

  display.clearDisplay();
  display.setTextSize(1.5);
  display.setCursor(0,0);
  display.println("Card added");
  display.setTextSize(2.5);
  display.display();
  
}

void RemoveTrustedCard(char* rfid)
{
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);

  sprintf(query, DELETE_SQL, rfid);
  cur_mem->execute(query);
  
  display.clearDisplay();
  display.setTextSize(1.5);
  display.setCursor(0,0);
  display.println("Card removed");
  display.setTextSize(2.5);
  display.display();
}

bool IsCardRegistered(char* rfid)
{

  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  sprintf(query, QUERY_POP, 9000000);
  cur_mem->execute(query);
  column_names *cols = cur_mem->get_columns();
  row_values *row = NULL;
  row = cur_mem->get_next_row();
  if (row != NULL) {
      return true;
    }
  return false;
}

void LogCard(char* rfid)
{
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);

  sprintf(query, INSERT_LOG, rfid);
  cur_mem->execute(query);

  display.clearDisplay();
  display.setTextSize(1.5);
  display.setCursor(0,0);
  display.println("Card logged");
  display.display();
  display.setTextSize(2.5);
}

void Alarm()
{
  String message = "Someone has broken into your house!";
  if (client.connect(logServer, 80)) {
    
    String postStr = "devid=";
    postStr += String(deviceId);
    postStr += "&message_parameter=";
    postStr += String(message);
    postStr += "\r\n\r\n";
    
    Serial.println("- sending notification...");
    
    client.print("POST /pushingbox HTTP/1.1\n");
    client.print("Host: api.pushingbox.com\n");
    client.print("Connection: close\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  }

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("ALARM");
  display.display();
}

void PleaseWait()
{
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(2);
  display.println("Please wait");
  display.display();
}

void Lock()
{
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Locked");
  display.display();
  
  isLocked = true;
}

void Unlock()
{
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Unlocked");
  display.display();
  
  isLocked = false;
}

void InitDisplay()
{
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setTextSize(2.5);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
}
void ReadZodiac(){
  Serial.println(analogRead(A0));
  
}
void setup() {
  // put your setup code here, to run once:

  InitDisplay();
  pinMode(D8, INPUT);
  ConnectToDB();
  InitRFID();
}

void loop() {
  ReadZodiac();
  // put your main code here, to run repeatedly:
  ReadCard();
  if(strlen(contentC) != 0)
  {
    if(IsMasterCard(contentC))   {
       display.clearDisplay();
       display.setCursor(0,0);
       display.println("Add or remove card");
       display.display();
       while(strlen(contentC) == 0 || IsMasterCard(contentC)){
        ReadCard();
       }
       PleaseWait();
       if(IsCardRegistered(contentC))
       {
        RemoveTrustedCard(contentC);
       }
       else
       {
        InsertTrustedCard(contentC);
       }
       
    }
    else{
      PleaseWait();
      LogCard(contentC);
      if(IsTrustedCard(contentC)){
         Unlock();
         delay(5000);
         Lock();
      }
      else{
        Alarm();
        delay(1000);
        Lock();
      }
     }
  }
}
