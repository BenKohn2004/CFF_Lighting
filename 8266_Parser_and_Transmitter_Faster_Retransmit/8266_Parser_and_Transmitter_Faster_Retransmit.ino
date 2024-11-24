#include <ESP8266WiFi.h>
#include <espnow.h>

#define Test_Mode true
#define verbose false
#define relay false
#define BoxName "Strip 1"
#define fast_retransmit true

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

// For Reading Favero Serial
const unsigned int MAX_MESSAGE_LENGTH = 10;
const unsigned int MAX_SERIAL_BUFFER_BYTES = 128;
const char STARTING_BYTE = 255;

// Lights for the Relay Outputs
const int RedLights = 4;           // D2
const int GreenLights = 5;         // D1
const int White_RedLights = 14;    // D5
const int White_GreenLights = 12;  // D6

// Shows if new data is available for display
bool new_data = false;

// Initializes Message_Position
unsigned int message_pos = 0;

unsigned long lastTransmitTime = 0;
unsigned long transmitInterval = 10;  // send readings timer

unsigned long lastChangeTime = 0;
unsigned long changeInterval = 1000;  // How often the light changes

unsigned long previousMillis = 0;  // will store the last time the dot was printed
const long interval = 1000;        // interval at which to print the dot (milliseconds)

// ESPNow communication packet
typedef struct struct_message {
  uint8_t msgType;     // Transmitter, Receiver
  uint8_t macAddr[6];  // The MAC address of the message sender
  int unsigned Right_Score;
  int unsigned Left_Score;
  int unsigned Seconds_Remaining;
  int unsigned Minutes_Remaining;
  bool Green_Light;
  bool Red_Light;
  bool White_Green_Light;
  bool White_Red_Light;
  bool Yellow_Green_Light;
  bool Yellow_Red_Light;
  bool Yellow_Card_Green;
  bool Yellow_Card_Red;
  bool Red_Card_Green;
  bool Red_Card_Red;
  bool Priority_Left;
  bool Priority_Right;
  char customMessage[32];  // Array to hold a fixed-length string of 31 characters + null terminator, This is the name of the sender
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  if (verbose) {
    Serial.print(BoxName);
    Serial.print(": Last Packet Send Status: ");
    if (sendStatus == 0) {
      Serial.println("Delivery success");
    } else {
      Serial.println("Delivery fail");
    }
  }
}

void parseMacAddress(const String &macAddressStr, uint8_t macAddr[6]) {
  char macChars[18];
  macAddressStr.toCharArray(macChars, sizeof(macChars));
  sscanf(macChars, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
         &macAddr[0], &macAddr[1], &macAddr[2],
         &macAddr[3], &macAddr[4], &macAddr[5]);
}

void setup() {
  // Init Serial Monitor
  if (Test_Mode) {
    Serial.begin(115200);
  } else {
    Serial.begin(2400);
  }

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Serial.println("");
  Serial.println(WiFi.macAddress());

  // Get MAC address as a string
  String macAddressStr = WiFi.macAddress();

  // Parse MAC address string and store it in myData.macAddr
  parseMacAddress(macAddressStr, myData.macAddr);

  // Use strcpy to copy the string "Favero_One" into the customMessage array
  strcpy(myData.customMessage, BoxName);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Sets Arduino Pins to Output Mode
  pinMode(RedLights, OUTPUT);
  pinMode(GreenLights, OUTPUT);
  pinMode(White_RedLights, OUTPUT);
  pinMode(White_GreenLights, OUTPUT);

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop() {

  if (fast_retransmit) {
    if (millis() - lastTransmitTime > transmitInterval) {  // by alternating Green and Red Lights
      new_data = true;
    }
  }

  if (Test_Mode) {                                     // Allows for testing the receiver functionality
    if (millis() - lastChangeTime > changeInterval) {  // by alternating Green and Red Lights
      lastChangeTime = millis();
      if (myData.Green_Light) {
        if (verbose) {
          Serial.println("Red Light Lit");  // Red Light Lit
        }
        myData.Green_Light = 0;       // Turns off the Green Light
        myData.Red_Light = 1;         // Turns on the Red Light
        myData.Yellow_Red_Light = 0;  // Turns on Yellow Light for Red
      } else {
        if (verbose) {
          Serial.println("Green Light Lit");  // Green Light Lit
        }
        myData.Green_Light = 1;       // Turns on Green Light
        myData.Red_Light = 0;         // Turns off Red Light
        myData.Yellow_Red_Light = 1;  // Turns off Yellow Light for Red
      }
    }
  } else {

    Favero_Parser();  // Calls the Function to Parse the Favero Data
  }

  if ((new_data == true) || (millis() - lastTransmitTime > transmitInterval)) {
    lastTransmitTime = millis();
    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    if (verbose) {
      Serial.print("Red Light is: ");
      Serial.print(myData.Red_Light);
      Serial.print(". Green Light is: ");
      Serial.println(myData.Green_Light);
    }
    new_data = false;
  }

  if (relay) {
    if (myData.Red_Light) {
      digitalWrite(RedLights, HIGH);  // Turn the Red Relay on
    } else {
      digitalWrite(RedLights, LOW);  // Turn the Red Relay off
    }


    if (myData.Green_Light) {
      digitalWrite(GreenLights, HIGH);  // Turn the Green Relay on
    } else {
      digitalWrite(GreenLights, LOW);  // Turn the Green Relay off
    }
  }
}
