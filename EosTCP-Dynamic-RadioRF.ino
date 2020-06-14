/*
   DEVELOPER NOTES
   Copyright Cameron Affleck 2020

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.


   Function:
   To allow an ETC Eos Family Console to be remotely trigged by a RF remote over the network.
   A web form allows the user to amend basic IP information, such as Local IP, Subnet Mask,
   and Remote IP address (Eos) range. The defined range allows the sketch to use ARP to find
   a console with the correct IP address and open port number. The Port number is allocated
   in the sketch. The MAC address is allocated in the sketch.
   This sketch fires TCP OSC messages to trigger Macros on the Eos console. The sketch constantly
   pings the console to confirm the connection is still active. If a reply is received, a LED flashes.
   If a reply has not been received, the sketch re-pings the console, and if a reply is still not
   received, the sketch disconnects from the console.
   RF Functionality is only enabled when IP address is selected.

   Parts:
   1x Arduino Uno
   1x Arduino Ethernet Shield
   1x RF Solutions ZPT-8RS RF Module

   PINS 4, 10-13 CANNOT be used due to Ethernet!
*/

/*================ LIBRARIES =================*/
#include <SPI.h>
#include <Ethernet.h>
#include <TextFinder.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>
#include <osc.h>

/*=============== DEFINITIONS ================*/
#define F(string_literal)(reinterpret_cast < const __FlashStringHelper * > (PSTR(string_literal)))

//Radio Definitions
#define RX1 7
#define RX2 6
#define RX3 5
#define RX4 3

int StateChange = 0;
byte Inputs18 = 0b00000000;

byte myMac[6] = {0xA8, 0x61, 0x0A, 0xAE, 0x14, 0x7A};
byte myIP[4] = {10, 101, 75, 101};
byte myNM[4] = {255, 255, 0, 0};
byte myGW[4] = {10, 101, 75, 1};
byte RemIP[4] = {10, 101, 100, 0};
int RemPort = 3032; // remote port to transmit to

int timeoutValue = 500; //connection timeout value in milliseconds (default 500)
int RemIPmin = 1; // lowest 4th octet value in range
int RemIPmax = 254; // highest 4th octet value in range
EthernetServer webServer(80);
EthernetClient osc;

char buffer[120];

const char htmlx0[] PROGMEM = "<html><title>Eos Remote Network Setup</title><body marginwidth=\"0\" marginheight=\"0\" ";
const char htmlx1[] PROGMEM = "leftmargin=\"0\" style=\"margin: 0; padding: 0;\"><table bgcolor=\"#999999\" border";
const char htmlx2[] PROGMEM = "=\"0\" width=\"100%\" cellpadding=\"1\" style=\"font-family:Verdana;color:#fff";
const char htmlx3[] PROGMEM = "fff;font-size:12px;\"><tr><td>&nbsp Eos Remote Network Setup</td></tr></table><br>";
PGM_P const string_table0[] PROGMEM = {
  htmlx0,
  htmlx1,
  htmlx2,
  htmlx3
};

const char htmla0[] PROGMEM = "<script>function hex2num (s_hex) {eval(\"var n_num=0X\" + s_hex);return n_num;}";
const char htmla1[] PROGMEM = "</script><table style=\"margin-left:10; width:410\"><form><input type=\"hidden\" name=\"SBM\" value=\"1\">";
PGM_P const string_table1[] PROGMEM = {
  htmla0,
  htmla1
};

const char htmlb0[] PROGMEM = "<input id=\"T2\" type=\"hidden\" name=\"DT1\"><input id=\"T4\" type=\"hidden\" name=\"DT2";
const char htmlb1[] PROGMEM = "\"><input id=\"T6\" type=\"hidden\" name=\"DT3\"><input id=\"T8\" type=\"hidden\" name=\"DT4";
const char htmlb2[] PROGMEM = "\"><input id=\"T10\" type=\"hidden\" name=\"DT5\"><input id=\"T12\" type=\"hidden\" name=\"D";
const char htmlb3[] PROGMEM = "T6\"></td></tr>";
const char htmlb4[] PROGMEM = "<tr><td>Device IP:</td><td><input type=\"number\" min=\"1\" max=\"254\" size=\"3\" maxlength=\"3\" name=\"DT7\" value=\"";
const char htmlb5[] PROGMEM = "\">.<input type=\"number\" min=\"1\" max=\"254\" size=\"3\" maxlength=\"3\" name=\"DT8\" value=\"";
const char htmlb6[] PROGMEM = "\">.<input type=\"number\" min=\"1\" max=\"254\" size=\"3\" maxlength=\"3\" name=\"DT9\" value=\"";
const char htmlb7[] PROGMEM = "\">.<input type=\"number\" min=\"1\" max=\"254\" size=\"3\" maxlength=\"3\" name=\"DT10\" value=\"";
PGM_P const string_table2[] PROGMEM = {
  htmlb0,
  htmlb1,
  htmlb2,
  htmlb3,
  htmlb4,
  htmlb5,
  htmlb6,
  htmlb7
};

