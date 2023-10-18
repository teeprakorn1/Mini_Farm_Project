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
#define LAMP_PIN 5
#define PUMP_PIN 4

#define DHTTYPE DHT22  

#include <TridentTD_LineNotify.h>
#include "BlynkEdgent.h"
#include <HTTPClient.h>
#include <TimeLib.h>
#include <WiFi.h>
#include <SPI.h>
#include <DHT.h>


int light_status,light_sensor,light_auto,light_manual,light_timer,light_sensor_m,light_sensor_n;
int lamp_status;

int dirt_status,dirt_sensor,dirt_auto,dirt_manual,dirt_timer,dirt_sensor_m,dirt_sensor_n;
int pump_status;

char weekday_set_light,weekday_set_dirt,day_of_week;

int rtc_sec,air_set,httpCode;

int defualt_value = 4095;

int timer_start_light = 0;
int timer_start_dirt = 0;

int timer_stop_light = 0;
int timer_stop_dirt = 0;

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
HTTPClient http;

void setup(){
  Serial.begin(115200);
  wifi_connected();

  pinMode(LAMP_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(DIRTPIN, INPUT);
  pinMode(LIGHTPIN,INPUT);

  dht.begin();
  BlynkEdgent.begin();
  timer.setInterval(1000L, blynk_post);
  LINE.notify("<<<---MINI-FARM-IOT-START--->>>");
}

void loop(){
  BlynkEdgent.run();
  Blynk.run();
  timer.run();

  dirtSensor();
  LightSensor();
  getSheet();
}

BLYNK_CONNECTED(){
  Blynk.sendInternal("rtc", "sync"); 
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
  light_status = IfLight();
  dirt_status = IfDirt();
  
  //DHT Sensor Add Sheet.
  if(air_set%60 == 0){
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    String urls = setSheet("air_humidity", "air_temp", h, t);
    goSheet(urls);
  }

  //Light Sensor Add Sheet.
  if(light_timer == 1 && light_sensor_n == 0){
    if(light_auto == 1 && light_sensor == 1){
      light_sensor = 2;
      light_sensor_m = 0;
    }else if(light_manual == 1 && light_sensor == 3){
      light_sensor = 4;
      light_sensor_m = 0;
    }else{
      light_sensor = 5;
      light_sensor_n =  1;
    }
    String urls = setSheet("light_status_id", "sensor_status_id", light_status, light_sensor);
    goSheet(urls);
  }else if(light_timer == 0 && light_sensor_n == 1){
    light_sensor = 6;
    light_sensor_n = 0;
    String urls = setSheet("light_status_id", "sensor_status_id", light_status, light_sensor);
    goSheet(urls);
  }else if(light_timer == 0 && light_sensor_n == 0){

    if(light_auto==1 && light_timer == 0){
      if(lamp_status==1 && light_sensor_m == 0){
        light_sensor = 1;
        light_sensor_m = 1;
        String urls = setSheet("light_status_id", "sensor_status_id", light_status, light_sensor);
        goSheet(urls);
      }else if(lamp_status==0 && light_sensor_m == 1){
        light_sensor = 2;
        light_sensor_m = 0;
        String urls = setSheet("light_status_id", "sensor_status_id", light_status, light_sensor);
        goSheet(urls);
      }
    }else if(light_auto == 0 && light_sensor == 1){
      light_sensor = 2;
      light_sensor_m = 0;
      String urls = setSheet("light_status_id", "sensor_status_id", light_status, light_sensor);
      goSheet(urls);
    }else{
      if(light_manual == 1 && light_sensor_m == 0){
        light_sensor = 3;
        light_sensor_m = 1;
        String urls = setSheet("light_status_id", "sensor_status_id", light_status, light_sensor);
        goSheet(urls);
      }else if(light_manual == 0 && light_sensor_m == 1){
        light_sensor = 4;
        light_sensor_m = 0;
        String urls = setSheet("light_status_id", "sensor_status_id", light_status, light_sensor);
        goSheet(urls);
      }
    }
  }

  //Dirt Sensor Add Sheet.
  if(dirt_timer == 1 && dirt_sensor_n == 0){
    if(dirt_auto == 1 && dirt_sensor == 1){
      dirt_sensor = 2;
      dirt_sensor_m = 0;
    }else if(dirt_manual == 1 && dirt_sensor == 3){
      dirt_sensor = 4;
      dirt_sensor_m = 0;
    }else{
      dirt_sensor = 5;
      dirt_sensor_n =  1;
    }
    String urls = setSheet("dirt_status_id", "sensor_status_id", dirt_status, dirt_sensor);
    goSheet(urls);
  }else if(dirt_timer == 0 && dirt_sensor_n == 1){
    dirt_sensor = 6;
    dirt_sensor_n = 0;
    String urls = setSheet("dirt_status_id", "sensor_status_id", dirt_status, dirt_sensor);
    goSheet(urls);
  }else if(dirt_timer == 0 && dirt_sensor_n == 0){

    if(dirt_auto==1 && dirt_timer == 0){
      if(pump_status==1 && dirt_sensor_m == 0){
        dirt_sensor = 1;
        dirt_sensor_m = 1;
        String urls = setSheet("dirt_status_id", "sensor_status_id", dirt_status, dirt_sensor);
        goSheet(urls);
      }else if(pump_status==0 && dirt_sensor_m == 1){
        dirt_sensor = 2;
        dirt_sensor_m = 0;
        String urls = setSheet("dirt_status_id", "sensor_status_id", dirt_status, dirt_sensor);
        goSheet(urls);
      }
    }else if(dirt_auto == 0 && dirt_sensor == 1){
      dirt_sensor = 2;
      dirt_sensor_m = 0;
      String urls = setSheet("dirt_status_id", "sensor_status_id", dirt_status, dirt_sensor);
      goSheet(urls);
    }else{
      if(dirt_manual == 1 && dirt_sensor_m == 0){
        dirt_sensor = 3;
        dirt_sensor_m = 1;
        String urls = setSheet("dirt_status_id", "sensor_status_id", dirt_status, dirt_sensor);
        goSheet(urls);
      }else if(dirt_manual == 0 && dirt_sensor_m == 1){
        dirt_sensor = 4;
        dirt_sensor_m = 0;
        String urls = setSheet("dirt_status_id", "sensor_status_id", dirt_status, dirt_sensor);
        goSheet(urls);
      }
    }
  }

}

void blynk_post(){
  air_set = millis()/1000;
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  float dirt_value = CalPercent(analogRead(DIRTPIN), defualt_value);
  float light_value = CalPercent(analogRead(LIGHTPIN), defualt_value);

  if (isnan(h) || isnan(t)|| isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  Blynk.virtualWrite(V2, t);
  Blynk.virtualWrite(V3, dirt_value);
  Blynk.virtualWrite(V4, light_value);

  Blynk.sendInternal("rtc", "sync"); 
}

BLYNK_WRITE(V0) {  
  int value = param.asInt();
  if(value == 1){send_Line();}
}
BLYNK_WRITE(V1){
  TimeInputParam t(param);
  unsigned char week_day;

  if(t.hasStartTime() && t.hasStopTime()){
    timer_start_light = (t.getStartHour() * 60 * 60) + (t.getStartMinute() * 60) + t.getStartSecond();
    timer_stop_light = (t.getStopHour() * 60 * 60) + (t.getStopMinute() * 60) + t.getStopSecond();
                   
    for(int i = 1; i <= 7; i++){
      if(t.isWeekdaySelected(i)){
        week_day |= (0x01 << (i-1));
      }else{
        week_day &= (~(0x01 << (i-1)));
      }
    } 
    weekday_set_light = week_day;
  }else{
    timer_start_light = -1;
    timer_stop_light = -1;
  }
}
BLYNK_WRITE(V5){
  TimeInputParam t(param);
  unsigned char week_day;

  if(t.hasStartTime() && t.hasStopTime()){
    timer_start_dirt = (t.getStartHour() * 60 * 60) + (t.getStartMinute() * 60) + t.getStartSecond();
    timer_stop_dirt = (t.getStopHour() * 60 * 60) + (t.getStopMinute() * 60) + t.getStopSecond();
                   
    for(int i = 1; i <= 7; i++){
      if(t.isWeekdaySelected(i)){
        week_day |= (0x01 << (i-1));
      }else{
        week_day &= (~(0x01 << (i-1)));
      }
    } 
    weekday_set_dirt = week_day;
  }else{
    timer_start_dirt = -1;
    timer_stop_dirt = -1;
  }
}

BLYNK_WRITE(V6){
  int value = param.asInt();
  if(value ==1){
    light_auto = 1;
  }else if(value ==0){
    light_auto = 0;
  }
}

BLYNK_WRITE(V7){
  int value = param.asInt();
  if(value == 1){
    dirt_auto = 1;
  }else if(value == 0){
    dirt_auto = 0;
  }
}

BLYNK_WRITE(V8){
  int value = param.asInt();
  if(value == 1){
    light_manual = 1;
  }else if(value == 0){
    light_manual = 0;
  }
}

BLYNK_WRITE(V9){
  int value = param.asInt();
  if(value ==1){
    dirt_manual = 1;
  }else if(value == 0){
    dirt_manual = 0;
  }
}

BLYNK_WRITE(InternalPinRTC) {
  long DEFAULT_TIME = 1357041600; // Jan 1 2013
  long blynkTime = param.asLong(); 
  
  if(blynkTime >= DEFAULT_TIME){
    setTime(blynkTime);
    day_of_week = weekday();
  
    if( day_of_week == 1){day_of_week = 7;
    }else{
      day_of_week -= 1;
    }
    
    rtc_sec = (hour()*60*60) + (minute()*60) + second();
  }
}


int IfTime(int t_start,int t_stop, int t_w){
  bool time_overflow = 0;
  int i;

  if(t_start != -1 && t_stop != -1){
    if(t_stop < t_start){
      time_overflow = 1;
    }

    if((((time_overflow == 0 && (rtc_sec >= t_start) && (rtc_sec < t_stop)) || (time_overflow  && ((rtc_sec >= t_start) ||
    (rtc_sec < t_stop)))) && (t_w == 0x00 || (t_w & (0x01 << (day_of_week - 1) ))))){i = 1;}else{i= 0;}
  }else{
    i = 0;
  }
  return i;
}

void LightSensor(){
  light_status = IfLight();
  light_timer = IfTime(timer_start_light,timer_stop_light,weekday_set_light);
  int light_timestop;

  float light_value = analogRead(LIGHTPIN);
  Serial.printf("LIGHT , %.2f\n",light_value);
  delay(300);

  if(light_timer == 1){
    digitalWrite(LAMP_PIN,LOW);
    lamp_status = 1;
    light_timestop = 1;
  }else{
    light_timestop = 0;
  }

  if(light_timestop == 0){
    if(light_auto == 1){
      if(light_status==0){//ERROR VALUE
        digitalWrite(LAMP_PIN,HIGH);
        lamp_status = 0;
      }else if(light_status==4){//NO LIGHT VALUE
        digitalWrite(LAMP_PIN,LOW);
        lamp_status = 1;
      }else if(light_status==3){//LOW LIGHT VALUE
        digitalWrite(LAMP_PIN,LOW);
        lamp_status = 1;
      }else if(light_status==2){//MEDIUM LIGHT VALUE
        digitalWrite(LAMP_PIN,HIGH);
        lamp_status = 0;
      }else if(light_status==1){//HIGH LIGHT VALUE
        digitalWrite(LAMP_PIN,HIGH);
        lamp_status = 0;
      }else{
        digitalWrite(LAMP_PIN,HIGH);
        lamp_status = 0;
      }

    }else{
      if(light_manual == 1){
        digitalWrite(LAMP_PIN,LOW);
        lamp_status = 1;
      }else{
        digitalWrite(LAMP_PIN,HIGH);
        lamp_status = 0;
      }
    }
  }
}
void dirtSensor(){
  dirt_status = IfDirt();
  dirt_timer = IfTime(timer_start_dirt,timer_stop_dirt,weekday_set_dirt);
  int dirt_timestop;

  float dirt_value = analogRead(DIRTPIN);
  Serial.printf("%d , %.2f\n",dirt_status,dirt_value);
  delay(300);

  if(dirt_timer == 1){
    digitalWrite(PUMP_PIN,LOW);
    pump_status = 1;
    dirt_timestop = 1;
  }else{
    dirt_timestop = 0;
  }

  if(dirt_timestop == 0){
    if(dirt_auto == 1){
      if(dirt_status=0){//DIRT ERROR
        digitalWrite(PUMP_PIN,HIGH);
        pump_status = 0;
      }else if(dirt_status==5){//DIRT VERY DRY
        digitalWrite(PUMP_PIN,LOW);
        pump_status = 1;
      }else if(dirt_status==6){//DIRT NORMAL DRY
        digitalWrite(PUMP_PIN,LOW);
        pump_status = 1;
      }else if(dirt_status==4){//DIRT NORMAL MOIST
        digitalWrite(PUMP_PIN,HIGH);
        pump_status = 0;
      }else if(dirt_status==3){//DIRT VERY MOIST
        digitalWrite(PUMP_PIN,HIGH);
        pump_status = 0;
      }else if(dirt_status==2){//DIRL NORMAL WET
        digitalWrite(PUMP_PIN,HIGH);
        pump_status = 0;
      }else if(dirt_status==1){//DIRL VERY WET
        digitalWrite(PUMP_PIN,HIGH);
        pump_status = 0;
      }else{//DIRT ERROR
        digitalWrite(PUMP_PIN,HIGH);
        dirt_status = 0;
      }
    }else{
      if(dirt_manual == 1){
        digitalWrite(PUMP_PIN,LOW);
        dirt_status = 1;
      }else{
        digitalWrite(PUMP_PIN,HIGH);
        dirt_status = 0;
      }
    }
  } 
}

void send_Line(){
  float h = dht.readHumidity();
  float t = dht.readTemperature(); 
  float dirt_value = analogRead(DIRTPIN);
  float light_value = analogRead(LIGHTPIN);
  light_status = IfLight();
  dirt_status = IfDirt();

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

float CalPercent(float i, float value) {
  return (i * 100) / value;
}

String setSheet(String p1, String p2, int v1, int v2){
  return (url + "?" + p1 + "=" + v1 + "&" + p2 + "=" + v2);
}

void goSheet(String i){
  http.begin(i.c_str());  
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  httpCode = http.GET();

  if(httpCode == 200 || httpCode == 201) {
    String content = http.getString();
  }else{
    Serial.println("Fail: " + String(httpCode));
    delay(1000);
  }
}

int IfLight(){
  float light_value = analogRead(LIGHTPIN);
  int i;

  if(light_value>4095){//ERROR VALUE
    i = 0;
  }else if(light_value>4094){//NO LIGHT VALUE
    i = 4;
  }else if(light_value>1500){//LOW LIGHT VALUE
    i = 3;
  }else if(light_value>1000){//MEDIUM LIGHT VALUE
    i = 2;
  }else if(light_value>0){//HIGH LIGHT VALUE
    i = 1;
  }else{
    i = 0;
  }
  return i;
}

int IfDirt(){
  float dirt_value = analogRead(DIRTPIN);
  int i;

  if(dirt_value>=4095){//DIRT ERROR
    i = 0;
  }else if(dirt_value>3500){//DIRT VERY DRY
    i = 5;
  }else if(dirt_value>3000){//DIRT NORMAL DRY
    i = 6;
  }else if(dirt_value>2000){//DIRT NORMAL MOIST
    i = 4;
  }else if(dirt_value>1500){//DIRT VERY MOIST
    i = 3;
  }else if(dirt_value>500){//DIRL NORMAL WET
    i = 2;
  }else if(dirt_value>0){//DIRL VERY WET
    i = 1;
  }else{//DIRT ERROR
    i = 0;
  }
  return i;
}