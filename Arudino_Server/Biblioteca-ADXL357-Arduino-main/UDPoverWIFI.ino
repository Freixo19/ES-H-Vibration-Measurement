#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <time.h>

char ssid[] = "Vodafone-3C8267"; //  your network SSID (name) 
char pass[] = "76F4H4J39XGHD73X";    // your network password (use for WPA, or use as key for WEP)
/*

  WiFi UDP Send and Receive String

 This sketch wait an UDP packet on localPort using a WiFi shield.

 When a packet is received an Acknowledge packet is sent to the client on port remotePort

 Circuit:

 * WiFi shield attached

 created 30 December 2012

 by dlf (Metodo2 srl)

 */

int status = WL_IDLE_STATUS;
int keyIndex = 0;            // your network key Index number (needed only for WEP)

unsigned int localPort = 2390;      // local port to listen on

char packetBuffer[255]; //buffer to hold incoming packet
char ReplyBuffer[255];       // a string to send back

WiFiUDP Udp;

void setup() {

  //Initialize serial and wait for port to open:

  Serial.begin(9600);

  while (!Serial) {

    ; // wait for serial port to connect. Needed for native USB port only

  }

  // check for the presence of the shield:

  if (WiFi.status() == WL_NO_SHIELD) {

    Serial.println("WiFi shield not present");

    // don't continue:

    while (true);

  }

  String fv = WiFi.firmwareVersion();

  if (fv != "1.5.0") {

    Serial.println("Please upgrade the firmware");

  }

  // attempt to connect to Wifi network:

  while (status != WL_CONNECTED) {

    Serial.print("Attempting to connect to SSID: ");

    Serial.println(ssid);

   

    status = WiFi.begin(ssid,pass);

    // wait 10 seconds for connection:

    delay(1000);

  }

  Serial.println("Connected to wifi");

  printWifiStatus();

  //Serial.println("\nStarting connection to server...");

  // if you get a connection, report back via serial:

  Udp.begin(localPort);
  srand(time(NULL));
}

void loop() {

  // if there's data available, read a packet
  int packetSize;
  while((packetSize = Udp.parsePacket()) == 0);

  if (packetSize) {

    /*Serial.print("Received packet of size ");

    Serial.println(packetSize);

    Serial.print("From ");

    IPAddress remoteIp = Udp.remoteIP();

    Serial.print(remoteIp);

    Serial.print(", port ");

    Serial.println(Udp.remotePort());
    */
    // read the packet into packetBufffer
    int len = Udp.read(packetBuffer, 255);

    if (len > 0) {

      packetBuffer[len] = 0;

    }


    float mcfloatface = rand() % 100 + 1;
    mcfloatface = mcfloatface + (rand() + 1) / (double)RAND_MAX;
    //Serial.print("Float: ");
    //Serial.println(mcfloatface);
    
    int val_int = (int) mcfloatface;   // compute the integer part of the float
    //Serial.print("val_int: ");
    //Serial.println(val_int);

    float val_float = ( fabs(mcfloatface) - abs(val_int)) * 1000;
    //Serial.print("val_float: ");
    //Serial.println(val_float);
    int val_fra = (int)val_float;
    //Serial.print("val_fra: ");
    //Serial.println(val_fra);

    sprintf (ReplyBuffer, "%d.%d", val_int, val_fra); //
    //Serial.print("String: ");
    //Serial.println(ReplyBuffer);

    // Serial.println(packetBuffer);
    char checksum = 0;

    for (int i = 0; i < sizeof(ReplyBuffer); i++) 
    {
      checksum ^= ReplyBuffer[i];
    }

    Serial.print("Checksum of ");
    Serial.print(ReplyBuffer);
    Serial.print(": ");
    Serial.println(checksum, HEX);

    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());

    Udp.write(ReplyBuffer);
    Udp.write(checksum);

    Udp.endPacket();

    

    // Serial.println("Contents:");
    // send a reply, to the IP address and port that sent us the packet we received


  }
}

void printWifiStatus() {

  // print the SSID of the network you're attached to:

  Serial.print("SSID: ");

  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:

  IPAddress ip = WiFi.localIP();

  Serial.print("IP Address: ");

  Serial.println(ip);

  // print the received signal strength:

  long rssi = WiFi.RSSI();

  Serial.print("signal strength (RSSI):");

  Serial.print(rssi);

  Serial.println(" dBm");
}