const char htmlc0[] PROGMEM = "\"></td></tr><tr><td>Net Mask:</td><td><input type=\"number\" min=\"0\" max=\"255\" size=\"3\" maxlength=\"3\" name=\"DT11\" value=\"";
const char htmlc1[] PROGMEM = "\">.<input type=\"number\" min=\"0\" max=\"255\" size=\"3\" maxlength=\"3\" name=\"DT12\" value=\"";
const char htmlc2[] PROGMEM = "\">.<input type=\"number\" min=\"0\" max=\"255\" size=\"3\" maxlength=\"3\" name=\"DT13\" value=\"";
const char htmlc3[] PROGMEM = "\">.<input type=\"number\" min=\"0\" max=\"255\" size=\"3\" maxlength=\"3\" name=\"DT14\" value=\"";
PGM_P const string_table3[] PROGMEM = {
  htmlc0,
  htmlc1,
  htmlc2,
  htmlc3
};

const char htmld0[] PROGMEM = "\"></td></tr><tr><td>Gateway:</td><td><input type=\"number\" min=\"1\" max=\"254\" size=\"3\" maxlength=\"3\" name=\"DT15\" value=\"";
const char htmld1[] PROGMEM = "\">.<input type=\"number\" min=\"1\" max=\"254\" size=\"3\" maxlength=\"3\" name=\"DT16\" value=\"";
const char htmld2[] PROGMEM = "\">.<input type=\"number\" min=\"1\" max=\"254\" size=\"3\" maxlength=\"3\" name=\"DT17\" value=\"";
const char htmld3[] PROGMEM = "\">.<input type=\"number\" min=\"1\" max=\"254\" size=\"3\" maxlength=\"3\" name=\"DT18\" value=\"";
PGM_P const string_table4[] PROGMEM = {
  htmld0,
  htmld1,
  htmld2,
  htmld3
};

const char htmle0[] PROGMEM = "\"></td></tr><tr><td>Eos IP:</td><td><input type=\"number\" min=\"1\" max=\"254\" size=\"3\" maxlength=\"3\" name=\"DT19\" value=\"";
const char htmle1[] PROGMEM = "\">.<input type=\"number\" min=\"1\" max=\"254\" size=\"3\" maxlength=\"3\" name=\"DT20\" value=\"";
const char htmle2[] PROGMEM = "\">.<input type=\"number\" min=\"1\" max=\"254\" size=\"3\" maxlength=\"3\" name=\"DT21\" value=\"";
const char htmle3[] PROGMEM = "\">.<input type=\"number\" min=\"1\" max=\"254\" size=\"3\" maxlength=\"3\" name=\"DT22\" value=\"";
const char htmle4[] PROGMEM = "\" > - <input type=\"number\" min=\"1\" max=\"254\" size=\"3\" maxlength=\"3\" name=\"DT23\" value=\"";
PGM_P const string_table5[] PROGMEM = {
  htmle0,
  htmle1,
  htmle2,
  htmle3,
  htmle4
};

const char htmlf0[] PROGMEM = "\"></td></tr><tr><td><br></td></tr><tr><td>Button 1 (Red):</td><td>";
const char htmlf1[] PROGMEM = "<input type=\"number\" size=\"4\" min=\"1\" max=\"255\" maxlength=\"3\" name=\"DT24\" value=\"";
const char htmlf2[] PROGMEM = "\">&emsp;(001-255)</td></tr><tr><td>Button 2 (Yellow):</td>";
const char htmlf3[] PROGMEM = "<td><input type=\"number\" size=\"4\" min=\"1\" max=\"255\" maxlength=\"3\" name=\"DT25\" value=\"";
const char htmlf4[] PROGMEM = "\">&emsp;(001-255)</td></tr><tr><td>Button 3 (Blue):</td>";
const char htmlf5[] PROGMEM = "<td><input type=\"number\" size=\"4\" min=\"1\" max=\"255\" maxlength=\"3\" name=\"DT26\" value=\"";
const char htmlf6[] PROGMEM = "\">&emsp;(001-255)</td></tr><tr><td>Button 4 (Green):</td>";
const char htmlf7[] PROGMEM = "<td><input type=\"number\" size=\"4\" min=\"1\" max=\"255\" maxlength=\"3\" name=\"DT27\" value=\"";
const char htmlf8[] PROGMEM = "\">&emsp;(001-255)</td></tr>";
PGM_P const string_table6[] PROGMEM = {
  htmlf0,
  htmlf1,
  htmlf2,
  htmlf3,
  htmlf4,
  htmlf5,
  htmlf6,
  htmlf7,
  htmlf8
};

