#include <StringSplitter.h>
#include <Servo.h>
#include <ESP8266WiFi.h>

#define LED 2

WiFiServer server(80);

const char *ssid = "poil_2.4GHz";
const char *password = "poildepoche";

const int potPin = A0;
int potentiometerValue;

const int servoPinYaw = 2;
const int yawMin = 0;
const int yawMax = 90;

const int servoPinPitch = 0;
const int pitchMin = 90;
const int pitchMax = 180;

const int servoPinLid1 = 4;
const int lid1Min = 110;
const int lid1Max = 180;

const int servoPinLid2 = 5;
const int lid2Min = 60;
const int lid2Max = 135;

Servo servoYaw;
Servo servoPitch;
Servo servoLid1;
Servo servoLid2;

char delimiter = '/';

void handleRoot();
void handleLED();
void handleNotFound();

void setup()
{
    SetupSerial();
    SetupWifi();
    SetupLED();
    SetupServo();
    SetupPot();
}

void SetupPot()
{
    pinMode(potPin, OUTPUT);
}

void SetupSerial()
{
    Serial.begin(115200);
}

void SetupServo()
{
    servoYaw.attach(servoPinYaw);
    servoPitch.attach(servoPinPitch);
    servoLid1.attach(servoPinLid1);
    servoLid2.attach(servoPinLid2);

    ResetServos();
}

void ResetServos()
{
    SetServoAngle(servoYaw, 0.5, yawMin, yawMax);
    SetServoAngle(servoPitch, 0.5, pitchMin, pitchMax);
    SetLidsOpen(true);
}

void SetEyeAngles(float yaw, float pitch)
{
    SetServoAngle(servoPitch, pitch, pitchMin, pitchMax);
    SetServoAngle(servoYaw, yaw, yawMin, yawMax);
}

void SetLidsAngle(float lid1, float lid2)
{
    SetServoAngle(servoLid1, lid1, lid1Min, lid1Max);
    SetServoAngle(servoLid2, lid2, lid2Min, lid2Max);
}

void SetLidsOpen(bool open)
{
    if (open)
    {
        SetServoAngle(servoLid1, 0, lid1Min, lid1Max);
        SetServoAngle(servoLid2, 1, lid2Min, lid2Max);
    }
    else
    {
        SetServoAngle(servoLid1, 1, lid1Min, lid1Max);
        SetServoAngle(servoLid2, 0, lid2Min, lid2Max);
    }
}

