#include "smart_objects_project.h"
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Temboo.h>
#include "TembooAccount.h"
#include "DHT.h"
#include <LiquidCrystal.h>
//#include <Thread.h>
//#include "ThreadController.h"
#include "TaskScheduler.h"
#include <avr/pgmspace.h>

#define DHTPIN A1
#define DHTTYPE AM2301
#define MOTOR_PIN 9

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;
LiquidCrystal lcd(6, 17, 5, 8, 3, 2);
SpreadSheetData spreadSheetData;
String tmpLine = "";

void cursorBlink() {
    lcd.cursor();
    delay(500);
    lcd.noCursor();
    delay(500);
}

void lcdTerminalPrintLine(String data) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(tmpLine);
    lcd.setCursor(0, 1);
    lcdPrint(data);
    tmpLine = data;
    delay(1000);
}

void lcdPrint(String data) {
    for (unsigned int n = 0; n < data.length(); n++) {
        lcd.write(data.charAt(n));
        delay(50);
    }
}

void lcdTerminalPrint(String data) {
    tmpLine += data;
    lcd.setCursor(0, 1);
    lcd.print(tmpLine);
    delay(1000);
}

void lcdTerminalReset() {
        lcd.clear();
        tmpLine = "";
}
    
void connectToWiFi() {
  int wifiStatus = WL_IDLE_STATUS;

  // Determine if the WiFi Shield is present
  Serial.print("\n\nShield:");
  lcdTerminalPrintLine("Shield:");
  
  if (WiFi.status() == WL_NO_SHIELD) {
    lcdTerminalPrint("FAIL");
    Serial.println("FAIL");
  }

  lcdTerminalPrint("OK");
  Serial.println("OK");

  // Try to connect to the local WiFi network
  while(wifiStatus != WL_CONNECTED) {
    lcdTerminalPrintLine("WiFi:");  
    Serial.print("WiFi:");
    wifiStatus = WiFi.begin(WIFI_SSID, WPA_PASSWORD);

    if (wifiStatus == WL_CONNECTED) {
      lcdTerminalPrint("OK");
      Serial.println("OK");
    } else {
      Serial.println("FAIL, new try.");
      lcdTerminalPrint("FAIL, new try.");
    }
    delay(5000);
  }
}

void uploadToSpreadSheet() {
  TembooChoreo AppendRowChoreo(client);

  // Invoke the Temboo client
  AppendRowChoreo.begin();

  // Set Temboo account credentials
  AppendRowChoreo.setAccountName(TEMBOO_ACCOUNT);
  AppendRowChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  AppendRowChoreo.setAppKey(TEMBOO_APP_KEY);

  // Set Choreo inputs
  String SpreadsheetTitleValue = "arduino_data";
  AppendRowChoreo.addInput("SpreadsheetTitle", SpreadsheetTitleValue);
  String RowDataValue = (String)spreadSheetData.temperature + ", " +
    (String)spreadSheetData.humidity;
  AppendRowChoreo.addInput("RowData", RowDataValue);
  String RefreshTokenValue = "1/cyd9A709YHLo2gXxjNMWDLSk1StjOKqN0Oa0pm9qRcw";
  AppendRowChoreo.addInput("RefreshToken", RefreshTokenValue);
  String ClientSecretValue = "Bq77mQy5MIuusus7WnUwS-5L";
  AppendRowChoreo.addInput("ClientSecret", ClientSecretValue);
  String ClientIDValue ="140856436460-8671963qhae3d3vj4a03hp2n74aiabq0.apps.googleusercontent.com";
  AppendRowChoreo.addInput("ClientID", ClientIDValue);

  // Identify the Choreo to run
  AppendRowChoreo.setChoreo("/Library/Google/Spreadsheets/AppendRow");

  // Run the Choreo; when results are available, print them to serial
  AppendRowChoreo.run();

  while(AppendRowChoreo.available()) {
    char c = AppendRowChoreo.read();
    Serial.print(c);
  }
  lcdTerminalPrintLine("data sent");
  AppendRowChoreo.close();
}

void updateData() {
    spreadSheetData.humidity = dht.readHumidity();
    spreadSheetData.temperature = dht.readTemperature();

    //display data
    lcdTerminalPrintLine("Tem:" + (String)spreadSheetData.temperature); 
    lcdTerminalPrintLine("Hum:" + (String)spreadSheetData.humidity); 

    uploadToSpreadSheet();  
}

void setup() {
  Serial.begin(9600);
  delay(4000);
  while(!Serial);
  dht.begin();
  lcd.begin(16,2);
  
  connectToWiFi();
  lcdTerminalPrintLine("Setup complete.");

  pinMode(MOTOR_PIN, OUTPUT);
 
}

void loop() {
    //analogWrite(MOTOR_PIN, 80);

    updateData();
}
