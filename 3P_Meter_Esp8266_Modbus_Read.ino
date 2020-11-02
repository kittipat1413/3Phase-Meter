#include "ModbusRTU.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//uint8_t read_Volt[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB};
//uint8_t read_Amp[8] = {0x01, 0x03, 0x00, 0x03, 0x00, 0x03, 0xF5, 0xCB};
//uint8_t read_Watt[8] = {0x01, 0x03, 0x00, 0x08, 0x00, 0x03, 0x84, 0x09};
//uint8_t read_Var[8] = {0x01, 0x03, 0x00, 0x0C, 0x00, 0x03, 0xC5, 0xC8};
//uint8_t read_PF[8] = {0x01, 0x03, 0x00, 0x14, 0x00, 0x03, 0x45, 0xCF};

////////////////////////////////////////////////////////////////////////////////
/*WiFi Name & Pass*/
const char* ssid     = "";
const char* password = "";
uint8_t Reconnect_attempt = 0;
WiFiClient espClient;
PubSubClient client(espClient);

////////////////////////////////////////////////////////////////////////////////
/*Netpie*/
const char* mqtt_server = "broker.netpie.io";
const char* Client_ID  = "a983c8d2-2bf7-40fc-8d67-b76305ab6812";
const char* Token   = "pkeu7bbhZHppZaGMCnrPRoSBVQgEh8Lq";
const char* Secret = "l1~DuBhpuxFS#xehvVdVc5zYYCu)h1$B";

////////////////////////////////////////////////////////////////////////////////
char data_buffer[80];
char pub_buffer[512];
ModbusRTU Modbus(Serial, 9600); 
uint8_t *buf;
int timer = 0;
float scaling = (100/5);

////////////////////////////////////////////////////////////////////////////////
uint16_t Va = 0;
uint16_t Vb = 0;
uint16_t Vc = 0;
uint16_t Ia = 0;
uint16_t Ib = 0;
uint16_t Ic = 0;
uint16_t Pa = 0;
uint16_t Pb = 0;
uint16_t Pc = 0;
uint16_t Qa = 0;
uint16_t Qb = 0;
uint16_t Qc = 0; 
uint16_t PFa = 0;
uint16_t PFb = 0;
uint16_t PFc = 0;

////////////////////////////////////////////////////////////////////////////////


void WIFI_Connect()
{
  WiFi.disconnect();
  Serial1.println("Re-connecting...");
  WiFi.begin(ssid, password);
  // Wait for connection
  for (int i = 0; i < 50; i++)
  {
    if ( WiFi.status() != WL_CONNECTED ) {
      Serial1.print ( "." );
      delay ( 200 );
    }
    else{
      Reconnect_attempt = 0;
      Serial1.println("\nWiFi connected");  
      Serial1.println("IP address: ");
      Serial1.println(WiFi.localIP());
      return;
    }
  }
}

void netpie_connect() {
  uint8_t reset_cnt = 0;
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial1.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(Client_ID,Token,Secret)) {
      reset_cnt = 0;
      Serial1.println("Netpie-Connected");
    } 
    else {
      if (WiFi.status() != WL_CONNECTED){
        Serial1.println("failed, WiFi-disconnected");
        return;
      }
      Serial1.print("failed, rc=");
      Serial1.print(client.state());
      Serial1.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      reset_cnt++;
         if(reset_cnt >= 36){
         ESP.reset();
        }
    }
      delay(100);
  }   
}