const char htmlg0[] PROGMEM = "<tr><td colspan=\"2\"><br>Please reload webpage after clicking update!</td></tr>";
const char htmlg1[] PROGMEM = "<tr><td><input id=\"button1\"type=\"submit\" value=\"UPDATE\" ";
const char htmlg2[] PROGMEM = "></td></tr></form></table></body></html>";
PGM_P const string_table7[] PROGMEM = {
  htmlg0,
  htmlg1,
  htmlg2
};

const byte ID = 0x92;

//OSC Definitions
#define SUBSCRIBE 1
#define UNSUBSCRIBE 0
unsigned long pingLast = millis();
unsigned long pingInt = 3000;
unsigned long lastMessageRXTime = 0;
unsigned long lastPingRXTime = 0;
bool timeoutPingSent = false;
#define PING_AFTER_IDLE_INTERVAL 3500
#define TIMEOUT_AFTER_IDLE_INTERVAL 4000
char pingValue[24] = "RX_POE_01";
int button1 = 1;
int button2 = 2;
int button3 = 3;
int button4 = 4;
char oscMessage[64];
int16_t oscMessageLength;
int32_t value = 1;
bool subscribed = false;
bool filtered = false;
bool printWebData = true;
const int MAX_LEN = 20;
const byte numChars = 100;
char receivedChars[numChars];
boolean newData = false;

//LED Definitions
#define LINK 9

/*================== SETUP ===================*/
void setup() {
  delay(50);
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  pinMode(LINK, OUTPUT);
  digitalWrite(LINK, HIGH);
  ShieldSetup();
  radioSetup();
  webServer.begin();
}

/*=============== SETUP EVENTS ===============*/
void ShieldSetup() {
  int idcheck = EEPROM.read(0);

  if (idcheck != ID) {}
  if (idcheck == ID) {
    for (int i = 0; i < 4; i++) {
      myIP[i] = EEPROM.read(i + 7);
    }
    for (int i = 0; i < 4; i++) {
      myNM[i] = EEPROM.read(i + 11);
    }
    for (int i = 0; i < 4; i++) {
      myGW[i] = EEPROM.read(i + 15);
    }
    for (int i = 0; i < 4; i++) {
      RemIP[i] = EEPROM.read(i + 19);
    }
    RemIPmin = EEPROM.read(22);
    RemIPmax = EEPROM.read(23);
    button1 = EEPROM.read(24);
    button2 = EEPROM.read(25);
    button3 = EEPROM.read(26);
    button4 = EEPROM.read(27);
  }
  Ethernet.begin(myMac, myIP, myGW, myGW, myNM);
  Ethernet.setRetransmissionCount(1);
}

void radioSetup() {
  pinMode(RX1, INPUT);
  pinMode(RX2, INPUT);
  pinMode(RX3, INPUT);
  pinMode(RX4, INPUT);
}

/*=================== LOOP ===================*/
void loop() {
  webForm();
  EosConnect();
}

/*=============== LOOP EVENTS ================*/

//Eos Events
void EosConnect() {
  if (!osc.connected()) {
    int RemIPmax1;
    RemIPmax1 = RemIPmax + 1;
    RemIP[3] = constrain(RemIP[3], RemIPmin, RemIPmax1);
    osc.setConnectionTimeout(timeoutValue);
    osc.connect(RemIP, RemPort);
    digitalWrite(LINK, LOW);
    RemIP[3]++;
    if (RemIP[3] == RemIPmax1) {
      RemIP[3] = RemIPmin;
    }
    subscribed = false;
    filtered = false;
  } else if (osc.connected()) {
    issueEosFilters();
    issueEosSubscribes();
    radioRX();
    issueEosPings();
    eosParse();
    if (lastMessageRXTime > 0) {
      unsigned long diff = millis() - lastMessageRXTime;
      if (diff > TIMEOUT_AFTER_IDLE_INTERVAL) {
        osc.stop();
        lastMessageRXTime = 0;
        timeoutPingSent = false;
      }
      if (!timeoutPingSent && diff > PING_AFTER_IDLE_INTERVAL) {
        oscPing(pingValue);
        timeoutPingSent = true;
      }
    }
    if (lastPingRXTime > 0) {
      unsigned long diff = millis() - lastPingRXTime;
      if (diff > 50) {
        digitalWrite(LINK, LOW);
        lastPingRXTime = 0;
      }
    }
  }
}

