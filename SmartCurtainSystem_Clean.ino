/*
  Ameba 1 SmartCurtainSystem

  Item list:
  1. Magnetic reed switch (closed when near a magnet)
  2. DHT temperature sensor
  3. Ameba 1 dev. board
  4. Jumper wires 
  5. Servo motor

  Application:
  when windows open, ameba1 #1 send a message to ameba1 #2 to trigger a buzzer 
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <AmebaServo.h>
#include <String.h>

#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11

#define cOpen         false
#define cClose        true
#define inputPin      13
#define closeCurtain  0
#define openCurtain   179

// Update these with values suitable for your network.
char ssid[] = "YourWiFiName";     // your network SSID (name)
char pass[] = "YourWiFiPassword";  // your network password

int status  = WL_IDLE_STATUS;    // the Wifi radio's status

char mqttServer[] = "cloud.amebaiot.com";
char clientId[]       = "amebaClientXXX";    //replacce XXX with your initials
char clientUser[]     = "yourUsername";      //Same username you used to login amebaiot.com
char clientPass[]     = "YourPassword";      //Same password you used to login amebaiot.com
char publishTopic[]   = "outTopic";
char publishHello[]   = "---MQTT server Connected!---";
char publishClosed[]  = "Curtain is closed";
char publishOpened[]  = "Curtain opened, closing now";
char subscribeTopic[] = "inTopic"; 
char pubClosing[]     = "Curtain closing";
char pubOpenning[]    = "Curtain openning";
// Callback function header
void callback(char* topic, byte* payload, unsigned int length);

DHT dht(DHTPIN, DHTTYPE);
WiFiClient wifiClient;
PubSubClient client(mqttServer, 1883, callback, wifiClient);
AmebaServo myservo; 

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
    char arr[30] = {NULL};
    int i = 0;
    printf("the payload length is %d\n",length);
  
    for( i = 0; length > 0 ; i++){
        arr[i] = *payload;
        payload++;
        length--;
        printf("payload received: %c\n",arr[i]);
    }
    
    if(arr[0] == 'o' && arr[1] == 'n'){
        printf("---Received a ON msg, openning curtain now---\n"); 
        myservo.write(openCurtain);
        client.publish(publishTopic,pubOpenning);   
    } else if (arr[0] == 'o' && arr[1] == 'f' && arr[2] == 'f') {
        printf("---Received a OFF msg, closing curtain now---"); 
        myservo.write(closeCurtain);
        client.publish(publishTopic,pubClosing);
    } else {
        printf("---Received an INVALID msg---\n");
    }
    
} // end of MQTT callback


bool curClose(){
  if(digitalRead(inputPin)){
    Serial.println("magnet PRESENT");
    return cClose;
  } else {
    Serial.println("magnet AWAY");
    return cOpen;
  }
} // end of curClose()


void setup()
{
  Serial.begin(9600);
  pinMode(inputPin, INPUT);
  myservo.attach(9); // attaches the servo on pin 9 to the servo object
  myservo.write(openCurtain); //defualt curtain is open
  
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // Note - the default maximum packet size is 512 bytes. If the
  // combined length of clientId, username and password exceed this,
  // you will need to increase the value of MQTT_MAX_PACKET_SIZE in
  // PubSubClient.h

  if (client.connect(clientId, clientUser, clientPass)) {
    client.publish(publishTopic, publishHello);
    client.subscribe(subscribeTopic);
  }
  
  dht.begin();
} //end of setup()


int pubCount = 0;
int svoCount = 0;
void loop(){
    // Read temperature as Celsius (the default)
    float tempC = dht.readTemperature();
    printf("The temperature is %f *C\r\n",tempC);

    if(tempC > 22.5){
        if(curClose()){
            if( pubCount == 0){
                client.publish(publishTopic, publishClosed);
                pubCount = 1;
            }
            Serial.println("curtain closed");
        } else {
            if (svoCount == 0){
                myservo.write(closeCurtain);
                svoCount++;
                client.publish(publishTopic, publishOpened);
            }   
            pubCount = 0;
            Serial.println("Curtain is closing");
        }
    } else {
        svoCount = 0;
    }
  
    client.loop();
    delay(2000);
} // end of loop()
