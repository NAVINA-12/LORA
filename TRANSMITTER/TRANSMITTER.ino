#include <LoRa.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>  
#include <ArduinoJson.h>

// network credentials
const char* ssid = "NITHISH";
const char* password = "password";

// Initialize Telegram BOT
#define BOTtoken "6866350235:AAEIvVEfYZ_3vDdVPxnxj4LwbhGrrOEDkhI" 
#define SS 15
#define RST 16
#define DIO0 4

#define CHAT_ID "7148724139"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

int pinToSend = 2;

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

const int ledPin = 2;
bool ledState = LOW;

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "/Motor_on to turn GPIO ON \n";
      welcome += "/Motor_off to turn GPIO OFF \n";
      welcome += "/Motor_state to request current GPIO state \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/Motor_on") {
      bot.sendMessage(chat_id, "Motor state set to ON", "");
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
    }
    
    if (text == "/Motor_off") {
      bot.sendMessage(chat_id, "Motor state set to OFF", "");
      ledState = LOW;
      digitalWrite(ledPin, ledState);
    }
    
    if (text == "/Motor_state") {
      if (digitalRead(ledPin)){
        bot.sendMessage(chat_id, "MOTOR is ON", "");
      }
      else{
        bot.sendMessage(chat_id, "MOTOR is OFF", "");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Device Activated");

  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa Error");
    delay(100);
    while (1);
  }
  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      
    client.setTrustAnchors(&cert); 
  #endif

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
}

void loop() {
  if (digitalRead(pinToSend) == HIGH) { 
    Serial.println("Sending Message: HIGH");
    LoRa.beginPacket();
    LoRa.print("HIGH");
    LoRa.endPacket();
    delay(1000);
    
  }
  else if (digitalRead(pinToSend) == LOW) { 
    Serial.println("Sending Message: LOW");
    LoRa.beginPacket();
    LoRa.print("LOW");
    LoRa.endPacket();
    delay(1000);
    
  }
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