void issueEosSubscribes() {
  if (osc.connected() && subscribed == false) {
    oscSubscribe(SUBSCRIBE);
    subscribed = true;
  }
}

void issueEosFilters() {
  if (osc.connected() && filtered == false) {
    oscFilter("/eos/out/ping");
    filtered = true;
  }
}

void issueEosPings() {
  while ((millis() - pingLast) >= pingInt) {
    if (osc.connected()) {
      oscPing(pingValue);
    }
    pingLast = millis();
  }
}


void eosParse() {
recvWithEndMarker();
showNewData();
}

void recvWithEndMarker(){
  static byte ndx = 0;
  char endMarker = '\0';
  char rc;

  while (osc.available() && newData == false){
    rc = osc.read();
    if(rc !=endMarker){
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx>=numChars){
        ndx = numChars - 1;
      }
    } else {
      receivedChars[ndx] = '\0';
      ndx = 0;
      newData = true;
    } 
  }
 }

void showNewData() {
  if(newData == true){
    if(strlen(receivedChars)>0){
    lastMessageRXTime = millis();
    timeoutPingSent = false;
    }
    if(strstr(receivedChars, "/eos/out/ping") >0){
    lastPingRXTime = millis();
    digitalWrite(LINK, HIGH);
    }
    newData = false;
  }
}

//Radio Events
void radioRX() {
  StateChange = bitRead(Inputs18, 0);
  if ((digitalRead(RX1)) == HIGH) {
    if (StateChange == 0) {
      oscMacro(button1);
      bitSet(Inputs18, 0);
    }
  } else if (StateChange == 1) {
    bitClear(Inputs18, 0);
  }
  StateChange = bitRead(Inputs18, 1);
  if ((digitalRead(RX2)) == HIGH) {
    if (StateChange == 0) {
      oscMacro(button2);
      bitSet(Inputs18, 1);
    }
  } else if (StateChange == 1) {
    bitClear(Inputs18, 1);
  }
  StateChange = bitRead(Inputs18, 2);
  if ((digitalRead(RX3)) == HIGH) {
    if (StateChange == 0) {
      oscMacro(button3);
      bitSet(Inputs18, 2);
    }
  } else if (StateChange == 1) {
    bitClear(Inputs18, 2);
  }
  StateChange = bitRead(Inputs18, 3);
  if ((digitalRead(RX4)) == HIGH) {
    if (StateChange == 0) {
      oscMacro(button4);
      bitSet(Inputs18, 3);
    }
  } else if (StateChange == 1) {
    bitClear(Inputs18, 3);
  }
}

//OSC Events
void oscSubscribe(int32_t value) {
  char subscribe[64] = "/eos/subscribe";
  oscMessageLength = oscSend(oscMessage, subscribe, value, TCP10);
  osc.write(oscMessage, oscMessageLength);
}

void oscSubscribe(char * subscribeParam, int32_t value) {
  char subscribe[64] = "/eos/subscribe/param/";
  strcat(subscribe, subscribeParam);
  oscMessageLength = oscSend(oscMessage, subscribe, value, TCP10);
  osc.write(oscMessage, oscMessageLength);
}

void oscFilter(char * filterParam){
  char filter[64] = "/eos/filter/add=";
  strcat(filter, filterParam);
  oscMessageLength = oscSend(oscMessage, filter, TCP10);
  osc.write(oscMessage, oscMessageLength);
}

void oscPing(char * pingVal){
  char ping[64] = "/eos/ping/";
  strcat(ping, pingVal);
  oscMessageLength = oscSend(oscMessage, ping, TCP10);
  osc.write(oscMessage, oscMessageLength);
}

void oscMacro(int macroTrigger){
  char macroT[12];
  itoa(macroTrigger, macroT,10);
  char macro[64] = "/eos/macro/";
  char fire[12] = "/fire";
  strcat(macro, macroT);
  strcat(macro, fire);
  oscMessageLength = oscSend(oscMessage, macro, TCP10);
  osc.write(oscMessage, oscMessageLength);
}

//Web Events
void( * resetFunc)(void) = 0; //arduino reboot

