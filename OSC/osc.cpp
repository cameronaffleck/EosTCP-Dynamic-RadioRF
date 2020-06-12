/* OSC library code is placed under the MIT license
 * Copyright (c) 2017 Stefan Staub
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "osc.h"

int32_t oscSend(char oscPacket[], char oscAddressPattern[], int32_t value, protocol_t protocol) {
	char oscMessage[128];
	memset (oscMessage, 0, 128);
	int32_t oscAddressPatternLength;
	uint8_t *int32Array = (uint8_t *) &value; // itoa
	strcpy(oscMessage, oscAddressPattern);
	oscAddressPatternLength = strlen(oscMessage);
	int32_t oscTypeStart = oscAddressPatternLength + 4 - (oscAddressPatternLength % 4);
	oscMessage[oscTypeStart] = ',';
	oscMessage[oscTypeStart + 1] = 'i';
	oscMessage[oscTypeStart + 4] = int32Array[3]; // little endian to big endian
	oscMessage[oscTypeStart + 5] = int32Array[2];
	oscMessage[oscTypeStart + 6] = int32Array[1];
	oscMessage[oscTypeStart + 7] = int32Array[0];
	int32_t oscMessageLength = oscTypeStart + 8;
	if (protocol == TCP10) {
		oscMessageLength = tcpSend(oscMessage, oscMessageLength);
		}
	if (protocol == TCP11) {
		oscMessageLength = slipSend(oscMessage, oscMessageLength);
		}
	memcpy(oscPacket, oscMessage, oscMessageLength);
	return oscMessageLength;
	}

int32_t oscSend(char oscPacket[], char oscAddressPattern[], float value, protocol_t protocol) {
	char oscMessage[128];
	memset (oscMessage, 0, 128);
	int32_t oscAddressPatternLength;
	uint8_t *floatArray = (uint8_t *) &value; // ftoa
	strcpy(oscMessage, oscAddressPattern);
	oscAddressPatternLength = strlen(oscMessage);
	int32_t oscTypeStart = oscAddressPatternLength + 4 - (oscAddressPatternLength % 4);
	oscMessage[oscTypeStart] = ',';
	oscMessage[oscTypeStart + 1] = 'f';
	oscMessage[oscTypeStart + 4] = floatArray[3]; // little endian to big endian
	oscMessage[oscTypeStart + 5] = floatArray[2];
	oscMessage[oscTypeStart + 6] = floatArray[1];
	oscMessage[oscTypeStart + 7] = floatArray[0];
	int32_t oscMessageLength = oscTypeStart + 8;
	if (protocol == TCP10) {
		oscMessageLength = tcpSend(oscMessage, oscMessageLength);
		}
	if (protocol == TCP11) {
		oscMessageLength = slipSend(oscMessage, oscMessageLength);
		}
	memcpy(oscPacket, oscMessage, oscMessageLength);
	return oscMessageLength;
	}

int32_t oscSend(char oscPacket[], char oscAddressPattern[], char string[], protocol_t protocol) {
	char stringBuffer[64];
	memset (stringBuffer, 0, 64);
	strcpy(stringBuffer, string);
	int32_t stringLength = strlen(stringBuffer);
	char oscMessage[128];
	memset (oscMessage, 0, 128);
	int32_t oscAddressPatternLength;
	strcpy(oscMessage, oscAddressPattern);
	oscAddressPatternLength = strlen(oscMessage);
	int32_t oscTypeStart = oscAddressPatternLength + 4 - (oscAddressPatternLength % 4);
	oscMessage[oscTypeStart] = ',';
	oscMessage[oscTypeStart + 1] = 's';
	memcpy(oscMessage + oscTypeStart + 4, stringBuffer, stringLength);
	int32_t oscMessageLength = oscTypeStart + 4 + stringLength + 4 - (stringLength % 4);
	if (protocol == TCP10) {
		oscMessageLength = tcpSend(oscMessage, oscMessageLength);
		}
	if (protocol == TCP11) {
		oscMessageLength = slipSend(oscMessage, oscMessageLength);
		}
	memcpy(oscPacket, oscMessage, oscMessageLength);
	return oscMessageLength;
	}

int32_t oscSend(char oscPacket[], char oscAddressPattern[], flag_t flag, protocol_t protocol) {
	char oscMessage[128];
	memset (oscMessage, 0, 128);
	int32_t oscAddressPatternLength;
	strcpy(oscMessage, oscAddressPattern);
	oscAddressPatternLength = strlen(oscMessage);
	int32_t oscTypeStart = oscAddressPatternLength + 4 - (oscAddressPatternLength % 4);
	oscMessage[oscTypeStart] = ',';
	if(flag == T) oscMessage[oscTypeStart + 1] = 'T';
	if(flag == F) oscMessage[oscTypeStart + 1] = 'F';
	if(flag == N) oscMessage[oscTypeStart + 1] = 'N';
	if(flag == I) oscMessage[oscTypeStart + 1] = 'I';
	int32_t oscMessageLength = oscTypeStart + 4;
	if (protocol == TCP10) {
		oscMessageLength = tcpSend(oscMessage, oscMessageLength);
		}
	if (protocol == TCP11) {
		oscMessageLength = slipSend(oscMessage, oscMessageLength);
		}
	memcpy(oscPacket, oscMessage, oscMessageLength);
	return oscMessageLength;
	}

int32_t oscSend(char oscPacket[], char oscAddressPattern[], protocol_t protocol) {
	char oscMessage[128];
	memset (oscMessage, 0, 128);
	int32_t oscAddressPatternLength;
	strcpy(oscMessage, oscAddressPattern);
	oscAddressPatternLength = strlen(oscMessage);
	int32_t oscTypeStart = oscAddressPatternLength + 4 - (oscAddressPatternLength % 4);
	int32_t oscMessageLength = oscTypeStart;
	if (protocol == TCP10) {
		oscMessageLength = tcpSend(oscMessage, oscMessageLength);
		}
	if (protocol == TCP11) {
		oscMessageLength = slipSend(oscMessage, oscMessageLength);
		}
	memcpy(oscPacket, oscMessage, oscMessageLength);
	return oscMessageLength;
	}

int32_t tcpSend(char message[], int32_t messageLength) {
	int32_t tcpLength = messageLength + 4;
	char tcp[128];
	memset(tcp, 0, 128);
	uint8_t *lengthArray = (uint8_t *) &messageLength; // itoa for TCP10
	tcp[0] = lengthArray[3]; // little endian to big endian
	tcp[1] = lengthArray[2];
	tcp[2] = lengthArray[1];
	tcp[3] = lengthArray[0];
	for (uint8_t i = 0; i < messageLength; i++) {
		tcp[i + 4] = message[i];
		}
	memcpy(message, tcp, tcpLength);
	return tcpLength;
	}


int32_t slipSend(char message[], int32_t messageLength) {
	int32_t slipLength = 0;
	char slip[128];
	memset(slip, 0, 128);
	slip[0] = END;
	slipLength++;
	for (uint8_t i = 0; i < messageLength; i++) {
		if (message[i] == END) {
			slip[slipLength] = ESC;
			slip[slipLength + 1] = ESC_END;
			slipLength = slipLength + 2;
			}
		else if (message[i] == ESC) {
			slip[slipLength] = ESC;
			slip[slipLength + 1] = ESC_ESC;
			slipLength = slipLength + 2;
			}
		else {
			slip[slipLength] = message[i];
			slipLength++;
			}
		}
	slip[slipLength] = END;
	slipLength++;
	memcpy(message, slip, slipLength);
	return slipLength;
	}
