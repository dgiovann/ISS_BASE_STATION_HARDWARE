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
#define RED 0xF800
#define BLUE 0x1F
#define GREEN 0x7e0

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

//Servo pins
int pwmBase = 11;
int pwmArm = 13;

//Global variables for azimuth and elevation
int azi = 0;
int ele = 0;
int pointing = 0; //Either true or false;

//RGB Matrix
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);
char matrix_str[] = "ISS OVERHEAD";
int textX   = matrix.width();
int textMin = sizeof(matrix_str) * -12;
long hue = 0;

void drawText(){
  // Clear background
  matrix.setCursor(1, 1);
  matrix.setTextSize(1);
  matrix.setTextWrap(true);
  matrix.fillScreen(0);

  if(pointing == 1) {
    matrix.setTextColor(RED);
    matrix.print(matrix_str);
  }
  if(pointing == 0) {
    matrix.setTextColor(GREEN);
    matrix.print("SEARCHING...");
  }

  // Move text left (w/wrap), increase hue
  if((--textX) < textMin) textX = matrix.width();
  hue += 7;

}

void getHTTP(){
  //String readString = String(100);

  // listen for incoming clients
  EthernetClient client = server.available();

  int i = 0;
  char readString[255];

  if (client) {
    Serial.println("new client");

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // c is the character read. Parse GET request here
        readString[i++] = c;
        if(c == '\n'){
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connnection: close");
          client.println();
          client.println("0K");
          break;
        }
        if (i > 254) break;
      }
    }
    readString[i] = '\0';

    // String format should be:                                                 
    // GET http://192.168.1.177/?azimuth,elevation,pointing HTTPstuff\n         
    char* parsed_token = strtok(readString, "?"); // Tokenize up to ?           

    // Parse azimuth                                                            
    parsed_token = strtok(NULL, ",");
    if (!parsed_token) return;    
    int azimuth = atoi(parsed_token);                        
    Serial.print("Azimuth: "); 
    Serial.println(azimuth);   

    // Parse elevation                                                          
    parsed_token = strtok(NULL, ",");
    if (!parsed_token) return;                                             
    int elevation = atoi(parsed_token);                      
    Serial.print("Elevation: "); 
    Serial.println(elevation);   

    // Parse pointing                                                           
    parsed_token = strtok(NULL, " ");
    if (!parsed_token) return;                                             
    pointing = atoi(parsed_token);  
    Serial.print("Pointing: "); 
    Serial.println(pointing);

    //Makes a call to the LED Matrix to update based on pointer value
    drawText();
    //Increments servos by the (azimuth - azi, elevation - ele);
    point(azimuth,elevation);

  }

  // give the web browser time to receive the data
  delay(5);
  // close the connection:
  client.stop();
}

//Library command to move a continous rotation servo by a specified angle
void moveServo(int angle, int pin) {
  int gr = 1; // 1:1 Gear Ratio
  if (pin == pwmBase) {
    gr = 2.411; //Gear ratio for a Large to Medium Gear using Knex
  }
  int cnt = 0;
  int numPulses = abs(floor(angle * gr * 2.0 / 3.0));

  while (cnt != numPulses) {

    if (cnt < numPulses){

      delayMicroseconds(20000);
      digitalWrite(pin, HIGH);
      if (angle > 0){
        delayMicroseconds(1400); //CCW at mid-speed
      }
      else if (angle == 0) {
        delayMicroseconds(1500);
      }
      else{
        delayMicroseconds(1600); //CW at mid-speed
      }
      digitalWrite(pin, LOW);
      //hasMoved = true;
      cnt++;

    }
  }

  delay(1);

}

// Increments servo's by the absolute orientation given.
void point(int a, int e) {
  if( a != 0 && e != 0 ) {
    moveServo(a - azi, pwmBase);
    moveServo(e - ele, pwmArm);
    azi = a;
    ele = e;
  }
}

void setup(){
  pinMode(pwmBase, OUTPUT); 
  pinMode(pwmArm, OUTPUT); 
  digitalWrite(pwmBase, LOW);
  digitalWrite(pwmArm, LOW);
  moveServo(0, pwmArm);
  moveServo(0, pwmBase);
  //Open serial communications and wait for port to open:
  Serial.begin(9600);

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();

  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  matrix.begin();
  drawText();

}

void loop(){

  moveServo(180, pwmArm);
  moveServo(90, pwmBase);
  delay(2000);
  moveServo(-180, pwmArm);
  moveServo(-90, pwmBase);
  delay(2000);
  //    getHTTP();
}

