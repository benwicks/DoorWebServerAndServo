/*
  Door Web Server And Servo
 
 A simple web server that triggers a servo to turn my door handle
 using an Arduino Wiznet Ethernet shield. 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Servo output attached to pin 6
 * Magnetic reed switch on pin 8

 created 6 Jan 2013
 by Benjamin L. Wicks 
*/

#include <SPI.h>
#include <Ethernet.h>
#include <Servo.h>

// Servo object
Servo myservo;

// MAC address and IP address for controller below.
// The MAC address is found printed on the bottom of newer Arduino Ethernet shields
// The IP address will be dependent on your local network:
byte mac[] = {  
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
IPAddress myIP(10,3,3,10);

// my server on the local network
byte demetriusServer[] = {
  10,3,3,100};

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

// Initialize the Ethernet client
EthernetClient eClient;

// Magnet switch
int MAG_PIN = 8;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, myIP);
  server.begin();
  Serial.print("arduino is at ");
  Serial.println(Ethernet.localIP());
  
  // set the reed switch as input
  pinMode(MAG_PIN, INPUT);
}


void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // store requested GET /<page> -- can be up to 14 characters long
    char str[15];
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    // read GET requested page into memory
    boolean haveReadGET = false;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (!haveReadGET) {
          if (c == '/') {
            // read string until space
            int i = 0;
            do {
              c = client.read();
              Serial.write(c);
              if (c == ' ') {
                haveReadGET = true;
                str[i] = '\0';
                break;
              }
              str[i] = c;
              i = i+1;
            } 
            while (i < 15);
          }
        }
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // What did they request?
          Serial.print("Requested: ");
          Serial.println(str);

          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connnection: close");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          if (strcmp(str,"stat") == 0) {
            // print the status of the door
            client.println("<head><title>Status</title></head>");
            client.println("<body>Door is ");
            if (digitalRead(MAG_PIN)) {
              client.println("shut.</body></html>");
            } else {
              client.println("open.</body></html>");
            }
          }
          else if (strcmp(str,"open") == 0) {
            // Open the door
            openDoor();
            client.println("<head><title>Open</title></head><body>");
            // record opened time on server
            if (eClient.connect(demetriusServer, 80)) {
              Serial.println("connected to DB on Demetrius.");
              eClient.println("GET /doorOpener/insertLast.php?atid=1 HTTP/1.0");
              eClient.println();
              client.print("<h1>Communicating with Demetrius</h1><p>");
              while (eClient.available()) {
                char c = eClient.read();
                client.print(c);
              }
              eClient.stop();
              client.println("</p>");
            } 
            else {
              client.println("<h1 style='color:red;'>Connection to Demetrius' DB failed.</h1>");
              eClient.stop();
            }
            client.println("</body></html>");

          } 
          else {
            // return this by default
            client.println("<head><title>Door</title></head>");
            client.println("</html>");
          }
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}

// turn the servo to turn the door handle
void openDoor() {
  myservo.attach(6);
  myservo.write(30);
  delay(1500);
  myservo.detach();
  delay(3000);
  myservo.attach(6);
  myservo.write(180);
  delay(700);
  myservo.detach();
}