bool read_meter() {
  int res_size;
  
  ////////////////////////////////////////////////////////////////////////////////////////////
  Serial1.println("Read Voltage");
  res_size = Modbus.holdingRegisterRead(1,0x00,0x03,&buf);
  if(res_size > 0){
    Serial1.println("Data OK!!");
    Serial1.print("Response size : ");
    Serial1.println(res_size);
    Serial1.print("Response data : ");
    for(int i = 0; i < res_size; i++){
            Serial1.print(buf[i],HEX);
            Serial1.print("||");            
        }
    Serial1.println("");      
    Va = (((uint16_t)buf[0])<<8 | (uint16_t)buf[1] );
    Vb = (((uint16_t)buf[2])<<8 | (uint16_t)buf[3] );
    Vc = (((uint16_t)buf[4])<<8 | (uint16_t)buf[5] );
    sprintf(data_buffer, "Va : %.2f , Vb : %.2f, Vc : %.2f ",Va/10.0,Vb/10.0,Vc/10.0);
    Serial1.println(data_buffer);  
    }
  else{
    Serial1.println("Read Voltage Error!!");
    return false;
    }
  delay(200);
  
  ////////////////////////////////////////////////////////////////////////////////////////////
  Serial1.println("Read Current");
  res_size = Modbus.holdingRegisterRead(1,0x03,0x03,&buf);
  if(res_size > 0){
    Serial1.println("Data OK!!");
    Serial1.print("Response size : ");
    Serial1.println(res_size);
    Serial1.print("Response data : ");
    for(int i = 0; i < res_size; i++){
            Serial1.print(buf[i],HEX);
            Serial1.print("||");            
        }
    Serial1.println("");      
    Ia = (((uint16_t)buf[0])<<8 | (uint16_t)buf[1] );
    Ib = (((uint16_t)buf[2])<<8 | (uint16_t)buf[3] );
    Ic = (((uint16_t)buf[4])<<8 | (uint16_t)buf[5] );
    sprintf(data_buffer, "Ia : %.2f , Ib : %.2f, Ic : %.2f ",(Ia*scaling)/1000.0,(Ib*scaling)/1000.0,(Ic*scaling)/1000.0);
    Serial1.println(data_buffer);  
    }
  else{
    Serial1.println("Read Current Error!!");
    return false;
    }
  delay(200);

  ////////////////////////////////////////////////////////////////////////////////////////////
  Serial1.println("Read Watt");
  res_size = Modbus.holdingRegisterRead(1,0x08,0x03,&buf);
  if(res_size > 0){
    Serial1.println("Data OK!!");
    Serial1.print("Response size : ");
    Serial1.println(res_size);
    Serial1.print("Response data : ");
    for(int i = 0; i < res_size; i++){
            Serial1.print(buf[i],HEX);
            Serial1.print("||");            
        }
    Serial1.println("");      
    Pa = (((uint16_t)buf[0])<<8 | (uint16_t)buf[1] );
    Pb = (((uint16_t)buf[2])<<8 | (uint16_t)buf[3] );
    Pc = (((uint16_t)buf[4])<<8 | (uint16_t)buf[5] );
    sprintf(data_buffer, "Pa : %.3f , Pb : %.3f, Pc : %.3f ",(Pa*scaling)/1000.0,(Pb*scaling)/1000.0,(Pc*scaling)/1000.0);
    Serial1.println(data_buffer);  
    }
  else{
    Serial1.println("Read Watt Error!!");
    return false;
    }
  delay(200);

  ////////////////////////////////////////////////////////////////////////////////////////////
  Serial1.println("Read Var");
  res_size = Modbus.holdingRegisterRead(1,0x0C,0x03,&buf);
  if(res_size > 0){
    Serial1.println("Data OK!!");
    Serial1.print("Response size : ");
    Serial1.println(res_size);
    Serial1.print("Response data : ");
    for(int i = 0; i < res_size; i++){
            Serial1.print(buf[i],HEX);
            Serial1.print("||");            
        }
    Serial1.println("");      
    Qa = (((uint16_t)buf[0])<<8 | (uint16_t)buf[1] );
    Qb = (((uint16_t)buf[2])<<8 | (uint16_t)buf[3] );
    Qc = (((uint16_t)buf[4])<<8 | (uint16_t)buf[5] );
    sprintf(data_buffer, "Qa : %.3f , Qb : %.3f, Qc : %.3f ",(Qa*scaling)/1000.0,(Qb*scaling)/1000.0,(Qc*scaling)/1000.0);
    Serial1.println(data_buffer);  
    }
  else{
    Serial1.println("Read Var Error!!");
    return false;
    }
  delay(200);
  
  ////////////////////////////////////////////////////////////////////////////////////////////
  Serial1.println("Read PF");
  res_size = Modbus.holdingRegisterRead(1,0x14,0x03,&buf);
  if(res_size > 0){
    Serial1.println("Data OK!!");
    Serial1.print("Response size : ");
    Serial1.println(res_size);
    Serial1.print("Response data : ");
    for(int i = 0; i < res_size; i++){
            Serial1.print(buf[i],HEX);
            Serial1.print("||");            
        }
    Serial1.println("");      
    PFa = (((uint16_t)buf[0])<<8 | (uint16_t)buf[1] );
    PFb = (((uint16_t)buf[2])<<8 | (uint16_t)buf[3] );
    PFc = (((uint16_t)buf[4])<<8 | (uint16_t)buf[5] );
    sprintf(data_buffer, "PFa : %.2f , PFb : %.2f, PFc : %.2f ",PFa/1000.0,PFb/1000.0,PFc/1000.0);
    Serial1.println(data_buffer);  
    }
  else{
    Serial1.println("Read PF Error!!");
    return false;
    }  
    
  return true;
    
}

