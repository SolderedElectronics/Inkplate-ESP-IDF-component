/**
 **************************************************
 * @file        Inkplate4TEMPERA_HTTP_POST_Request.ino
 * @brief       WiFi HTTP POST request example using webhook.site (Inkplate 4TEMPERA).
 *
 * @details     Demonstrates how to connect Inkplate 4TEMPERA to a WiFi network and
 *              send periodic HTTP POST requests to webhook.site. This free
 *              online service allows real-time inspection of HTTP requests,
 *              making it useful for testing IoT data transmission.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Hardware:   Inkplate 4TEMPERA, USB cable
 * - Extra:      Stable WiFi connection, webhook.site URL
 *
 * Configuration:
 * - Boards Manager -> Inkplate Boards -> Soldered Inkplate4TEMPERA
 * - Serial settings: 115200 baud
 * - Enter your WiFi credentials (ssid, pass) in the code
 * - Set WEBHOOK_PATH to your unique webhook.site path
 *
 * Don't have Inkplate Boards in Arduino Boards Manager?
 * See https://docs.soldered.com/inkplate/4TEMPERA/quick-start-guide/
 *
 * How to use:
 * 1) Visit https://webhook.site and copy your unique webhook URL.
 * 2) Paste only the path part (e.g. "/abcd-1234-efgh") into WEBHOOK_PATH.
 * 3) Enter your WiFi credentials in the sketch.
 * 4) Upload the sketch to Inkplate 4TEMPERA.
 * 5) Open Serial Monitor (115200 baud) to observe connection status.
 * 6) Watch incoming POST requests live on webhook.site.
 *
 * Expected output:
 * - Inkplate display shows example information.
 * - Serial Monitor logs WiFi connection and POST status.
 * - webhook.site displays incoming POST requests every 20 seconds.
 *
 * Notes:
 * - Example uses HTTP (port 80) for simplicity.
 * - Data is sent in URL-encoded format (application/x-www-form-urlencoded).
 * - Replace example data with real sensor readings if needed.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 *
 * @author      Soldered
 * @date        2026-01-01
 * @license     GNU GPL V3
 **************************************************/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE4TEMPERA
#error "Wrong board selection for this example, please select Inkplate 4 TEMPERA in the boards menu."
#endif

// Include needed libraries
#include "Inkplate.h"
#include "WiFi.h"

// Create objects
Inkplate display(INKPLATE_1BIT);
WiFiClient client;

// Interval between POST requests (seconds)
#define POSTING_INTERVAL_IN_SECS 20

// WiFi credentials
const char *ssid = "";
const char *pass = "";

// Webhook.site settings
const char *server = "webhook.site";
const char *WEBHOOK_PATH = "/YOUR-UNIQUE-WEBHOOK-ID"; // e.g. "/abcd-1234-efgh"

// Last connection time
unsigned long lastConnectionTime = 0;

void setup()
{
    Serial.begin(115200);

    // Init Inkplate
    display.begin();
    display.clearDisplay();
    display.setTextColor(BLACK, WHITE);
    display.setTextSize(6);

    display.printf("HTTP POST example\n\n");
    display.printf("Using webhook.site\n\n");
    display.printf("Open Serial Monitor\nat 115200 baud");
    display.display();

    // Connect to WiFi
    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(ssid, pass);

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
}

void loop()
{
    if ((unsigned long)(millis() - lastConnectionTime) > POSTING_INTERVAL_IN_SECS * 1000UL)
    {
        display.clearDisplay();

        if (!client.connect(server, 80))
        {
            Serial.println("Connection failed");
            lastConnectionTime = millis();
            client.stop();
            return;
        }

        // Example data (replace with sensor readings if needed)
        int value = random(40);

        // URL-encoded POST body
        String data = "value=" + String(value);

        // Send HTTP POST request
        client.print(String("POST ") + WEBHOOK_PATH + " HTTP/1.1\r\n");
        client.print(String("Host: ") + server + "\r\n");
        client.println("Connection: close");
        client.println("User-Agent: Inkplate-ESP32");
        client.println("Content-Type: application/x-www-form-urlencoded");
        client.print("Content-Length: ");
        client.println(data.length());
        client.println();
        client.print(data);

        Serial.print("POST sent: ");
        Serial.println(data);

        lastConnectionTime = millis();
        delay(250);

        client.stop();
    }
}