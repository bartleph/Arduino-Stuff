/*
  Ping Example
 
 This example sends an ICMP ping every xxx milliseconds, sends the human-readable
 result over the serial port. 

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 
 created 30 Sep 2010
 by Blake Foster

 Modified 04 Feb 1018
 by Paul Bartlett
 
 * Modified by Paul Bartlett Feb 2018 for external Ping
 * 1) Change byte array to IPAddress for ip.  Line 28
 * 2) Add IPAddress subnet(xxx,xxx,xxx,xxx);  Line 30
 * 3) Add IPAddress gateway(xxx,xxx,xxx,xxx); Line 31
 * 4) Modify Ethernet.begin call.             Line 44
 */

#include <SPI.h>         
#include <Ethernet.h>
#include <ICMPPing.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // mac address for ethernet shield
IPAddress ip = (192,168,0,138); // ip address for ethernet shield
IPAddress pingAddr(188,125,80,144); // ip address to ping
IPAddress subnet(255,255,0,0);
IPAddress gateway(192,168,1,254);

SOCKET pingSocket = 0;

char buffer [256];
ICMPPing ping(pingSocket, (uint16_t)random(0, 255));

void setup() 
{
  // start Ethernet
/*
 * Was Ethernet.begin(mac, ip);
 */
  Ethernet.begin(mac, ip, subnet, gateway);

  
  Serial.begin(9600);
}

void loop()
{
  ICMPEchoReply echoReply = ping(pingAddr, 4);
  if (echoReply.status == SUCCESS)
  {
    sprintf(buffer,
            "Reply[%d] from: %d.%d.%d.%d: bytes=%d time=%ldms TTL=%d",
            echoReply.data.seq,
            echoReply.addr[0],
            echoReply.addr[1],
            echoReply.addr[2],
            echoReply.addr[3],
            REQ_DATASIZE,
            millis() - echoReply.data.time,
            echoReply.ttl);
  }
  else
  {
    sprintf(buffer, "Echo request failed; %d", echoReply.status);
  }
  Serial.println(buffer);
  delay(5000);
}










