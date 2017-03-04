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

#define LEDON LOW
#define LEDOFF HIGH

char ssid[] = "bodysuit";          // your network SSID (name)
char pass[] = "bodysuit";                    // your network password

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
IPAddress outIp(192,168,1,255);       // remote IP of your computer
IPAddress myIP;
unsigned int outPort = 55000;          // remote port to receive OSC
const unsigned int localPort = 8888;        // local port to listen for UDP packets (here's where we send the packets)

OSCErrorCode error;
bool networkDiscoveryMode = 0;
bool sensorActive = 0;
unsigned long lastTime = -1;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LEDOFF);

  Serial.begin(115200);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(myIP = WiFi.localIP());
  outPort+=myIP[3];

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.print("Output port: ");
  Serial.println(outPort);

}

void led(OSCMessage &msg) 
{  
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();
  if(msg.isInt(0))
  {
    int ledBlink = msg.getInt(0);
    Serial.print("/led "); Serial.println(ledBlink);
    switch(ledBlink) 
    {
      case 2:
        // blink
        for(char i = 0; i<16; i++)
        {
          digitalWrite(LED_BUILTIN, LEDON);
          delay(50);
          digitalWrite(LED_BUILTIN, LEDOFF);
          delay(75);
        }
        break;
      case 3:
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

void networkDiscovery(OSCMessage &msg)
{
  // send network/discovery masterIP to start discovery mode
  // this sets the master IP and enables the discovery mode routine in the main loop
  // send network/discovery 0 to end discovery mode
  // e.g. after the master has recorded all of the local device IP addresses
  Serial.print("/networkDiscovery "); Serial.println(msg.getInt(0));
  if(msg.isInt(0))
  {
    int masterIP = msg.getInt(0);
    if (masterIP == 0) networkDiscoveryMode = 0;
    else
    {
      outIp[3] = masterIP;
      Serial.print("New master IP: "); Serial.println(masterIP);
      networkDiscoveryMode = 1;
    }
  }
}

void sensor(OSCMessage &msg)
{
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();
  if (msg.isInt(0))
  {
    sensorActive = msg.getInt(0) > 0;
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
      bundle.dispatch("/network/discovery", networkDiscovery);
      bundle.dispatch("/sensor", sensor);
    } else {
      error = bundle.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }

  if (sensorActive)
  {
    unsigned long timer = millis();
    Serial.print("Time is: "); Serial.println(timer);
    if (timer != lastTime)
    {
      lastTime = timer;
      OSCMessage msg("/sensor/timer");
      msg.add((int32_t)lastTime);
      Udp.beginPacket(outIp, outPort);
      msg.send(Udp);
      Udp.endPacket();
      msg.empty();
    }
  }

  if (networkDiscoveryMode)
  {
    // in discovery mode the device will send its IP address to the master on every loop
    OSCMessage msg("/network/suitIP");
    msg.add((int32_t)myIP[3]);
    Udp.beginPacket(outIp, 55256);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
    delay(100);
  }
}



