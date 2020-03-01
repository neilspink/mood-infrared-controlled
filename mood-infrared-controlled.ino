
// http://www.radupascal.com/2016/04/03/esp8266-arduino-iot-hub
// instructions that make sense, including modification of the PubSubClient.h itself.

#include <IRrecv.h>         // lib IRremoteESP8266 by David Conran, Sebastien Warin, Mark Szabo, Ken Shirriff
#include <IRutils.h>        // lib IRremoteESP8266 by David Conran, Sebastien Warin, Mark Szabo, Ken Shirriff
#include <PubSubClient.h>   // by Nick O'Leary
#include <WiFi.h>           // by Arduino

// WiFi
WiFiClient espClient;
const char* ssid = "";
const char* password =  "";

//SharedAccessSignature sr=tec2020-iot.azure-devices.net%2Fdevices%2Fteam1-barometer&sig=XXXXX

// MQTT
PubSubClient client(espClient);

const char* mqttServer = "tec2020-iot.azure-devices.net";
const unsigned int mqttPort = 8883;

const char* mqttClientId = "team1-device1";

// {iothubhostname}/{device_id}/?api-version=2018-06-30
const char* mqttUser = "tec2020-iot.azure-devices.net/team1-device1";    

// SharedAccessSignature sig={signature-string}&se={expiry}&sr={URL-encoded-resourceURI}
const char* mqttPassword = "";

const char* mqttCommandTopic = "devices/team1-device1/messages/events/";
const char* mqttVoteTopic = "test/vote";


//LED Component
const unsigned int LedPin = 32;
boolean ledState = false;
const unsigned int FlashEveryMillis = 5000;
unsigned long timeLastFlash = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("setup starting");
  
  pinMode(LedPin, OUTPUT);

  SetupWifi();
  client.setServer(mqttServer, mqttPort);

  Serial.println("setup complete");
}

void loop() {

  unsigned long now = millis();
  
  if (now - timeLastFlash >= FlashEveryMillis) {

    Serial.println("about try to connect");
    Serial.print("mqttServer =");
    Serial.println(mqttServer);
    Serial.print("mqttClientId =");
    Serial.println(mqttClientId);
    Serial.print("mqttUser =");
    Serial.println(mqttUser);
    Serial.print("mqttPassword =");
    Serial.println(mqttPassword);    
    
    timeLastFlash = now;
    ledState = !ledState;
    digitalWrite(LedPin, ledState);

    if (client.connect(mqttClientId, mqttUser, mqttPassword)) {
      Serial.println("connected");
      client.publish(mqttCommandTopic, "hello world");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);  // Wait before retrying
    }
    
  }
  
}

void SetupWifi() {
  delay(10);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
