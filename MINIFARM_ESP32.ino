#define WFID "AiyuT"
#define PASSWORD "19652508"
#define LINE_TOKEN "cDXyz51IQ6M8rdOr8SJmTbdqNZRNimj1jdTlKBRV5tA"
String url = "https://script.google.com/macros/s/AKfycbwApU7xXiWK96RPd9GugsHOtBebiS4MJZwtodTbqlVzRPAwsJcJG57-hNBqG1C37YfI/exec";

#define BLYNK_TEMPLATE_ID "TMPL6dehztIRR"
#define BLYNK_TEMPLATE_NAME "LED"
#define BLYNK_FIRMWARE_VERSION        "0.1.0"
#define BLYNK_PRINT Serial
#define APP_DEBUG

#define DHTPIN 14
#define DIRTPIN 32
#define LIGHTPIN 33
#define LED_PIN 4
#define PUMP_PIN 5

#define DHTTYPE DHT22  

#include <TridentTD_LineNotify.h>
#include "BlynkEdgent.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <SPI.h>
#include <DHT.h>

int dirt_status,light_status,led_status,pump_status,dry_status;
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

void setup(){
  Serial.begin(115200);
  wifi_connected();

  pinMode(LED_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(DIRTPIN, INPUT);
  pinMode(LIGHTPIN,INPUT);

  dht.begin();
  BlynkEdgent.begin();
  timer.setInterval(1000L, blynk_post);
}

void loop(){
  BlynkEdgent.run();
  Blynk.run();
  timer.run();

  dirtSensor();
  LightSensor();
  getSheet();
}

void wifi_connected(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(WFID, PASSWORD);
  Serial.println(LINE.getVersion());
  Serial.printf("WiFi connecting to %s\n",  WFID);
  while(WiFi.status() != WL_CONNECTED){ Serial.print(".");delay(400);}
  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());  
  LINE.setToken(LINE_TOKEN); 
}

void getSheet(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  String urls = url + "?air_humidity=" + h + "&air_temp=" + t;

  HTTPClient http;
  http.begin(urls.c_str());  
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  if(httpCode == 200 || httpCode == 201) {
    String content = http.getString();
  }else{
    Serial.println("Fail: " + String(httpCode));
    }delay(2000);
  }

