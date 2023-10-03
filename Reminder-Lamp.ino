/**
 * Logic using as few new components as possible. The board has a lamp and a reset button.
 *
 * Turn the lamp on when it is time to feed our fishes.
 * Fishes are to be fed once a day some time between 07:00 and 20:00
 *
 * User resets program by pressing the button after feeding fishes.
 * Lamp starts turned off
 * If the program has ran more than 11 hours and time is after 07:00, turn the lamp on 
 *
 * Weakness: if the power is lost, the program will reset and lamp will be turned off without fishes being fed
 * We'll build this anyway and make smarter logic later.
 */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK "your-password"
#endif

const char* ssid = STASSID;  // your network SSID (name)
const char* pass = STAPSK;   // your network password

void setup() {

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

}

void loop() {

  sendNTPpacket(timeServerIP);  // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);


  // TODO: We want to turn the lamp OFF if the program has run

  // This bit will make the lamp turn on and off repeatedly:
  if(LOW == digitalRead(LED_BUILTIN)) {
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED off (LOW is the voltage level)
  }

  delay(1000);  

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
