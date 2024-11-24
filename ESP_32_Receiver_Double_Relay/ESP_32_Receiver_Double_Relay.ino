#include <WiFi.h>
#include <esp_now.h>

// Define strip name and LED pin for Green Light
const char* STRIP_NAME = "Strip 1";  // Name to match incoming message
#define LED_GREEN_PIN 5              // GPIO5
#define LED_RED_PIN 4                // GPIO4

// Enable or disable verbose output
bool verbose = true;

// Structure to hold the incoming message
struct struct_message {
  uint8_t msgType;
  uint8_t macAddr[6];
  unsigned int Right_Score;
  unsigned int Left_Score;
  unsigned int Seconds_Remaining;
  unsigned int Minutes_Remaining;
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
  char customMessage[32];
};

struct_message incomingMessage;

void setup() {
  Serial.begin(115200);

  // Initialize the LED pins
  pinMode(LED_GREEN_PIN, OUTPUT);
  digitalWrite(LED_GREEN_PIN, HIGH);  // Start with the LED off
  pinMode(LED_RED_PIN, OUTPUT);
  digitalWrite(LED_RED_PIN, HIGH);    // Start with the LED off

  // Initialize WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    if (verbose) Serial.println("ESP-NOW initialization failed");
    return;
  }

  // Register the receive callback function
  esp_now_register_recv_cb(OnDataRecv);

  if (verbose) Serial.println("ESP-NOW Initialized. Waiting for messages...");
}

void loop() {
  delay(1); // Allow ESP tasks to run
}

void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  // Copy incoming data into our structure
  memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));

  // Check if the custom message matches the strip name
  if (String(incomingMessage.customMessage) == STRIP_NAME) {
    if (verbose) {
      Serial.print("Message from ");
      Serial.println(STRIP_NAME);
      Serial.print("Red Light: ");
      Serial.println(incomingMessage.Red_Light ? "ON" : "OFF");
    }

    // Control the LED for Green Light status
    if (incomingMessage.Green_Light) {
      digitalWrite(LED_GREEN_PIN, LOW);  // Turn on the LED
    } else {
      digitalWrite(LED_GREEN_PIN, HIGH);  // Turn off the LED
    }

    // Control the LED for Red Light status
    if (incomingMessage.Red_Light) {
      digitalWrite(LED_RED_PIN, LOW);  // Turn on the LED
    } else {
      digitalWrite(LED_RED_PIN, HIGH);  // Turn off the LED
    }

    if (verbose) Serial.println();
  }
}

