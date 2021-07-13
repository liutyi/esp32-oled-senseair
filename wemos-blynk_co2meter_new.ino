#include <Wire.h>
#include "SSD1306Wire.h"

//Senseair Sensor UART pins
#define TXD2 2
#define RXD2 14

// SSD1306 Display i2c pins
#define DPORT 0x3c
#define DSDA 5
#define DSCL 4

SSD1306Wire display(DPORT, DSDA, DSCL);

//Blynk
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// You should get Auth Token in the Blynk App. Go to the Project Settings (nut icon).
BlynkTimer timer;

char auth[] = "Blynk Token";

// Your WiFi credentials. Set password to "" for open networks.
char ssid[] = "Wifi AP";
char pass[] = "pass";
//char host[] = "0.0.0.0";

// Wi-Fi Reconnect settings
int lastConnectionAttempt = millis();
int connectionDelay = 5000; // try to reconnect every 5 seconds

// Senseair
byte CO2req[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};
byte CO2out[] = {0, 0, 0, 0, 0, 0, 0};

void RequestCO2 ()
{
while (!Serial1.available())
{
Serial1.write(CO2req, 7);
delay(50);

}

int timeout = 0;
while (Serial1.available() < 7 )
{
timeout++;
if (timeout > 10)
{
while (Serial1.available())
Serial1.read();
break;
}
delay(50);
}

for (int i = 0; i < 7; i++)
{
CO2out[i] = Serial1.read();
}
}

unsigned long CO2count()
{
int high = CO2out[3];
int low = CO2out[4];
unsigned long val = high * 256 + low;
return val * 1; // S8 = 1. K-30 3% = 3, K-33 ICB = 10
}

void sendSensor()
{
// Get Data from the Sensor
RequestCO2();
unsigned long CO2 = CO2count();
String CO2s = "CO2: " + String(CO2);
// Debug to PC COM Prot
Serial.println(CO2s);
// Display Data on OLED
display.clear();
display.setFont(ArialMT_Plain_10);
display.drawString(0, 0, "Online");
display.drawString(60, 0, ssid);
display.setFont(ArialMT_Plain_24);
display.drawString(10, 20, CO2s);

//display.drawString(0, 52, host);
display.display();
Blynk.virtualWrite(V1, CO2);
}

void offlineSensor()
{
// Get Data from the Sensor
RequestCO2();
unsigned long CO2 = CO2count();
String CO2s = "CO2: " + String(CO2);
// Debug to PC COM Prot
Serial.println(CO2s);
// Display Data on OLED
display.clear();
display.setFont(ArialMT_Plain_10);
display.drawString(0, 0, "Offline mode");
display.setFont(ArialMT_Plain_24);
display.drawString(10, 20, CO2s);
display.display();
}
void setup() {
// PC COM Port
Serial.begin(921600);
// UART to Sensair CO2 Sensor
Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);

// Setup display
display.init();
display.setContrast(255);
display.flipScreenVertically();
display.clear();
display.setFont(ArialMT_Plain_24);
display.setTextAlignment(TEXT_ALIGN_LEFT);
display.drawString(10, 20, "Wait..");
display.display();

WiFi.begin((char*)ssid, (char*)pass);
while (WiFi.status() != WL_CONNECTED) {
delay(2000);
offlineSensor(); //Offline Display
}
// Blynk init
// Blynk.begin(auth, ssid, pass, host, 8080);
Blynk.begin(auth, ssid, pass);

// Set DataSend by blynk timer 4s = 4000L
timer.setInterval(4000L, sendSensor);
}

void loop() {

// check WiFi connection:
if (WiFi.status() != WL_CONNECTED)
{
Serial.println("No connect");
delay(2000);
if (millis() - lastConnectionAttempt >= connectionDelay)
{
lastConnectionAttempt = millis();
Serial.println("Offline Display");
offlineSensor();
if (pass && strlen(pass))
{
Serial.println("connect to wifi..");
WiFi.begin((char*)ssid, (char*)pass);
}
else
{
Serial.println("connect to wifi no pass..");
WiFi.begin((char*)ssid);
}
}
}
else
{
Blynk.run();
timer.run();

}
}
