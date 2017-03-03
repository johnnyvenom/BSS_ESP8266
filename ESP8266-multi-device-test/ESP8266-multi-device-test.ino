/*---------------------------------------------------------------------------------------------

  This sketch is adapted from the example sketch described below.

  Open Sound Control (OSC) library for the ESP8266

  Example for receiving open sound control (OSC) bundles on the ESP8266
  Send integers '0' or '1' to the address "/led" to turn on/off the built-in LED of the esp8266.

  This example code is in the public domain.

  I added support for a static IP address.

  I also added the ability to set the LED with PWM; note that the built-in LED doesn't seem
  to support PWM, so it will be necessary to attach an external LED for that.

  Finally, I added support for the DRV2603 haptic motor driver chip. Testing at home I found
  that the motor would vibrate, but not as expected; specifically, from the datasheet I would
  expect that at 50% duty cycle the motor should be still (I added a deadzone to make sure),
  with maximum amplitude at 0 or 255. Instead I found that the motor had maximum amplitude at
  0, getting less and less until 255. The motor only seems to stop at 127 because of the hard
  coded deadzone.

  —Travis

  PS Sometimes I get an error uploading the sketch. Usually clears if I try again.

--------------------------------------------------------------------------------------------- */
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

//#define LEDON HIGH
//#define LEDOFF LOW
#define LEDON LOW
#define LEDOFF HIGH

char ssid[] = "bodysuit";          // your network SSID (name)
char pass[] = "bodysuit";                    // your network password

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
const IPAddress outIp(192,168,1,255);       // remote IP of your computer
const unsigned int outPort = 9999;          // remote port to receive OSC
const unsigned int localPort = 8888;        // local port to listen for UDP packets (here's where we send the packets)

OSCErrorCode error;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LEDOFF);

  Serial.begin(115200);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());

}

void led(OSCMessage &msg) 
{  
  if(msg.isInt(0))
  {
    int ledBlink = msg.getInt(0);
    OSCMessage echo("/led");
    echo.add(WiFi.localIP()[3]);
    echo.add(ledBlink);
    Udp.beginPacket(outIp, outPort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
    switch(ledBlink) 
    {
      case 2:
        // blink
        Serial.println("led 2");
        for(char i = 0; i<16; i++)
        {
          digitalWrite(LED_BUILTIN, LEDON);
          delay(50);
          digitalWrite(LED_BUILTIN, LEDOFF);
          delay(75);
        }
        break;
      case 3:
        Serial.println("led 3");
        //solid on
        digitalWrite(LED_BUILTIN, LEDON);
        delay(3000);
        digitalWrite(LED_BUILTIN, LEDOFF);
        break;
      case 1: digitalWrite(LED_BUILTIN, LEDON); break; // turn led on,  used for random
      case 0: digitalWrite(LED_BUILTIN, LEDOFF); break;// turn led off, used for random
      default:
        //no blink
        Serial.println("Warning: Out of range for /led method");
        break;
    }
  }

  else
  {
    Serial.println("Warning: Unsupported type for /led method");
  }
}

void loop() {
  OSCBundle bundle;
  int size = Udp.parsePacket();

  if (size > 0) {
    while (size--) {
      bundle.fill(Udp.read());
    }
    if (!bundle.hasError()) {
      bundle.dispatch("/led", led);
    } else {
      error = bundle.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }
}