void blynk_post(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  float dirt_value = analogRead(DIRTPIN);
  float light_value = analogRead(LIGHTPIN);

  if (isnan(h) || isnan(t)|| isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Blynk.virtualWrite(V1, h);
  Blynk.virtualWrite(V2, t);
  Blynk.virtualWrite(V3, dirt_value/100);
  Blynk.virtualWrite(V4, light_value/100);
  Blynk.virtualWrite(V5, pump_status);
  Blynk.virtualWrite(V6, led_status);
  Blynk.virtualWrite(V7, dry_status);
}

BLYNK_WRITE(V0) {  
  int value = param.asInt();
  float h = dht.readHumidity();
  float t = dht.readTemperature(); 
  float dirt_value = analogRead(DIRTPIN);
  float light_value = analogRead(LIGHTPIN);

  if(value ==1){
    LINE.notify(">>>>>>>>>>>>>>>>>>>");
    LINE.notify("ความชื้นอากาศ="+String(h,2)+" %");
    LINE.notify("อุณหภูมิอากาศ="+String(t,2)+" C");

    if(light_status==0){//ERROR VALUE
      LINE.notifySticker("ตอนนี้บริเวณกระถางวัดค่าไม่ได้  ",1070,17875);
    }else if(light_status==4){//NO LIGHT VALUE
      LINE.notifySticker("ตอนนี้บริเวณกระถางไม่มีแสงสว่าง",1070,17856);
    }else if(light_status==3){//LOW LIGHT VALUE
      LINE.notifySticker("ตอนนี้บริเวณกระถางมีแสงน้อย   ",6370,11088022);
    }else if(light_status==2){//MEDIUM LIGHT VALUE
      LINE.notifySticker("ตอนนี้บริเวณกระถางมีแสงที่พอดี ",789,10874);
    }else if(light_status==1){//HIGH LIGHT VALUE
      LINE.notifySticker("ตอนนี้บริเวณกระถางมีแสงมาก   ",11539,52114142);
    }

    if(dirt_status==5){
      LINE.notifySticker("ตอนนี้ดินของคุณแห้งมากๆ     ",446,2006);
    }else if(dirt_status==6){
      LINE.notifySticker("ตอนนี้ดินของคุณแห้ง         ",446,2023);
    }else if(dirt_status==4){
      LINE.notifySticker("ตอนนี้ดินของคุณชื้น           ",446,1993);
    }else if(dirt_status==3){
      LINE.notifySticker("ตอนนี้ดินของคุณชื้นมากๆ       ",789,10874);
    }else if(dirt_status==2){
      LINE.notifySticker("ตอนนี้ดินของคุณเปียก         ",789,10893);
    }else if(dirt_status==1){
      LINE.notifySticker("ตอนนี้ดินของคุณเปียกมากๆ     ",789,10892);
    }else if(dirt_status==0){
      LINE.notifySticker("ตอนนี้ดินของคุณ Error       ",446,2007);
    }

    LINE.notify(">>>>>>>>>>>>>>>>>>>");
    Serial.print("Done.Send to Line.\n");
  }
}

void LightSensor(){
  float light_value = analogRead(LIGHTPIN);
  Serial.printf("LIGHT , %.2f\n",light_value);
  delay(300);

  if(light_value>4095){//ERROR VALUE
    digitalWrite(LED_PIN,LOW);
    light_status = 0;
    led_status = 0;
  }else if(light_value>4094){//NO LIGHT VALUE
    digitalWrite(LED_PIN,HIGH);
    light_status = 4;
    led_status = 1;
  }else if(light_value>1500){//LOW LIGHT VALUE
    digitalWrite(LED_PIN,HIGH);
    light_status = 3;
    led_status = 1;
  }else if(light_value>1000){//MEDIUM LIGHT VALUE
    digitalWrite(LED_PIN,LOW);
    light_status = 2;
    led_status = 0;
  }else if(light_value>0){//HIGH LIGHT VALUE
    digitalWrite(LED_PIN,LOW);
    light_status = 1;
    led_status = 0;
  }else{
    digitalWrite(LED_PIN,LOW);
    light_status = 0;
    led_status = 0;
  }
}

void dirtSensor(){
  float dirt_value = analogRead(DIRTPIN);
  Serial.printf("%d , %.2f\n",dirt_status,dirt_value);
  delay(300);

  if(dirt_value>3500){//DIRT VERY DRY
    digitalWrite(PUMP_PIN,HIGH);
    dirt_status = 5;
    pump_status = 1;
    dry_status = 1;
  }else if(dirt_value>3000){//DIRT NORMAL DRY
    digitalWrite(PUMP_PIN,HIGH);
    dirt_status = 6;
    pump_status = 1;
    dry_status = 1;
  }else if(dirt_value>2000){//DIRT NORMAL MOIST
    digitalWrite(PUMP_PIN,LOW);
    dirt_status = 4;
    pump_status = 0;
    dry_status = 0;
    }else if(dirt_value>1500){//DIRT VERY MOIST
    digitalWrite(PUMP_PIN,LOW);
    dirt_status = 3;
    pump_status = 0;
    dry_status = 0;
  }else if(dirt_value>500){//DIRL NORMAL WET
    digitalWrite(PUMP_PIN,LOW);
    dirt_status = 2;
    pump_status = 0;
    dry_status = 0;
  }else if(dirt_value>0){//DIRL VERY WET
    digitalWrite(PUMP_PIN,LOW);
    dirt_status = 1;
    pump_status = 0;
    dry_status = 0;
  }else{//DIRT ERROR
    digitalWrite(PUMP_PIN,LOW);
    dirt_status = 0;
    pump_status = 0;
    dry_status = 0;
  }
}