#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <time.h>

#include <Arduino.h>
#include <adxl357.h>
#include <math.h>

char ssid[] = "POCO X3 Pro";   //network SSID (name) 
char pass[] = "1234bruh"; //network password


int status = WL_IDLE_STATUS;
int keyIndex = 0;            

unsigned int localPort = 13325;  


double calib = 1.0 / (double)12490;  // calculate your own calibration constant
Adxl357 adxl357;
int value = 0;


char packetBuffer[255]; //buffer to hold incoming packet
char xBuffer[10];       // send x
char yBuffer[10];       // send y
char zBuffer[10];       // send z
char xSign;
char ySign;
char zSign;

WiFiUDP Udp;

void setup() {

  if (WiFi.status() == WL_NO_SHIELD) {

    while (true);

  }

  while (status != WL_CONNECTED) {

    status = WiFi.begin(ssid,pass);

    delay(1000);

  }

  //printWifiStatus();

  while (adxl357.init(ADXL357_DEF_ADD)) 
  {
    ;
  }

  adxl357.setAccelRange(ADXL357_FOUTY_G);

  adxl357.setPowerCTL(ADXL357_ALL_ON);

  adxl357.setCalibrationConstant(calib);

  Udp.begin(localPort);
}

double x, y, z;

void loop() {
  int packetSize;

  while((packetSize = Udp.parsePacket()) == 0);

  if (packetSize) {

    int len = Udp.read(packetBuffer, 255);

    if (len > 0) {

      packetBuffer[len] = 0;

    }
    if(strcmp(packetBuffer,"Hello Arduino!") == 0)
    {
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write("Im Here");
      Udp.endPacket();
    }
    else
    {
      char checksum = 0;
      
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      for(int j = 0; j < 64; j++)
      {
        if (adxl357.isDataReady()) {
          if (adxl357.getScaledAccelData(&x, &y, &z)) {
          }

          int x_int = (int)x;   // compute the integer part of the float

          float x_float = ( fabs(x) - abs(x_int)) * 100;

          int x_fra = (int)x_float;

          sprintf (xBuffer, "%d.%02d", x_int, x_fra); 

          if(x >=0)
          {
            xSign = 'P';
          }
          else
          {
            xSign = 'N';
          }
          
          int y_int = (int)y;   // compute the integer part of the float

          float y_float = ( fabs(y) - abs(y_int)) * 100;

          int y_fra = (int)y_float;

          sprintf (yBuffer, "%d.%02d", y_int, y_fra); //

          if(y >=0)
          {
            ySign = 'P';
          }
          else
          {
            ySign = 'N';
          }

          int z_int = (int)z;   // compute the integer part of the float

          float z_float = ( fabs(z) - abs(z_int)) * 100;

          int z_fra = (int)z_float;

          sprintf (zBuffer, "%d.%02d", z_int, z_fra); //

          if(z >=0)
          {
            zSign = 'P';
          }
          else
          {
            zSign = 'N';
          }


        }
        for (int i = 0; i < sizeof(xBuffer); i++) 
        {
          checksum ^= xBuffer[i];
        }
        checksum ^= xSign;

        for (int i = 0; i < sizeof(yBuffer); i++) 
        {
          checksum ^= yBuffer[i];
        }
        checksum ^= ySign;

        for (int i = 0; i < sizeof(zBuffer); i++) 
        {
          checksum ^= zBuffer[i];
        }
        checksum ^= zSign;
        
        Udp.write(xBuffer);
        Udp.write(xSign);
        Udp.write(yBuffer);
        Udp.write(ySign);
        Udp.write(zBuffer);
        Udp.write(zSign);
      }


      Udp.write(checksum);
      Udp.endPacket();
    }
  
  }
}
