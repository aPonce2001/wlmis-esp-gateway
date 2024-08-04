#include <WiFi.h>
#include <HTTPClient.h>

#define RXP2 16
#define TXP2 17

// State machine
enum State
{
    CONNECTING_TO_WIFI,
    WAITING_ACTIVATION_STATE,
    SENDING_ACTIVATION_STATE,
    SENDING_DATA
};

State currentState = CONNECTING_TO_WIFI;

// WIFI
const char *ssid = "wlmis";
const char *password = "wlmis123";

// API
const String server = "http://192.168.200.16:5000/api";

// HTTP Client
HTTPClient http;

void setup()
{
    Serial.begin(115200);
    Serial2.begin(9600, SERIAL_8N1, RXP2, TXP2);
}

void loop()
{
    switch (currentState)
    {
    case CONNECTING_TO_WIFI:
        connectToWifi();
        break;
    case WAITING_ACTIVATION_STATE:
        waitForActivation();
        break;
    case SENDING_ACTIVATION_STATE:
        sendActivation();
        break;
    case SENDING_DATA:
        sendData();
        break;
    default:
        break;
    }
}

void connectToWifi()
{
    Serial.println("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        continue;
    }
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    currentState = WAITING_ACTIVATION_STATE;
}

void waitForActivation()
{
    if (!Serial2.available())
    {
        return;
    }

    String activation = Serial2.readString();
    Serial.println(activation);
    currentState = SENDING_ACTIVATION_STATE;
}

void sendActivation()
{
    String sendActivationEndpoint = server + "/sensor-activated-state";
    http.begin(sendActivationEndpoint);

    int httpResponseCode = http.POST("");

    if (httpResponseCode == 200)
    {
        Serial.println("Toggled sensor state");
        currentState = SENDING_DATA;
    }

    http.end();
}

void sendData()
{
    if (!Serial2.available())
    {
        return;
    }

    String value = Serial2.readString();
    Serial.println(value);

    if (value == "sensor-state")
    {
        currentState = SENDING_ACTIVATION_STATE;
        return;
    }

    String sendDataEndpoint = server + "/water-level";
    http.begin(sendDataEndpoint);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(value);

    if (httpResponseCode == 200)
    {
        Serial.print("Data sent: ");
        Serial.println(value);
    }

    http.end();
}