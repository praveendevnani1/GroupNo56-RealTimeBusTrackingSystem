/*
*  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
*
*  You need to get streamId and privateKey at data.sparkfun.com and paste them
*  below. Or just customize this script to talk to other HTTP servers.
*
*/
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <TinyGPS.h>

const char* ssid     = "bhoomi";
const char* password = "123456789";
const char* host = "192.168.43.23";
const char* REG_NO = "MH-01-420";
String BUS_NO = "200";
int n;

const byte proximityIN = D3;
const byte proximityOUT = D7;
int flagIN = 0, flagOUT = 0;
int COUNT = 0;
unsigned long prev_in_millis, prev_out_millis;


TinyGPS gps;

static void smartdelay(unsigned long ms);

void setup() {
  Serial.begin(9600);
  Wire.begin(D1, D2); /* join i2c bus with SDA=D1 and SCL=D2 of NodeMCU */
  
  delay(10);
  
  pinMode(proximityIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(proximityIN), countIn, RISING);
  
  pinMode(proximityOUT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(proximityOUT), countOut, RISING);

  // We start by connecting to a WiFi network
  
  Serial.println();
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

int value = 0;

void loop() {
  smartdelay(5000);
  float flat, flon;
  unsigned long age;
  gps.f_get_position(&flat, &flon, &age);
delay(500);
  Wire.requestFrom(8, 13); // request & read data of size 13 from slave 
  BUS_NO = "";
  while(Wire.available()){
    char c = Wire.read();
    n = c;
    if(n != 255){
      BUS_NO += c;
      Serial.println(BUS_NO);
    }
  }
    
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  // We now create a URI for the request
  String url = "/Bus/put-details.php?";
  url += "Bus_No=";
  url += BUS_NO;
  url += "&Count=";
  url += COUNT;
  url += "&Reg_No=";
  url += urlencode(REG_NO);
  url += "&loc_X=";
  url += flon;
  url += "&loc_Y=";
  url += flat;
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
  "Host: " + host + "\r\n" +
  "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
}

String urlencode(String str)
{
  String encodedString="";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i =0; i < str.length(); i++){
    c=str.charAt(i);
    if (c == ' '){
      encodedString+= '+';
    } else if (isalnum(c)){
      encodedString+=c;
    } else{
      code1=(c & 0xf)+'0';
      if ((c & 0xf) >9){
        code1=(c & 0xf) - 10 + 'A';
      }
      c=(c>>4)&0xf;
      code0=c+'0';
      if (c > 9){
        code0=c - 10 + 'A';
      }
      code2='\0';
      encodedString+='%';
      encodedString+=code0;
      encodedString+=code1;
      //encodedString+=code2;
    }
    yield();
  }
  return encodedString;
  
}

unsigned char h2int(char c)
{
  if (c >= '0' && c <='9'){
    return((unsigned char)c - '0');
  }
  if (c >= 'a' && c <='f'){
    return((unsigned char)c - 'a' + 10);
  }
  if (c >= 'A' && c <='F'){
    return((unsigned char)c - 'A' + 10);
  }
  return(0);
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (Serial.available())
    gps.encode(Serial.read());
  } while (millis() - start < ms);
}



void countIn() {
  if(flagIN == 0 && (millis() - prev_in_millis)>2000){
    flagIN = 1;
    COUNT++;
    Serial.print("Count : ");
    Serial.println(COUNT);
    flagIN = 0;
    prev_in_millis = millis();
  }
}


void countOut() {
  if(flagOUT == 0 && (millis() - prev_out_millis)>2000){
    flagOUT = 1;
    
    if(COUNT>0){
      COUNT--;
    }
    
    Serial.print("Count : ");
    Serial.println(COUNT);
    flagOUT = 0;
    prev_out_millis = millis();
  }
}