float MapF(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void SetServoAngle(Servo &servo, float normalizedValue, int min, int max)
{
    int angle = round(MapF(normalizedValue, 0, 1, min, max));
    Serial.println(angle);
    servo.write(angle);
}

void SetupLED()
{
    pinMode(LED, OUTPUT);
}

void SetupWifi()
{
    Serial.println("");
    WiFi.mode(WIFI_OFF); // Prevents reconnection issue (taking too long to connect)
    delay(1000);
    WiFi.mode(WIFI_STA); // This line hides the viewing of ESP as wifi hotspot

    WiFi.begin(ssid, password);
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.println("TCP server started");
}

void loop()
{
    LoopPot();

    WiFiClient client;
    client = server.available();

    // Serial.println("HERE");

    if (client == 1)
    {
        String request = client.readStringUntil('\n');
        client.flush();
        Serial.println("Got request:");
        Serial.println(request);

        StringSplitter *requestSplitter = new StringSplitter(request, delimiter, 5);

        if (requestSplitter->getItemCount() > 0)
        {
            String commandType = requestSplitter->getItemAtIndex(1); // first item is just the "GET"
            if (commandType == "SET_EYE")
            {
                String yawString = requestSplitter->getItemAtIndex(2);
                String pitchString = requestSplitter->getItemAtIndex(3);

                Serial.print("EYE request: ");
                Serial.print(yawString + ", ");
                Serial.println(pitchString);

                float yawFloat = yawString.toFloat();
                float pitchFloat = pitchString.toFloat();

                SetEyeAngles(yawFloat, pitchFloat);
            }
        }

        client.println("HTTP/1.1 200 OK");
    }
}

// // Variable to store the HTTP request
// String header;

// // Current time
// unsigned long currentTime = millis();
// // Previous time
// unsigned long previousTime = 0;
// // Define timeout time in milliseconds (example: 2000ms = 2s)
// const long timeoutTime = 2000;

// void LoopWifi()
// {
//     WiFiClient client = server.available(); // Listen for incoming clients

//     if (client)
//     {                                  // If a new client connects,
//         Serial.println("New Client."); // print a message out in the serial port
//         String currentLine = "";       // make a String to hold incoming data from the client
//         currentTime = millis();
//         previousTime = currentTime;
//         while (client.connected() && currentTime - previousTime <= timeoutTime)
//         { // loop while the client's connected
//             currentTime = millis();
//             if (client.available())
//             {                           // if there's bytes to read from the client,
//                 char c = client.read(); // read a byte, then
//                 Serial.write(c);        // print it out the serial monitor
//                 header += c;
//                 if (c == '\n')
//                 { // if the byte is a newline character
//                     // if the current line is blank, you got two newline characters in a row.
//                     // that's the end of the client HTTP request, so send a response:
//                     if (currentLine.length() == 0)
//                     {
//                         // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
//                         // and a content-type so the client knows what's coming, then a blank line:
//                         client.println("HTTP/1.1 200 OK");
//                         client.println("Content-type:text/html");
//                         client.println("Connection: close");
//                         client.println();

//                         // // turns the GPIOs on and off
//                         // if (header.indexOf("GET /5/on") >= 0) {
//                         //   Serial.println("GPIO 5 on");
//                         //   output5State = "on";
//                         //   digitalWrite(output5, HIGH);
//                         // } else if (header.indexOf("GET /5/off") >= 0) {
//                         //   Serial.println("GPIO 5 off");
//                         //   output5State = "off";
//                         //   digitalWrite(output5, LOW);
//                         // } else if (header.indexOf("GET /4/on") >= 0) {
//                         //   Serial.println("GPIO 4 on");
//                         //   output4State = "on";
//                         //   digitalWrite(output4, HIGH);
//                         // } else if (header.indexOf("GET /4/off") >= 0) {
//                         //   Serial.println("GPIO 4 off");
//                         //   output4State = "off";
//                         //   digitalWrite(output4, LOW);
//                         // }

//                         // Display the HTML web page
//                         client.println("<!DOCTYPE html><html>");
//                         client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
//                         client.println("<link rel=\"icon\" href=\"data:,\">");
//                         // CSS to style the on/off buttons
//                         // Feel free to change the background-color and font-size attributes to fit your preferences
//                         client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
//                         client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
//                         client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
//                         client.println(".button2 {background-color: #77878A;}</style></head>");

//                         // Web Page Heading
//                         client.println("<body><h1>ESP8266 Web Server</h1>");

//                         // // Display current state, and ON/OFF buttons for GPIO 5
//                         // client.println("<p>GPIO 5 - State " + output5State + "</p>");
//                         // // If the output5State is off, it displays the ON button
//                         // if (output5State == "off")
//                         // {
//                         //     client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
//                         // }
//                         // else
//                         // {
//                         //     client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
//                         // }

//                         // // Display current state, and ON/OFF buttons for GPIO 4
//                         // client.println("<p>GPIO 4 - State " + output4State + "</p>");
//                         // // If the output4State is off, it displays the ON button
//                         // if (output4State == "off")
//                         // {
//                         //     client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
//                         // }
//                         // else
//                         // {
//                         //     client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
//                         // }
//                         // client.println("</body></html>");

//                         // The HTTP response ends with another blank line
//                         client.println();
//                         // Break out of the while loop
//                         break;
//                     }
//                     else
//                     { // if you got a newline, then clear currentLine
//                         currentLine = "";
//                     }
//                 }
//                 else if (c != '\r')
//                 {                     // if you got anything else but a carriage return character,
//                     currentLine += c; // add it to the end of the currentLine
//                 }
//             }
//         }
//         // Clear the header variable
//         header = "";
//         // Close the connection
//         client.stop();
//         Serial.println("Client disconnected.");
//         Serial.println("");
//     }
// }

void LoopPot()
{
    potentiometerValue = analogRead(potPin);
}