
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <stdio.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>

#define SSID_WIFI_ADDR 0
#define SSID_PWD_ADDR 20
#define SSID_URL_ADDR 40

#define EEPROM_SPACE 80

#define STATE_IDLE 0
#define STATE_DISPLAY_OPTIONS 1
#define STATE_DISPLAY_WIFI_CONFIG 2
#define STATE_SAVE_WIFI_CONFIG 3
#define STATE_CUSTOM_JOB 4
#define STATE_DISPLAY_DEVICE_INFO 5 

volatile int STATE=0;

char WifiSSid[20];
char WifiPass[20];
char Url[40];


Adafruit_BMP085 bmp;

void setup()
{
  int timeOutCounter=0;
  
  Serial.begin(115200);

  Serial.println("Initializing..");
  
  delay(2000);
  
  Wire.begin(4, 5); //4 and 5 are the GPIO pins of the esp8266 D1->CLK D2->DATA
  delay(500);

  if (!bmp.begin())
  {
    Serial.println("could not initialize bmp180 module!");
  }
  else
  {
    Serial.println("bmp180 module intialized.");
  }

  delay(300);

  Serial.println("Press any key to enter setup..");
  delay(5000);

 


  if (Serial.available() && Serial.read())
  {
    STATE=STATE_DISPLAY_OPTIONS;
    return;
  }
  
  
  Serial.println("Getting config..");
  ReadConfig();

  Serial.print("Connecting to ssid ");
  Serial.println(WifiSSid);
  
  WiFi.begin(WifiSSid, WifiPass);
  
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
    timeOutCounter++;
    if (timeOutCounter>20)
    {
      break;  
    }
  }

  if (timeOutCounter>20){
    Serial.println("Could not connect to ssid.");
    Serial.println("Restarting...");
    ESP.restart();
  }else{
    Serial.println("Connected.");
    STATE=STATE_CUSTOM_JOB;
  }
  
}

void loop() {

   switch(STATE)
   {
      case STATE_IDLE:
         Idle();
         break;
      case STATE_DISPLAY_OPTIONS:
         PrintOptions();
         break;
      case STATE_DISPLAY_WIFI_CONFIG:
        DisplayConfig();
        STATE=STATE_DISPLAY_OPTIONS;
        break;
      case STATE_SAVE_WIFI_CONFIG:
        SaveConfig();
        ReadConfig();
        STATE=STATE_DISPLAY_DEVICE_INFO;
        break;
      case STATE_DISPLAY_DEVICE_INFO:
        DisplayDeviceInfo();
        STATE=STATE_DISPLAY_OPTIONS;
        break;
      case STATE_CUSTOM_JOB:
        PostLightLevels(Url);
        break;
   }

  
}


void PostLightLevels(char* url)
{

    const int ADC_PIN=A0;
    
    HTTPClient http;

    int counter=0;
    int adc_value=0;
    float temp=0.0;
  
    String postData="{\"value\":24}";
    Serial.printf("posting data to %s \r\n",url);

    while(true)
    {
      delay(1000);

      adc_value=analogRead(ADC_PIN);
      temp=bmp.readTemperature();;

      Serial.printf("light value is %d. Temperature is %3.1f\r\n",adc_value,temp);
      

      if (WiFi.status() != WL_CONNECTED)
      {
        Serial.println("Wifi has disconnected.Restarting..");
        ESP.restart();
      }

      counter++;

      if (counter>10)
      {
          counter=0;
          http.begin(url);
          delay(1000);
          http.header("Content-Type: application/json");
          int httpCode=http.POST(postData);
          http.end();  
          Serial.printf("posted value and got a response of %d.\r\n",httpCode);
      }
    }
}

  
void Idle()
{
  delay(500);
  Serial.println("Idling..");
}

void PrintOptions()
{
  char input[20];
  Serial.println("Press 1 to reset.");
  Serial.println("Press 2 to read config.");
  Serial.println("Press 3 to save config.");
  Serial.println("Press 4 to get device info.");
  Serial.println("Press 5 to run custom job.");


  ReadSerial(input);
  Serial.println(input);
  switch(input[0])
  {
    case  '1' :
      ESP.restart();
      break;
     case  '2' :
      STATE=STATE_DISPLAY_WIFI_CONFIG;
      break;
     case  '3' :
      STATE=STATE_SAVE_WIFI_CONFIG;
      break;
     case  '4' :
      STATE=STATE_DISPLAY_DEVICE_INFO;
      break;
     case  '5' :
      STATE=STATE_CUSTOM_JOB;
      break;
     default:
      Serial.println("Invalid selection");  
      STATE=STATE_DISPLAY_OPTIONS;
  }
}

void ReadConfig()
{
  EEPROM.begin(EEPROM_SPACE);
  delay(200);
  EEPROM.get(SSID_WIFI_ADDR,WifiSSid);
  EEPROM.get(SSID_PWD_ADDR,WifiPass);
  EEPROM.get(SSID_URL_ADDR,Url);
  EEPROM.end();
}


void DisplayConfig()
{
    Serial.println("Config");
    Serial.println("========================");
    Serial.print("SSID : "); Serial.print(WifiSSid);
    Serial.println("");
    Serial.print("Password : "); Serial.print(WifiPass);
    Serial.println("\r\n");
    Serial.print("URL : "); Serial.print(Url);
    Serial.println("\r\n");
}


void DisplayDeviceInfo()
{
  Serial.printf("index heap size: %u bytes\n", ESP.getFreeHeap());
}

void SaveConfig()
{
  char ssid[20];
  char pass[20];
  char url[40];
  
  Serial.println("Enter the SSID:");
  ReadSerial(ssid);
  Serial.println("Enter the Password:");
  ReadSerial(pass);
  Serial.println("Enter the data url:");
  ReadSerial(url);
   
  EEPROM.begin(EEPROM_SPACE);
  delay(200);

  EEPROM.put(SSID_WIFI_ADDR,ssid);
  EEPROM.put(SSID_PWD_ADDR,pass);
  EEPROM.put(SSID_URL_ADDR,url);
  EEPROM.commit();
  EEPROM.end();

  Serial.println("Config saved.");
}




void ReadSerial(char *buffer)
{
  int counter=0;
  char chr[2];
  chr[1]='\0';
  strcpy(buffer,"");
  while(true)
  {
      if (Serial.available())
      {
        chr[0]=Serial.read();
        Serial.printf(chr);
        if (chr[0]=='\r')
        {
          break;
        }
        if (chr[0]!='\n')
        {
          strcat(buffer,chr);
          counter++;
        }
        if (counter>100)
        {
          break;  
        }
      }
  }
}