void setup() {
  //DEBUG
  Serial1.begin(9600);
  Modbus.setDebugOutput(true);

  Serial1.println("WiFi connecting ......"); 
  if (WiFi.begin(ssid, password)) {
      while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial1.print(".");
      }
  }
  Serial1.println("WiFi connected");  
  Serial1.println("IP address: ");
  Serial1.println(WiFi.localIP());

  Serial1.println("netpie connecting ......");
  client.setServer(mqtt_server, 1883);
  netpie_connect();
  Serial1.println("netpie connected");
  delay(500);
  
}

void loop() {

  if (WiFi.status() != WL_CONNECTED)
    { 
      Reconnect_attempt++;
      Serial1.println("\nLoss connection...");
      WIFI_Connect();
      
      if(Reconnect_attempt == 10){
          Serial1.println("\nLoss connection -> Reset ESP!!!");
          delay(1000);     
          ESP.reset();
        }
    }
   else
    {    
     if (client.connected()) { 
      /* Call this method regularly otherwise the connection may be lost */
      client.loop(); 
          if (timer >= 60000) {               
            if(read_meter()){
              Serial1.println("Read OK Publish...");  
              sprintf(pub_buffer, "{\"data\":{\"Va\": %.2f, \"Vb\": %.2f, \"Vc\": %.2f,\"Ia\": %.2f ,\"Ib\": %.2f,\"Ic\": %.2f,\"Pa\" : %.3f ,\"Pb\" : %.3f,\"Pc\" : %.3f,\"Qa\" : %.3f ,\"Qb\" : %.3f,\"Qc\" : %.3f,\"PFa\" : %.2f ,\"PFb\" : %.2f,\"PFc\" : %.2f,\"Status\": \"OK\" }}",Va/10.0,Vb/10.0,Vc/10.0,(Ia*scaling)/1000.0,(Ib*scaling)/1000.0,(Ic*scaling)/1000.0,(Pa*scaling)/1000.0,(Pb*scaling)/1000.0,(Pc*scaling)/1000.0,(Qa*scaling)/1000.0,(Qb*scaling)/1000.0,(Qc*scaling)/1000.0,PFa/1000.0,PFb/1000.0,PFc/1000.0);
              client.publish("@shadow/data/update",pub_buffer );
              Serial1.println(pub_buffer);           
              }
            else{
              Serial1.println("Read Error Publish...");
              client.publish("@shadow/data/update","{\"data\":{\"Status\": \"Error\" }}" );
              }
            
            timer = 0;
                            
          }
          else timer += 500;
        }
     else {
            if (timer >= 8000) {
            Serial1.println("netpie-reconnect");
            netpie_connect();
            timer = 0;
            }
            else timer += 500;
        }
    delay(500);
    }

}
