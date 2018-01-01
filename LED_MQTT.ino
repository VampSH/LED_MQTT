#include <FastLED.h>
#include <Ethernet.h>
#include <PubSubClient.h>

#define NUM_LEDS 100      //Define how many leds in your strip
#define DATA_PIN 3        //Pin to connect DATA LED on the arduino
//#define CLOCK_PIN 13    //Pin to connect Clock LED on the arduino 

//HSV configuration
int LED_Hue = 100;        //Initialize Hue is the 'angle' around a color wheel (0-255)
int LED_Saturation = 255; //Initialize Saturation is how 'rich' (versus pale) the color is (0-255)
int LED_Value = 255;      //Initialize Value is how 'bright' (versus dim) the color is (0-255)

int Mode_Select = 2;      //Initialize Mode selection for the patern of LED strip (1-4)
int LED_Select = 4;       //Initialize LED_Select for LED designation in the case switch

CRGBArray<NUM_LEDS> leds; // Define the array of leds
static uint8_t hue = 0;
byte mac[] = {0x11,0xED,0x1E,0x1F,0xFF,0xEF}; // Set your MAC address to a unique value in your network
IPAddress dnServer(172, 16, 0, 1);            // set the dns server ip
IPAddress gateway(172, 16, 0, 1);             // set the router's gateway address
IPAddress subnet(255, 255, 0, 0);             // set the subnet mask
IPAddress ip(172, 16, 11, 1);                 // set the IP address to a unique value in your network
IPAddress server(172, 16, 10, 1);             //set the IP address of your MQTT server
EthernetClient ethClient;
PubSubClient client(ethClient);

// function that pickup the MQTT massage from the lestening topic
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  char message_buff[length]; 
  int PLD;                            //initialize PLD: value of the Payload
  for (int i=0; i<length; i++) {
    Serial.print((char)payload[i]);
    message_buff[i] = payload[i];     //create a buffer message with payload
  }
  PLD = atoi(message_buff);           //copy buffer message to PLD (convert to int)
  if(strcmp(topic,"Mode_Select") == 0) Mode_Select = PLD;
  if(strcmp(topic,"LED_Hue") == 0) LED_Hue = PLD;
  if(strcmp(topic,"LED_Saturation") == 0) LED_Saturation = PLD;
  if(strcmp(topic,"LED_Value") == 0) LED_Value = PLD;
  FastLED.clear();
}

void fadeall() {
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(250);
  }
}

//Function to reconnect to MQTT server
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient2", "User", "Password")) { //type the User and Password for your MQTT broker
      Serial.println("connected");
      client.subscribe("Mode_Select");
      client.subscribe("LED_Hue");
      client.subscribe("LED_Saturation");
      client.subscribe("LED_Value");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() { 
  Serial.begin(115200);
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip, dnServer, gateway, subnet);
    Serial.print("IP = ");
    Serial.println(Ethernet.localIP());
    delay(1500);          // Allow the hardware to sort itself out
  }
  client.setServer(server, 1883);
  client.setCallback(callback);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  reconnect();
}

void loop() { 
  if (!client.connected()) reconnect();
  client.loop();
  switch (Mode_Select) {
    case 1:
      //do something when Mode_Select equals 1
      for(int i = 0; i < NUM_LEDS; i++) {                 //set CHSV for the led stripe
        leds[i] = CHSV(LED_Hue,LED_Saturation,LED_Value);
      }
      FastLED.show();                                     // Send the signal to the led stripe
      LED_Hue++;                                          //increase LED_Hue
      if(LED_Hue == 255)LED_Hue = 0;                      //reset LED_Hue if reaches max value (255)
      delay(25);
      break;
    case 2:
      //do something when Mode_Select equals 2
      for(int i = 0; i < NUM_LEDS; i++) {                     // First slide the led in one direction
        leds[i] = CHSV(LED_Hue++, LED_Saturation, LED_Value); // Set the i'th led to LED_Hue and increase the LED_Hue
        FastLED.show();                                       // Send the signal to the led stripe
        fadeall();                                            // now that we've shown the leds, fade all
        delay(10);
      }
        
      for(int i = (NUM_LEDS)-1; i >= 0; i--) {                // Now go in the other direction.
        leds[i] = CHSV(LED_Hue++, LED_Saturation, LED_Value); // Set the i'th led to LED_Hue value then increase LED_Hue
        FastLED.show();                                       // Send the signal to the led stripe
        fadeall();                                            // now that we've shown the leds, fade all
        delay(10);
      }
      break;
    case 3:
      //do something when Mode_Select equals 3
      for(int i = 0; i < NUM_LEDS/2; i++) {
        leds.fadeToBlackBy(40);                                 // fade everything out
        leds[i] = CHSV(LED_Hue++,LED_Saturation,LED_Value);     // let's set an led value
        leds(NUM_LEDS/2,NUM_LEDS-1) = leds(NUM_LEDS/2 - 1 ,0);  // now, let's first 20 leds to the top 20 leds,
        FastLED.delay(25);
      }
      break;
    case 4:
      //do something when Mode_Select equals 4
      for(int i = 0; i < NUM_LEDS; i++) {                 //Set all the leds to the same CHSV
        leds[i] = CHSV(LED_Hue,LED_Saturation,LED_Value);
      }
      FastLED.show();                                     // Send the signal to the led stripe
      break;
    default: 
      // if nothing else matches, do the default
      // default is optional
    break;
  }
}