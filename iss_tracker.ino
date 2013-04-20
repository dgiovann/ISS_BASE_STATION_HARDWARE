#include <Servo.h>
#include <SPI.h>
#include <Ethernet.h>
#include <string.h>
#include <stdlib.h>
#include <Adafruit_GFX.h>
#include <RGBmatrixPanel.h>
#include <stdint.h>

#define CLK 8 
#define LAT A3
#define OE 9
#define A A0
#define B A1
#define C A2
#define RED 0xf800

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,177);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

//Servos
Servo azimuth_servo;
Servo elevation_servo;

float azimuth = 0.0f;
float elevation = 0.0f;
char pointing = 0;

//RGB Matrix
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, true);
char matrix_str[] = "ISS OVERHEAD";
int textX   = matrix.width();
int textMin = sizeof(matrix_str) * -12;
long hue = 0;

void getHTTP(){
  //String readString = String(100);
  char* readString;
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // c is the character read. Parse GET request here
        readString += c;
        if(c == '\n'){
            break;
        }
      }
    }
    //String format should be ?azimuth,elevation,boolean\n
    char* parsed_token = strtok(readString, "?,\n"); //Parse azimuth
    azimuth = atof(parsed_token);
    parsed_token = strtok(NULL, "?,\n"); //Parse elevation
    elevation = atof(parsed_token);
    parsed_token = strtok(NULL, "?,\n"); //Parse boolean
    pointing = atoi(parsed_token);
  }
    // give the web browser time to receive the data
    delay(5);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
}

//Set the servos to rotate to the current azimuth and elevation
void setServos(){
    
}

void drawText(){
  // Clear background
  matrix.fillScreen(0);
  
  // Draw scrolling text
  matrix.setTextColor(matrix.ColorHSV(hue, 255, 255, true));
  matrix.setCursor(textX, 1);
  matrix.print(matrix_str);

  // Move text left (w/wrap), increase hue
  if((--textX) < textMin) textX = matrix.width();
  hue += 7;
  
  // Update display
  matrix.swapBuffers(false);
}

void setup(){
  //Open serial communications and wait for port to open:
  Serial.begin(9600);

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  azimuth_servo.attach(9);
  elevation_servo.attach(10);
 
  matrix.begin();
  matrix.setTextWrap(false); // Allow text to run off right edge
  matrix.setTextSize(2);
}

void loop(){
    getHTTP();
    setServos();
    drawText();
}
