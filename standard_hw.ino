#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#define DHTPN 23
#define DHTTYPE DHT11
#define DOOR_PN 22
#include<ESP32Servo.h>  //调用舵机库 
int DOOR_ANG;
Servo servo1;  // 定义舵机对象
DHT dht(DHTPN,DHTTYPE);

//--------------------------------------------------------修改信息------------------------------------------------
//加入了温湿度传感器函数void Temperature_Humidity_Sensor()，详细见66-88行。并没有进行数值返回。未连接华为云，未修改华为云属性。
//--------------------------------------------------------修改信息------------------------------------------------
int temp;      //温度
int humi;      //湿度
int sensor;
/*MQTT连接配置*/
/*-----------------------------------------------------*/
const char* ssid = "OnePlus 11";
const char* password = "p3p2kb4s";                                                //wifi名字与密码
const char* mqttServer = "7c0e16211a.st1.iotda-device.cn-north-4.myhuaweicloud.com";
const int   mqttPort = 1883;                                                      //华为云上mqtt服务器地址与端口（在华为云上查看）
//以下3个参数可以由HMACSHA256算法生成，为硬件通过MQTT协议接入华为云IoT平台的鉴权依据
const char* clientId = "65f4642471d845632a00b855_0001_0_0_2024031709";
const char* mqttUser = "65f4642471d845632a00b855_0001";
const char* mqttPassword = "433061ff16ffee94e2a3c9252ffda28555ebf259775fdecaa1792ea774e1a926";
#define device_id "65f4642471d845632a00b855_0001" 
#define secret "69fb11619357c38bd34540df3a6cc130" 
WiFiClient espClient; //ESP32WiFi模型定义
PubSubClient client(espClient);
const char* topic_properties_report = "$oc/devices/65f4642471d845632a00b855_0001/sys/properties/report";
const char* topic_command = "$oc/devices/65f4642471d845632a00b855_0001/sys/commands/#/";//设备接收命令，65f4642471d845632a00b855_0001是硬件id
//接收到命令后上发的响应topic
char* topic_Commands_Response = "$oc/devices/65f4642471d845632a00b855_0001/sys/commands/response/request_id=";
void MQTT_Init()
{
//WiFi网络连接部分
  WiFi.begin(ssid, password); //开启ESP32的WiFi
  while (WiFi.status() != WL_CONNECTED) { //ESP尝试连接到WiFi网络
    delay(3000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to the WiFi network");
 
 
//MQTT服务器连接部分
  client.setServer(mqttServer, mqttPort); //设置连接到MQTT服务器的参数
 
  client.setKeepAlive (60); //设置心跳时间
 
  while (!client.connected()) { //尝试与MQTT服务器建立连接
    Serial.println("Connecting to MQTT...");
  
    if (client.connect(clientId, mqttUser, mqttPassword )) {
  
      Serial.println("connected");  
  
    } else {
  
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(6000);
  
    }
  }
   client.setCallback(callback); //可以接受任何平台下发的内容
}

//--------------------------------------------------------温湿度传感器函数------------------------------------------------
void Temperature_Humidity_Sensor(){
  delay(2000);
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  humi=humidity;
  temp=temperature;
  if (isnan(humidity) || isnan(temperature)){
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  float hic = dht.computeHeatIndex(temperature, humidity, false);

  Serial.print(F("Humidity: "));
  Serial.print(humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(temperature);
  Serial.print(F("°C "));
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
}
//--------------------------------------------------------温湿度传感器函数------------------------------------------------


void MReport_Sensor_inform()
{
  Temperature_Humidity_Sensor();
  String JSONmessageBuffer;//定义字符串接收序列化好的JSON数据
  //以下将生成好的JSON格式消息格式化输出到字符数组中，便于下面通过PubSubClient库发送到服务器
  StaticJsonDocument<256> doc;

  JsonObject services_0 = doc["services"].createNestedObject();
  services_0["service_id"] = "ZHIMOU";            //“ZHIMOU”为服务id，在华为云上自定义

  JsonObject services_0_properties = services_0.createNestedObject("properties");
  services_0_properties["sensor"] = 57;                    //sensor为华为云上定义的属性，可有多个
  services_0_properties["humi"] = humi; 
  services_0_properties["temp"] = temp; 
  serializeJson(doc, JSONmessageBuffer);

    Serial.println("Sending message to MQTT topic..");
    Serial.println(JSONmessageBuffer);
    
    if(client.publish(topic_properties_report,JSONmessageBuffer.c_str())==true)//使用c_str函数将string转换为char
    {
      Serial.println("Success sending message");
    }else{
      Serial.println("Error sending message");
    }
}
void Servo_act(int door,int ang){
    if(door==1){
    servo1.write(ang); 
    }
}
void callback(char *topic,byte *payload,unsigned int length)
{

  char *pstr = topic; //指向topic字符串，提取request_id用
 
  /*串口打印出收到的平台消息或者命令*/
  Serial.println();
  Serial.println();
  Serial.print("Message arrived [");
  Serial.print(topic);  //将收到消息的topic展示出来
  Serial.print("] ");
  Serial.println();
 
  payload[length] = '\0'; //在收到的内容后面加上字符串结束符
  char strPayload[255] = {0}; 
  strcpy(strPayload, (const char*)payload);
  Serial.println((char *)payload);  //打印出收到的内容
  Serial.println(strPayload);
 
 
  /*request_id解析部分*///后文有详细解释为什么要提取下发命令的request_id
  char arr[100];  //存放request_id
  int flag = 0;
  char *p = arr;
  while(*pstr)  //以'='为标志，提取出request_id
  {
    if(flag) *p ++ = *pstr;
    if(*pstr == '=') flag = 1;
    pstr++;
  }
  *p = '\0';  
  Serial.println(arr);
 
 
  /*将命令响应topic与resquest_id结合起来*/
  char topicRes[200] = {0};
  strcat(topicRes, topic_Commands_Response);
  strcat(topicRes, arr);
  Serial.println(topicRes);

 // Stream& input;

  StaticJsonDocument<256> doc;

  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }


  const char* service_id = doc["service_id"]; // 
  const char* command_name = doc["command_name"]; 
// "door_open-1、car_door_open-2、LED_control-3、window_control-4、fire_alarm_off-5、gas_leak_alarm_off-6、home_model-7" 
//
  switch(command_name[0])
  {
    case 'a':
      int door=doc["paras"]["door"];                              //分析命令为谁，此处命令为a，a命令中有一个参数为ledpower
      int ang =doc["paras"]["angle"];
      Servo_act(door,ang);
      break;
  }
}
//有响应参数的命令的命令响应参数上报
void MQTT_response(char *topic)
{
 String responsed;
 StaticJsonDocument<128> doc;
 JsonObject response = doc.createNestedObject("response");
 //doc["result_code"] = 0;
 //doc["response_name"] = "doorcontrol";
 //doc["paras"]["state"] = state;
 serializeJson(doc, responsed);
 client.publish(topic,responsed.c_str());
 Serial.println(responsed);
}
void setup(){
  Serial.begin(115200);
  MQTT_Init();
  servo1.attach(DOOR_PN);
}
void loop(){
  client.loop();
  MReport_Sensor_inform();                     //上报函数可以写在其他函数里，比如读取传感器值后判断时间间隔大于阈值后上报，再重新计时
  client.setCallback(callback);
  delay(3000);
}
