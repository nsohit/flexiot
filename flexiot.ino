/*
  GENERIC SDK FOR ESP
  Add ESP8266 and PubSubClient libraries into your arduino libraries forlder
  You can find those library files inside the zip file.
  update the send_event function and do_actions function with respect your requirements.
*/

#include <ESP8266WiFi.h>

#include <PubSubClient.h>

#include <ArduinoJson.h>
#define PUB_INTERVAL 30
//#include <MPU6050_tockn.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h> 
unsigned long lastPublish = 0;
const int MPU_addr = 0x68; // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

TinyGPS gps;
SoftwareSerial ss(D3, D4);


const char* ssid = "";
const char* password = "bukasandi";

//Backend credentials
const char* mqtt_server = "";
String DEVICE_SERIAL = "" ; //update the device serial according to the serial given by the consumer portal


const char* EVENT_TOPIC = "generic_brand_805generic_device2/common";
String SUB_TOPIC_STRING = "+/" + DEVICE_SERIAL + "generic_brand_805/generic_device/2/sub";


WiFiClient espClient;
PubSubClient client(espClient);
char msg[300];

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//receiving a message
void callback(char* topic, byte* payload, long length) {
  Serial.print("Message arrived [");
  Serial.print(SUB_TOPIC_STRING);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    msg[i] = (char)payload[i];
  }
  do_actions(msg);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), "rabbit", "rabbit")) {
      Serial.println("connected");
      //subscribe to the topic
      const char* SUB_TOPIC = SUB_TOPIC_STRING.c_str();
      client.subscribe(SUB_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  ss.begin(9600);



}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }
  if (WiFi.status() == WL_CONNECTED && !client.connected()) {
    reconnect();
  }

  client.loop();

  if (millis() - lastPublish >= PUB_INTERVAL * 1000UL) {

    readSensor();
    
    lastPublish = millis();

  }

}

void publish_message(const char* message) {
  client.publish(EVENT_TOPIC, message);
}

//====================================================================================================================================================================
void send_event() {
  //create this function to send some events to the backend. You should create a message format like this
  /* Eg :{
        "mac":"6655591876092787",
        "eventName":"eventOne",
        "state":"none",
        "eventOne":{
          "ev1Value1":30
        }
      }

  */
  //Should call publish_message() function to send events. Pass your message as parameter
  // Eg : publish_message(message);
}
//====================================================================================================================================================================
//====================================================================================================================================================================
void do_actions(const char* message) {
  //Create this function according to your actions. you will receive a message something like this
  /* Eg : {
        "action":"actionOne",
        "param":{
          "ac1Value1":"1110" ,
          "parentMac":"6655591876092787",
          "ac1Value4":"port:03",
          "ac1Value5":"on",
          "mac":"6655591876092787",
          "ac1Value2":"2220",
          "ac1Value3":"567776"
        }
      } */


}
void readSensor() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers
  AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
 
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      // Serial.write(c); // hilangkan koment jika mau melihat data yang dikirim dari modul GPS
      if (gps.encode(c)) // Parsing semua data
        newData = true;
    }
  }

    float flat, flon;
    flat == (TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    flon == (TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    gps.satellites() == (TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
    gps.hdop() == ( TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
 
  
  
  
  String message = "{\"eventName\":\"HIMASIKOM\",\"status\":\"none\",\"line\":" + String (AcX) + ",\"lot\":" + String(flon) + ",\"lat\":" + String(flat) + ",\"MAC\":\"" + String(DEVICE_SERIAL) + "\"}";
  Serial.println(message);
  char* msg = (char*)message.c_str();
  publish_message(msg); // send the event to backend

    
}







//=====================================================================================================================================================================
