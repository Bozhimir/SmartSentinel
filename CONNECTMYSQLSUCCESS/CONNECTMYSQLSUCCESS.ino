/*
  MySQL Connector/Arduino Example : connect by wifi

  This example demonstrates how to connect to a MySQL server from an
  Arduino using an Arduino-compatible Wifi shield. Note that "compatible"
  means it must conform to the Ethernet class library or be a derivative
  thereof. See the documentation located in the /docs folder for more
  details.

  INSTRUCTIONS FOR USE

  1) Change the address of the server to the IP address of the MySQL server
  2) Change the user and password to a valid MySQL user and password
  3) Change the SSID and pass to match your WiFi network
  4) Connect a USB cable to your Arduino
  5) Select the correct board and port
  6) Compile and upload the sketch to your Arduino
  7) Once uploaded, open Serial Monitor (use 115200 speed) and observe

  If you do not see messages indicating you have a connection, refer to the
  manual for troubleshooting tips. The most common issues are the server is
  not accessible from the network or the user name and password is incorrect.

  Note: The MAC address can be anything so long as it is unique on your network.

  Created by: Dr. Charles A. Bell
*/
#include <ESP8266WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

byte mac_addr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//IPAddress server_addr(52,29,239,198);  // IP of the MySQL *server* here
//const char* host = "sql7.freesqldatabase.com";
//char user[] = "sql7236757";              // MySQL user login username
//char password[] = "EHsWrKxJuw";        // MySQL user login password

const char* host = "den1.mysql2.gear.host";
char user[] = "sentinel";              // MySQL user login username
char password[] = "Sc2r0I-~CgE9";        // MySQL user login password

char CREATE_SQL[] = "CREATE TABLE sentinel.trusted(rfid VARCHAR(30) NOT NULL)";
char QUERY_POP[] = "SELECT rfid FROM sentinel.log";
char query[128];

// WiFi card example
char ssid[] = "FMI-403";    // your SSID
char pass[] = "";       // your SSID Password

WiFiClient client;           // Use this for WiFi instead of EthernetClient
MySQL_Connection conn((Client *)&client);

void connectToDB(){
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

void readFromDB(){
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
        Serial.print(row->values[f]);
        Serial.print("\n");
        f++;
       }
    }
}while (row != NULL);
}

void setup() {
  connectToDB();
 
conn.close();
}

void loop() {
  

  


    
 
  delay(50);
  // print out info about the connection:

}
