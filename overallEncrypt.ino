#include <SPI.h>
#include <Wire.h>
#include <ThingSpeak.h>
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <HTTPClient.h>

// RSA Encryption

int p = 5;
int q = 29;

long int n;

long int phi;

long int e;

long int d;

long int power(long long int x, long long int p) {
  if (p == 0) return 1;
  if (p == 1) return x;
  if (p % 2 == 0) return power(x * x, p / 2);
  return x * power(x * x, (p - 1) / 2);
}

long int encrypt(int m) {
  return (long int)pow(m, e) % n;
}

char decrypt(long int m) {
  return (char)(power(m, d) % n);
}

//

#define OLED_SDA 21
#define OLED_SCL 22

#define TdsSensorPin 35
#define VREF 3.3              // analog reference voltage(Volt) of the ADC
#define SCOUNT  30 

//Adafruit_SH1106 display(21, 22);
WiFiClient client;
const int sensorPin = 34;
char ssid[]="Konosuba";
char password[] = "Kazumaaa" ;
//char ssid[]="ayush-Mi-NoteBook-14";
//char password[] = "Ayush123" ;
unsigned long myChannelNumber = 1834719;
const char *myWriteAPIKey = "Y9FQO0I5SA4WKYQA";
const char *myReadAPIKey = "6SUM6F8C3K2L9KF1";

unsigned long encryptChannel = 1569746;
// unsigned long encryptChannel = 1871121;
const char *encryptWriteAPIKey = "ITA2KOTWMDO2ZP16";
// const char *encryptWriteAPIKey = "6FD0B1X1XVJJW42O";

const int potPin = A0;
float ph;
float Value=0;

int analogBuffer[SCOUNT];     // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;
float averageVoltage = 0;
float tdsValue = 0;
float temperature = 25;
// Temperature Sensor is connected to pin 4
const int TempPin = 4;
// Setup OneWire Instance to communicate with the temperature sensor
OneWire oneWire(TempPin);
// Pass the OneWire Instance to the Dallas Temperature Sensor
DallasTemperature tempSensor(&oneWire); 

String Teamid = "Team-8";
String cnt = "Node-1";
String server = "http://esw-onem2m.iiit.ac.in:443/~/in-cse/in-name/Team-8/Node-1/Data";

void createCI(String& val){
  // add the lines in step 3-6 inside this function
  HTTPClient http;
  http.begin(server);
  
  http.addHeader("X-M2M-Origin", "1uVxsR:qYf8QP");// username:password
  http.addHeader("Content-Type", "application/json;ty=4");

  int code = http.POST("{\"m2m:cin\": {\"cnf\":\"application/json\",\"con\": \"" + String(val) + "\"}}");

  Serial.println(code);
  if (code == -1) {
    Serial.println("UNABLE TO CONNECT TO THE SERVER");
  }
  http.end();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(potPin, INPUT);
  Serial.println("Started");
  WiFi.begin(ssid,password);
  delay(5000);
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(5000);
    Serial.print(".");
    WiFi.begin(ssid,password);
  }
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);

  // RSA Encryption
  n = p * q;
  phi = (p - 1) * (q - 1);
  e = 3;
  d = 75;
}
//M
int getTurbidity()
{
  int tds_sensor_value = analogRead(sensorPin);
  Serial.print("turbidity Sensor Value: ");
  Serial.println(tds_sensor_value);
  int turbidity = map(tds_sensor_value, 0, 1800, 100, 0);
  Serial.print("Turbidity :");
  Serial.println(turbidity);
  return turbidity;
}

float getTDS()
{
   float tdsValue = 0.0;
      int n = 5;
      averageVoltage = 0;
      for(int i = 0; i < n; i++) 
      {
        averageVoltage += analogRead(TdsSensorPin);
        delay(500);
      }
      averageVoltage /= n;
      Serial.print("TDS pin");
      Serial.println(averageVoltage);
      averageVoltage = averageVoltage * (float)VREF / 4096.0;
      
      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0)); 
      float compensationCoefficient = 1.0+0.02*(temperature - 24.0);
      //temperature compensation
      float compensationVoltage=averageVoltage/compensationCoefficient;
//      Serial.print("comp volt : ");
//      Serial.println(compensationVoltage);
      //convert voltage value to tds value
      tdsValue  = (133.42*compensationVoltage*compensationVoltage*compensationVoltage - 252.86*compensationVoltage*compensationVoltage + 861.39*compensationVoltage);
      
      return tdsValue;
}

float getTemperature()
{
    tempSensor.requestTemperatures();
    double temp = tempSensor.getTempCByIndex(0);
    return temp;
}

float getPH()
{
    Value= analogRead(potPin);
    Serial.print("val");
    Serial.println(Value);
    Serial.print(" | ");
      float voltage=Value*(3.4/4095.0);
      Serial.print("PH volt"); 
      Serial.println(voltage);
      ph = (3.6*voltage);
      Serial.print("PH");
      Serial.println(ph);
//    delay(1500);
    return ph;
}

void loop() {
  // put your main code here, to run repeatedly:
  // delay(180000);
delay(2000);
  if(WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(ssid,password);
    while(WiFi.status() != WL_CONNECTED)
    {
      delay(5000);
      Serial.print(".");
      WiFi.begin(ssid,password);
    }
  } 
  float temp = getTemperature();
  int TDS = getTDS();
  int turbidity = getTurbidity();
  float ph = getPH();
  Serial.print("Temperature: ");
  Serial.println(temp);
  Serial.print("TDS: ");
  Serial.println(TDS);
//  Serial.print("Turbidity: ");
//  Serial.println(turbidity);
//  Serial.print("pH: ");
//  Serial.println(ph);
  // ThingSpeak.setField(1, temp);
  // ThingSpeak.setField(2, turbidity);
  // ThingSpeak.setField(3, TDS);
  // ThingSpeak.setField(4, ph);
  String dt = "[" + String((float)((int)(temp * 100)) / 100) + "," + String(turbidity) + "," + String(TDS) + "," + String((float)((int)(ph * 100)) / 100) + "]";
  char str[128];
  dt.toCharArray(str, 128);

  char encrypted[128];
  String encryptedString = "[";

  for(int i = 0; i < dt.length(); i++)
  {
    encrypted[i] = encrypt(str[i]);
    if(i != dt.length() - 1)
    {
      encryptedString.concat((int)encrypted[i]);
      encryptedString.concat(",");
    }
    else
    {
      encryptedString.concat((int)encrypted[i]);
    }
  }
  encryptedString.concat("]");
  // ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  // ThingSpeak.setField(1, encryptedString);
  delay(15000);
  // int x = ThingSpeak.writeFields(encryptChannel, encryptWriteAPIKey);
  int x = ThingSpeak.writeField(encryptChannel, 1, encryptedString, encryptWriteAPIKey);
  Serial.print("Error code: ");
  Serial.println(x);
  Serial.println(encryptedString.length());
  Serial.println(encrypted);
  Serial.println(encryptedString);
   createCI(dt);
}
