/************
GeeksTips.com
ESP8266 Arduino Tutorial - Push notification messages example
Arduino code example
www.geekstips.com

- replace the dots with your Wi-fi credentials and
- your deviceId from pushingBox account
*/
#include <ESP8266WiFi.h>

// PushingBox scenario DeviceId code and API
String deviceId = "vBECF1260DE1D1A7";
const char* logServer = "api.pushingbox.com";

const char* ssid = "AndroidAP";
const char* password = "aaaaaaaa";

void setup() {
  Serial.begin(115200);
  // Sending a notification to your mobile phone
  // function takes the message as a parameter
  sendNotification("Hello World from ESP8266!");
}

void sendNotification(String message){

  Serial.println("- connecting to Home Router SID: " + String(ssid));
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("- succesfully connected");
  Serial.println("- starting client");
  
  WiFiClient client;

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
  client.stop();
  Serial.println("- stopping the client");
}

void loop() {}



// navsqkude kudeto ima ******* trqbva da se promeni sus iziskvanoto neshto, primerno wifi ceredenshali


//za pushhbullet i pushbox se logva s gmail
//user:bygones97@gmail.com
//pass:esp8266project
// device id ot pushbox: vBECF1260DE1D1A7
