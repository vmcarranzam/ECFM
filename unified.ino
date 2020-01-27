//libraries and variables for WiFi connection
#include <WiFi.h>
const char* ssid     = "";     // insert your network SSID (name of wifi network) between ""
const char* password = ""; // insert your network password between ""
WiFiClient espClient;
  
//libraries and variables to implement MQTT protocol
#include <PubSubClient.h>
  const char*  usuarioBroker=""; // insert broker user
  const char* claveBroker=""; // insert broker password
  const char* direccionBroker=""; // insert broker address
  PubSubClient client(espClient);
  long lastMsg = 0;
  char msg[50];
  int value = 0;


//libraries and variables to work with MPU6050 module, the standard one is not compatible with the ESP32, but the one referenced below works perfectly
  #include <MPU6050_tockn.h>
  #include <Wire.h>
  MPU6050 mpu6050(Wire);
  float acx,acy,acz,gx,gy,gz,anx,any,anz;

// RTOS handlers, for multicore tasking
  TaskHandle_t Sensores, Publicar;
  SemaphoreHandle_t datos; // for interaction between cores
  
// libraries and variables to work with the load cell using an HX711 driver module
  #include "HX711.h"
  const int LOADCELL_DOUT_PIN = 15;
  const int LOADCELL_SCK_PIN = 2;
  long cal;
  HX711 scale;
  float peso;

// encoder variables
  int pulso=4;
  int contador;
  long vuel;

// function for MQTT client reconnection
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      // Wait 3 seconds before retrying
      delay(3000);
    }
  }
}

//Task for Core 0 (default one): get sensor output
void DAQ( void * pvParameters ){
  datos=xSemaphoreCreateMutex();
    for (;;) {
    xSemaphoreTake( datos, portMAX_DELAY );   
  datosMPU(acx,acy,acz,gx,gy,gz,anx,any,anz);
  datosCelda(peso);
  datosEncoder(contador, vuel);
  delay(100);

// uncomment the next line to monitor the data flow via the serial monitor, this will introduce a delay in the flow
//Serial.print("Angulo X: ");Serial.print(anx); Serial.print("  Angulo Y: "); Serial.print(any);Serial.print("  Angulo Z: "); Serial.println(anz);
  
    xSemaphoreGive( datos );
    delay(50);
  }
}

//Parameters to enable the two main cores woeking in parallel
  xTaskCreatePinnedToCore(
    &DAQ,            /* Task function. */
    "Sensores",                 /* name of task. */
    3000,                    /* Stack size of task */
    NULL,                     /* parameter of the task */
    1,                        /* priority of the task */
    &Sensores,                   /* Task handle to keep track of created task */
    0);                       /* Core */
  delay(500);
 
//setup WiFi
  WiFi.begin(ssid, password);
  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    // Give 3 seconds to establish connection
    if(millis()<3100){
    // wait 500 milliseconds for re-trying
    delay(500);
    }
    else{
     //restart after 3 seconds of failed attempts 
     ESP.restart();
      }

  }


//setup MQTT
  client.setServer(direccionBroker, 1883);
  client.setCallback(callback);
  
  //setup load cell
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.tare();
  scale.set_scale(133868);

  //SDA en 18 y SCL en 19
  Wire.begin(18,19);
  //setup MPU  
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  }

void callback(char* topic, byte* message, unsigned int length) {
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }

}

void loop() {
 float tinicial=millis();
 delay(50);
 datos=xSemaphoreCreateMutex();
 xSemaphoreTake( datos, portMAX_DELAY );   

if(!client.connected()){
  reconnect();
}
client.loop();

  //MQTT
  long now = millis();
  if (now - lastMsg > 100) {
    lastMsg = now;

  
    //post data output from the MPU6050, each parameter from each axis is posted in a separate topic
    char angx[8];
    char angy[8];
    char angz[8];
    char accx[8];
    char accy[8];
    char accz[8];
    char gyx[8];
    char gyy[8];
    char gyz[8];  

    char vueltas[8];
    char contadorp[8];

    char pesoim[8];
    
    char Tiempo[8];


    dtostrf(anx, 1, 2, angx);
    client.publish("anx",angx);
    dtostrf(any, 1, 2, angy);
    client.publish("any",angy);
    dtostrf(anz, 1, 2, angz);
    client.publish("anz",angz);

    dtostrf(acx, 1, 2, accx);
    client.publish("acx",accx);
    dtostrf(any, 1, 2, accy);
    client.publish("acy",accy);
    dtostrf(anz, 1, 2, accz);
    client.publish("acz",accz);

    dtostrf(gx, 1, 2, gyx);
    client.publish("gx",gyx);
    dtostrf(gy, 1, 2, gyy);
    client.publish("gy",gyy);
    dtostrf(gz, 1, 2, gyz);
    client.publish("gz",gyz);
         

    //post encoder topics
    dtostrf(vuel, 1, 2, vueltas);
    client.publish("vueltas",vueltas);
    dtostrf(contador, 1, 2, contadorp);
    client.publish("pulsos",contadorp);

    //post load cell topics
    dtostrf(peso, 1, 2, pesoim);
    client.publish("peso",pesoim);


    float tiempo=millis()-tinicial-100;
    dtostrf(tiempo, 1, 2, Tiempo);
    client.publish("tiempo",Tiempo);
    
    xSemaphoreGive( datos );
    delay(50);


}
}


//Function to get data output from MPU6050 module, must be saved in global variables when called
void datosMPU(float & acx,float & acy, float & acz, float & gx, float & gy, float & gz, float & anx, float & any, float & anz){
  mpu6050.update();      
  acx=mpu6050.getAccX();
  acy=mpu6050.getAccY();
  acz=mpu6050.getAccZ();
  gx=mpu6050.getGyroX();
  gy=mpu6050.getGyroY();
  gz=mpu6050.getGyroZ();
  anx=mpu6050.getAngleX();
  any=mpu6050.getAngleY();
  anz=mpu6050.getAngleZ();

  delay(50);
}

//Function to get data output from the load cell, must be saved in global variables when called
void datosCelda(float & peso){

  if (scale.is_ready()) {
    long reading = scale.read();
    peso=scale.get_units();

  } 
  
  delay(50);

}

//Function to get data output from optical encoder, must be saved in global variables when called
void datosEncoder(int & cont,long & vueltas){

  
  if (digitalRead(pulso)==HIGH ){
    cont=cont+1;
    vueltas=cont/20;

  }

  delay(50);


}
