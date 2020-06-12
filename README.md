# EosTCP-Dynamic-RadioRF
A sketch to facilitate Radio communication via TCP OSC to an ETC Eos Family Console. This allows up to four macros (or other user-defined OSC commands) to be remotely triggered by 868Mhz RF over a TCP/IP network. A web form allows the end user to change settings as desired.


The OSC library is @sstaub's 2017 OSC library.


Function:
To allow an ETC Eos Family Console to be remotely trigged by a RF remote over the network. 
A web form allows the user to amend basic IP information, such as Local IP, Subnet Mask, and Remote IP address (Eos) range. The defined range allows the sketch to use ARP to find a console with the correct IP address and open port number. The Port number is allocated in the sketch. The MAC address is allocated in the sketch.


This sketch fires TCP OSC messages to trigger Macros on the Eos console. This sketch checks it has received a reply within a specified time. If a reply has not been received, this sketch pings the console, and if a reply is still not received, the sketch disconnects from the console.


RF Functionality is only enabled when IP address is selected.
