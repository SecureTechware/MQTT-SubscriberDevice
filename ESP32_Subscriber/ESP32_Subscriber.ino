// MCP2515 Connection with ESP32
/*
 * MCP2515  = WEMOS ESP32
 * VCC      =       5V
 * GND      =      GND
 * CS       =       P5
 * SO       =      P19
 * SI       =      P23
 * SCK      =      P18
 * INT      =      P22
 */


//Libraries to include 
#include <WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient
#include <mcp_can.h>  // https://github.com/coryjfowler/MCP_CAN_lib
#include <SPI.h>




#define CS 5  // Define CS PIN
MCP_CAN CAN0(CS);  // Initialize MCP_CAN                             

//WiFi Connection
const char* ssid = "SecureTechware2"; 
const char* password = "$7001123";
//MQTT Server Connection
#define mqtt_server "broker.hivemq.com"
#define owner "ArsalanSaleemSecureTechwareSubscriber"
#define mqtt_user "ArsalanSaleemSecureTechwareSubscriber"
#define mqtt_password "ArsalanSaleemSecureTechware"
#define status_topic "SecureTechware_MCP2515_Status"
#define data_topic "SecureTechware_MCP2515_Data"



WiFiClient espClient;
PubSubClient client(espClient);

void mqttconnect() {   // Loop until reconnected 
  while (!client.connected()) {
    Serial.print("connecting to MQTT Server ...");
    /* connect now */
    if (client.connect( owner, mqtt_user, mqtt_password, status_topic, 2, true, "offline")) {
      client.publish(status_topic, "Subscriber is Connected!", true);
      Serial.println("Subscriber is Connected!");
    } else {
      Serial.print("Failed, status code =");
      Serial.print(client.state());
      Serial.println("Try again in 5sec ...");  // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200); //baud rate

  Serial.println();
  Serial.print("Connecting to WiFi ");
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

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); //Get Subscribed Data

  //send data
    /* if MQTT client was disconnected then try to reconnect again */
    if (!client.connected()) {
           Serial.print("MQTT reconnecting ...");
           mqttconnect();
    } 

  // Initialize MCP2515 running at MCP_8MHZ or MCP_16MHZ with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) Serial.println("MCP2515 Initialized Successfully!");
  else Serial.println("Error Initializing MCP2515...");
  
  CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.

  
  Serial.println("ESP32 --> MCP2515 --> MQTT Initialization. Complete Subscriber Successed.");
  client.subscribe(data_topic);
}
void callback(char *topic, byte *payload, unsigned int length) {
  
    String data;  
    data = String((char *) payload);
 
     
     int index = data.indexOf('#');  //Seprate CAN ID and Frame
    
    int frame_len=(length-(index+1))/3; //Set Frame Length
    String id = data.substring(0,index); 
    String frame=data.substring(index+1,length); //Set CAN Frame
   /*
    Serial.println();
    Serial.println("id");
    Serial.println(id);
    Serial.println();
    Serial.println("frame");
    Serial.println(frame);
    Serial.println();
    */
    int str_len = frame.length(); 
    char char_array[str_len];
    frame.toCharArray(char_array, str_len);
    unsigned long IDSend = (long) strtol(id.c_str(),NULL,16); //Frame ID
   /*
    Serial.println();
    Serial.println("frame_len");
    Serial.println(frame_len);
    Serial.println();
  */
  unsigned char stmp[frame_len]; //CAN Frame
  for (int i=0;i<frame_len;i++) stmp[i]= strtol(&char_array[3*i],NULL,16);
   
      byte sndStat = CAN0.sendMsgBuf(IDSend, 0, frame_len, stmp); //Send Data to MCP2515 Module
      if(sndStat == CAN_OK){
        Serial.println("Message Sent Successfully!");
      } else {
        Serial.println("Error Sending Message...");
      }
}


void loop()
{
   client.loop();
}