void webForm() {
  EthernetClient client = webServer.available();
  if (client) {
    TextFinder finder(client);
    while (client.connected()) {
      if (client.available()) {
        if (finder.find("GET /")) {
          if (finder.findUntil("SBM", "\n\r")) {
            byte SET = finder.getValue();
            while (finder.findUntil("DT", "\n\r")) {
              int val = finder.getValue();
              if (val >= 7 && val <= 10) {
                myIP[val - 7] = finder.getValue();
              }
              if (val >= 11 && val <= 14) {
                myNM[val - 11] = finder.getValue();
              }
              if (val >= 15 && val <= 18) {
                myGW[val - 15] = finder.getValue();
              }
              if (val >= 19 && val <= 21) {
                RemIP[val - 19] = finder.getValue();
              }
              if (val >= 22 && val <= 22) {
                RemIPmin = finder.getValue();
              }
              if (val >= 23 && val <= 23) {
                RemIPmax = finder.getValue();
              }
              if(val == 24){
                button1 = finder.getValue();
              }
              if(val == 25){
                button2 = finder.getValue();
              }
              if(val == 26){
                button3 = finder.getValue();
              }
              if(val == 27){
                button4 = finder.getValue();
              }                                          
            }
            for (int i = 0; i < 4; i++) {
              EEPROM.write(i + 7, myIP[i]);
            }
            for (int i = 0; i < 4; i++) {
              EEPROM.write(i + 11, myNM[i]);
            }
            for (int i = 0; i < 4; i++) {
              EEPROM.write(i + 15, myGW[i]);
            }
            for (int i = 0; i < 3; i++) {
              EEPROM.write(i + 19, RemIP[i]);
            }
            EEPROM.write(22, RemIPmin);
            EEPROM.write(23, RemIPmax);
            EEPROM.write(24, button1);
            EEPROM.write(25, button2);
            EEPROM.write(26, button3);
            EEPROM.write(27, button4);
            EEPROM.write(0, 0x92);
            delay(5);
            resetFunc(); //reset arduino after save to EEPROM
          }
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          for (int i = 0; i < 4; i++) {
            strcpy_P(buffer, (char * ) pgm_read_word( & (string_table0[i])));
            client.print(buffer);
          }
          for (int i = 0; i < 2; i++) {
            strcpy_P(buffer, (char * ) pgm_read_word( & (string_table1[i])));
            client.print(buffer);
          }
          for (int i = 0; i < 5; i++) {
            strcpy_P(buffer, (char * ) pgm_read_word( & (string_table2[i])));
            client.print(buffer);
          }
          client.print(myIP[0], DEC);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table2[5])));
          client.print(buffer);
          client.print(myIP[1], DEC);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table2[6])));
          client.print(buffer);
          client.print(myIP[2], DEC);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table2[7])));
          client.print(buffer);
          client.print(myIP[3], DEC);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table3[0])));
          client.print(buffer);
          client.print(myNM[0], DEC);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table3[1])));
          client.print(buffer);
          client.print(myNM[1], DEC);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table3[2])));
          client.print(buffer);
          client.print(myNM[2], DEC);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table3[3])));
          client.print(buffer);
          client.print(myNM[3], DEC);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table4[0])));
          client.print(buffer);
          client.print(myGW[0], DEC);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table4[1])));
          client.print(buffer);
          client.print(myGW[1], DEC);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table4[2])));
          client.print(buffer);
          client.print(myGW[2], DEC);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table4[3])));
          client.print(buffer);
          client.print(myGW[3], DEC);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table5[0])));
          client.print(buffer);
          client.print(RemIP[0], DEC);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table5[1])));
          client.print(buffer);
          client.print(RemIP[1], DEC);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table5[2])));
          client.print(buffer);
          client.print(RemIP[2], DEC);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table5[3])));
          client.print(buffer);
          client.print(RemIPmin, DEC);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table5[4])));
          client.print(buffer);
          client.print(RemIPmax, DEC);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table6[0])));
          client.print(buffer);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table6[1])));
          client.print(buffer);          
          client.print(button1);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table6[2])));
          client.print(buffer);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table6[3])));
          client.print(buffer);
          client.print(button2);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table6[4])));
          client.print(buffer);          
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table6[5])));
          client.print(buffer);
          client.print(button3);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table6[6])));
          client.print(buffer);          
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table6[7])));
          client.print(buffer);
          client.print(button4);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table6[8])));
          client.print(buffer);          
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table7[0])));
          client.print(buffer);                                        
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table7[1])));
          client.print(buffer);
          strcpy_P(buffer, (char * ) pgm_read_word( & (string_table7[2])));
          client.print(buffer);
          break;
        }
      }
    }
    delay(1);
    client.stop();
  }
}
