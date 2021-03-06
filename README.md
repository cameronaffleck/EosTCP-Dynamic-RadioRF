# EosTCP-Dynamic-RadioRF
A sketch to facilitate Radio communication via TCP OSC to an ETC Eos Family Console. This allows up to four macros to be remotely triggered by 868Mhz RF over a TCP/IP network. A web form allows the end user to change settings as desired.


The OSC library is @sstaub's 2017 OSC library.


## Function:
To allow an ETC Eos Family Console to be remotely trigged by a RF remote over the network. 
A web form allows the user to amend select information such as; Local IP, Subnet Mask, Gateway, Remote IP address (Eos) range, and the four defined macros. The defined range allows the sketch to use ARP (Address Resolution Protocol) to find a console with the correct IP address and open port number. The Port number is allocated in the sketch. The MAC address is allocated in the sketch.


This sketch fires TCP OSC messages to trigger Macros on the Eos console. The sketch constantly pings the console to confirm the connection is still active. If a reply is received, a LED flashes. If a reply has not been received, this sketch re-pings the console, and if a reply is still not received, the sketch disconnects from the console.


RF Functionality is only enabled when device is successfully subscribed to Eos.


## Parts:
1x Arduino Uno

1x Arduino Ethernet Shield - WizNet W5500

1x RF Solutions ZPT-8RS RF Module - https://www.rfsolutions.co.uk/radio-modules-c10/zpt-radio-telemetry-receiver-module-868mhz-smt-p774

1x RF Soltuions Fobber-8T4 Remote - https://www.rfsolutions.co.uk/remote-control-systems-c9/fobber-4-button-150m-868mhz-key-fob-transmitter-p447

(Arduino Uno and Arduino Ethernet Shield can be swapped for FreeTronics's EtherTen. This board is an Uno and Ethernet Shield combined. The ethernet chip is W5100, so cannot provide LINKSTATUS updates. This board has built-in headers to make POE compatibility easy. - https://www.freetronics.com.au/collections/arduino/products/etherten )
