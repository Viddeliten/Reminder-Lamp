/**
 * Logic using as few new components as possible. The board has a lamp and a reset button.
 *
 * Turn the lamp on when it is time to feed our fishes.
 * Fishes are to be fed once a day some time between 07:00 and 20:00
 *
 * User resets program by pressing the button after feeding fishes.
 * Lamp starts turned off
 * Wait until 07:00 and then turn lamp on!
 *
 * Weakness: if the power is lost, the program will reset and lamp will be turned off without fishes being fed
 * We'll build this anyway and make smarter logic later.
 *
 * We'll use the NTPClient example code as a base
 */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "config.h"

const char* ssid = STASSID;  // your network SSID (name)
const char* pass = STAPSK;   // your network password

unsigned int localPort = 2390;  // local port to listen for UDP packets

/* Don't hardwire the IP address or we won't get the benefits of the pool.
    Lookup the IP address for the host name instead */
// IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP;  // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";

const int NTP_PACKET_SIZE = 48;  // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[NTP_PACKET_SIZE];  // buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

// We need to know when the program started
unsigned long start_time;

// We want a flag for if the lamp should be on or not
bool lampon;

void setup() {

  // Sets the data rate in bits per second (baud) for serial data transmission
  Serial.begin(115200);

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Connecting to a WiFi network
  connectToWifi();

  // Get time
  start_time = getNTPTime();
}

void loop() {

  // Get current time
  unsigned long current_time = getNTPTime();

  // Print time
  // printTime(NTPtoUnixTime(current_time));

  // If the program has ran more than 11 hours and time is after 07:00, turn the lamp on
  unsigned long running_time = current_time - start_time;

  Serial.print("program has been running for:");
  Serial.println(running_time);

  int hours = running_time / (11*60*60);

  if(running_time >= 11*60*60) {
    Serial.print("Running time is longer than 11 hours: ");
  } else {
    Serial.print("Running time is SHORTR than 11 hours: ");
  }
  Serial.println(hours);


  // This bit will make the lamp turn on and off repeatedly:
  if(LOW == digitalRead(LED_BUILTIN)) {
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED off (LOW is the voltage level)
  }

  delay(10000);  

}

// Connecting to a WiFi network
void connectToWifi(void) {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
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
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address) {
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;  // LI, Version, Mode
  packetBuffer[1] = 0;           // Stratum, or type of clock
  packetBuffer[2] = 6;           // Polling Interval
  packetBuffer[3] = 0xEC;        // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123);  // NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

// Get current time over internet
unsigned long getNTPTime(void)
{
  int packet_length;
  
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);

  sendNTPpacket(timeServerIP);  // send an NTP packet to a time server

  // Stop untill we get a packet back (TODO: add a fail state in case it never happens)
  do {
    Serial.println("looking for packet");
    packet_length = udp.parsePacket();
  
    // wait to see if a reply is available
    if (!packet_length) {
      Serial.println("No packet yet");
      delay(1000);
    } 
  } while(!packet_length);

  Serial.print("packet received, length=");
  Serial.println(packet_length);
  // We've received a packet, read the data from it
  udp.read(packetBuffer, NTP_PACKET_SIZE);  // read the packet into the buffer

  // the timestamp starts at byte 40 of the received packet and is four bytes,
  //  or two words, long. First, esxtract the two words:

  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  unsigned long secsSince1900 = highWord << 16 | lowWord;
  Serial.print("Seconds since Jan 1 1900 = ");
  Serial.println(secsSince1900);

  return secsSince1900;
}

// Convert NTP time into unix timestamp
unsigned long NTPtoUnixTime(int secsSince1900) {
  Serial.print("Unix time = ");
  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years:
  unsigned long epoch = secsSince1900 - seventyYears;
  // print Unix time:
  Serial.println(epoch);

  return epoch;
}

void printTime(unsigned long UnixTime) {

    // print the hour, minute and second:
  Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
  Serial.print((UnixTime % 86400L) / 3600);  // print the hour (86400 equals secs per day)
  Serial.print(':');
  if (((UnixTime % 3600) / 60) < 10) {
    // In the first 10 minutes of each hour, we'll want a leading '0'
    Serial.print('0');
  }
  Serial.print((UnixTime % 3600) / 60);  // print the minute (3600 equals secs per minute)
  Serial.print(':');
  if ((UnixTime % 60) < 10) {
    // In the first 10 seconds of each minute, we'll want a leading '0'
    Serial.print('0');
  }
  Serial.println(UnixTime % 60);  // print the second
}
