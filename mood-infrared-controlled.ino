/*
* Mood infrared controlled
*
* 1 = Super Happy
* 2 = Happy
* 3 = OK
* 4 = Not Happy
* 5 = Angry
* Up = Vote Up
* Down = Vote Down
*/

#include <IRrecv.h>         // lib IRremoteESP8266 by David Conran, Sebastien Warin, Mark Szabo, Ken Shirriff
#include <IRutils.h>        // lib IRremoteESP8266 by David Conran, Sebastien Warin, Mark Szabo, Ken Shirriff
#include <PubSubClient.h>   // by Nick O'Leary
#include <WiFi.h>           // by Arduino

// WiFi
WiFiClient espClient;
const char* ssid = "";
const char* password =  "";

// MQTT
PubSubClient client(espClient);
const char* mqttCommandTopic = "test/mood";
const char* mqttVoteTopic = "test/vote";
const char* mqttServer = "broker.tec2020.fun";
const unsigned int mqttPort = 1883;
const char* mqttClientId = "test1ctrl";
const char* mqttUser = "sammy";
const char* mqttPassword = "1234";

//LED Component
const unsigned int LedPin = 32;
boolean ledState = false;
const unsigned int FlashEveryMillis = 5000;
unsigned long timeLastFlash = 0;

//Infrared Control Component
const unsigned int ReceiverPin = 14;
const unsigned long Mood1 = 0xFD08F7;     //button 1
const unsigned long Mood2 = 0xFD8877;     //button 2
const unsigned long Mood3 = 0xFD48B7;     //button 3
const unsigned long Mood4 = 0xFD28D7;     //button 4
const unsigned long Mood5 = 0xFDA857;     //button 5
const unsigned long VoteUp = 0xFD50AF;    //button up
const unsigned long VoteDown = 0xFD10EF;  //button down
IRrecv irrecv(ReceiverPin);

void setup() {
  Serial.begin(115200);
  Serial.println("setup starting");
  
  pinMode(LedPin, OUTPUT);
  irrecv.enableIRIn();  // Start the receiver
  
  SetupWifi();
  client.setServer(mqttServer, mqttPort);

  Serial.println("setup complete");
}

void loop() {
  if (Serial.available() > 0) {
    SerialCommand();
  }
  ProcessRemoteCommands();
  FlashingLed();
}

void ProcessRemoteCommands() {
    decode_results results;
  
  if (irrecv.decode(&results)) {

    if (!client.connected()) {
      Reconnect();
    }

    client.loop();
      
    switch (results.value) {
      case Mood1:
        Serial.println("Mood 1 - Super Happy");
        client.publish(mqttCommandTopic, "1");
        break;
      case Mood2:
        Serial.println("Mood 2 - Happy");
        client.publish(mqttCommandTopic, "2");
        break;
      case Mood3:
        Serial.println("Mood 3 - OK");
        client.publish(mqttCommandTopic, "3");
        break;
      case Mood4:
        Serial.println("Mood 4 - Not Happy");
        client.publish(mqttCommandTopic, "4");
        break;
      case Mood5:
        Serial.println("Mood 5 - Angry");
        client.publish(mqttCommandTopic, "5");
        break;        
      case VoteUp:
        Serial.println("Vote Up");
        client.publish(mqttVoteTopic, "+");  
        break;    
      case VoteDown:
        Serial.println("Vote Down");
        client.publish(mqttVoteTopic, "-");  
        break;                              
      default:
        serialPrintUint64(results.value, HEX);
        Serial.println("");
        break;
    }
    
    irrecv.resume();  // Receive the next value
  }
}

void FlashingLed() {
  unsigned long now = millis();
  if (now - timeLastFlash >= FlashEveryMillis) {
    timeLastFlash = now;
    ledState = !ledState;
    digitalWrite(LedPin, ledState);
  }
}

void SerialCommand() {
  int command = Serial.parseInt();

  if (command > 0 && command < 6) {
    Serial.println("Serial command to set mood to "+(String)command);
    
    if (!client.connected()) {
      Reconnect();
    }

    client.loop();

    char buf[1];
    dtostrf(command, 1, 0, buf);

    client.publish(mqttCommandTopic, buf);
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

void Reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect(mqttClientId, mqttUser, mqttPassword)) {
      Serial.println("connected");
      Serial.println("subscribing to topic " + (String)mqttCommandTopic);
      client.subscribe(mqttCommandTopic);
      Serial.println("subscribing to topic " + (String)mqttVoteTopic);
      client.subscribe(mqttVoteTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);  // Wait before retrying
    }
  }
}
