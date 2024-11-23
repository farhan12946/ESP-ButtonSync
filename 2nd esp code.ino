#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "SSID";         // Your WiFi SSID
const char* password = "Password";         // Your WiFi Password


// Your Google Apps Script Web App URL
const char* googleScriptURL = "your script url";

const int buttonPin = 19; // D19 for the button
const int ledPin = 2;     // D2 for the onboard LED

int buttonState = HIGH;         // Current debounced button state
int lastButtonState = HIGH;     // Previous reading from the input pin

unsigned long lastDebounceTime = 0;  // The last time the button state changed
unsigned long debounceDelay = 50;    // Debounce delay time in milliseconds

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP); // Button connected between D19 and GND
  pinMode(ledPin, OUTPUT);

  // Connect to WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
}

void loop() {
  // Read the state of the button
  int reading = digitalRead(buttonPin);

  // If the button state has changed
  if (reading != lastButtonState) {
    lastDebounceTime = millis(); // Reset the debouncing timer
    Serial.println("Button state changed, debouncing...");
  }

  // If the button state has been stable for debounceDelay
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the button state has changed (after debouncing)
    if (reading != buttonState) {
      buttonState = reading;

      // If the button is pressed (reading == LOW)
      if (buttonState == LOW) {
        Serial.println("Button pressed, sending signal");
        sendWriteRequest();
      } else {
        Serial.println("Button released");
      }
    }
  }

  // Save the current reading for the next loop
  lastButtonState = reading;

  // Read the value from Google Sheet
  String value = sendReadRequest();

  if (value == "1") {
    // Blink the LED
    Serial.println("Signal received, blinking LED");
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    delay(500);
    // After blinking, reset the value to 0
    sendResetRequest();
  }
  delay(1000); // Wait before next read
}

void sendWriteRequest() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); // Ensure redirects are followed
    String url = String(googleScriptURL) + "?action=write&value=1";
    Serial.println("Sending write request to Google Sheet...");
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Write Response: " + payload);
    } else {
      Serial.printf("Error on HTTP request: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi not connected, cannot send write request");
  }
}

String sendReadRequest() {
  String payload = "";
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); // Ensure redirects are followed
    String url = String(googleScriptURL) + "?action=read";
    Serial.println("Sending read request to Google Sheet...");
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      payload = http.getString();
      Serial.println("Read Response: " + payload);
    } else {
      Serial.printf("Error on HTTP request: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi not connected, cannot send read request");
  }
  return payload;
}

void sendResetRequest() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); // Ensure redirects are followed
    String url = String(googleScriptURL) + "?action=write&value=0";
    Serial.println("Sending reset request to Google Sheet...");
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Reset Response: " + payload);
    } else {
      Serial.printf("Error on HTTP request: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi not connected, cannot send reset request");
  }
}
