/**
 **************************************************
 * @file        Inkplate4TEMPERA_HTTP_Web_Server.ino
 * @brief       Standalone WiFi web server example for Soldered Inkplate 4TEMPERA.
 *
 * @details     Demonstrates how to use Inkplate 4TEMPERA as a simple standalone
 *              WiFi access point and HTTP web server. After connecting a PC,
 *              smartphone, or other WiFi-capable device to the Inkplate’s
 *              access point, a web page can be opened in a browser where text
 *              can be entered and sent directly to the Inkplate e-paper display.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Hardware:   Inkplate 4TEMPERA, USB cable
 * - Extra:      WiFi-capable device with a web browser (PC, laptop, smartphone)
 *
 * Configuration:
 * - Boards Manager -> Inkplate Boards -> Soldered Inkplate4TEMPERA
 * - Inkplate operates as a WiFi Access Point (AP)
 * - SSID and password can be changed in the code via #define macros
 *
 * Don't have Inkplate Boards in Arduino Boards Manager?
 * See https://docs.soldered.com/inkplate/4TEMPERA/quick-start-guide/
 *
 * How to use:
 * 1) Upload the sketch to Inkplate 4TEMPERA.
 * 2) Connect your device to the Inkplate WiFi access point.
 * 3) Open the IP address shown on the Inkplate display in a web browser.
 * 4) Enter text into the web page and press “Send to display”.
 * 5) The submitted text appears on the Inkplate display.
 *
 * Expected output:
 * - Inkplate display shows its IP address and received text.
 * - Web page allows sending custom text to the display.
 *
 * Notes:
 * - This is a basic demonstration of using Inkplate as a web server.
 * - Intended for simple interaction and prototyping.
 * - More advanced web interfaces and logic can be built on top of this example.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 *
 * @author      Soldered
 * @date        2022-12-02
 * @license     GNU GPL V3
 **************************************************/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE4TEMPERA
#error "Wrong board selection for this example, please select Inkplate 4 TEMPERA in the boards menu."
#endif

#include "Inkplate.h"   //Include Inkplate library to the sketch
#include "htmlCode.h"   //Include .h file where we stored out html code of our web page
#include <WebServer.h>  //Include ESP32 library for Web server
#include <WiFi.h>       //Include ESP32 WiFi library
#include <WiFiClient.h> //Include ESP32 WiFi library for AP
#include <uri/UriBraces.h>

#define ssid "Inkplate 4TEMPERA"
#define pass "Soldered"

Inkplate display(INKPLATE_1BIT); // Create an object on Inkplate library and also set library into 1 Bit mode (BW)
WebServer server(80);            // Create Web server on port 80 (HTTP port number)

IPAddress serverIP;
String txt;

void setup()
{
    display.begin();                    // Init Inkplate library (you should call this function ONLY ONCE)
    display.clearDisplay();             // Clear frame buffer of display
    display.display();                  // Put clear image on display
    display.setTextSize(2);             // Scale text to be two times bigger then original (5x7 px)
    display.setCursor(30, 30);          // Set cursor position so text is visible
    display.setTextColor(BLACK, WHITE); // Set text color to black and background color to white
    display.setTextWrap(true);          // If text does not fit on screen, send it to new line

    WiFi.begin();            // Init. WiFi library
    WiFi.mode(WIFI_AP);      // Set WiFi to Access point mode
    WiFi.softAP(ssid, pass); // Set SSID (WiFi name) and password for Access point

    serverIP = WiFi.softAPIP(); // Get the server IP address

    server.on("/", handleRoot); // If you open homepage, go to handle root function
    server.on(UriBraces("/string/{}"),
              handleString); // If you send some text to Inkplate, go to handleString function. Note that {} brackets at
                             // the end of address. That means that web address has some arguments (our text!).
    server.begin();          // Start the web server
    updatePaper();
}

void loop()
{
    server.handleClient(); // You have to constantly read if there is any new client connected to web server
}

void updateHTML()
{ // This function will send response to client and send HTML code of our web page
    server.send(200, "text/html", s);
}

void handleRoot()
{ // This function will send response to client if client open a root (homepage) of our web page
    updateHTML();
}

void handleString()
{ // This function will send response to client, send HTML code of web page, get the text from argument sent in web page
  // address and refresh screen with new text
    txt = server.arg(0);
    updateHTML();
    updatePaper();
}

void updatePaper()
{                              // This function updates screen with new data (text)
    display.clearDisplay();    // Clear everything from epaper frame buffer
    display.setCursor(30, 30); // Print out instruction on how to connect to Inkplate WiFi and how to open a web page
    display.print("Connect to: ");
    display.print(ssid);
    display.print(" WiFi");
    display.setCursor(30, 70);
    display.print("with password: ");
    display.print(pass);
    display.setCursor(30, 150);
    display.print("Open Your web browser and open");
    display.setCursor(30, 190);
    display.print("http://");
    display.print(serverIP);
    display.println('/');
    display.println();
    display.setCursor(30, 230);
    display.fillRect(10, 270, 580, 4, BLACK);
    display.println("User text:"); // Print out what user typed in web page
    display.setCursor(30, 290);
    display.print(txt);
    display.display(); // Send everything to screen (refresh the screen)
}
