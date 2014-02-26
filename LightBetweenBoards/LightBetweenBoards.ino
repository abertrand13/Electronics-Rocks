/**
File: LightBetweenBoards.ino

Author: Derin Dutz

Date: 2.24.14

Description: Allows for an LED on the Arduino to be remotely controlled from the computer.

Important Information:
How To View/Edit the xBee Serial on a mac
  1. Open Terminal
  2. Type ls /dev/tty.*
  3. Type screen /dev/tty.usbserial 9600
*/


char computerChar;
char arduinoChar;

int RXLED = 17;

void setup() {
  pinMode(RXLED, OUTPUT);
  
  Serial.begin(9600);
  Serial1.begin(9600);
}

void loop() {
  // Receives information from computer
  receiveCharFromComputer();
  
  // Receives information from arduino
  receiveCharFromArduino();

  if (computerChar == '1') {
    digitalWrite(RXLED, HIGH);
    TXLED
  } else if (computerChar == '0') {
    digitalWrite(RXLED, LOW);
    TXLED0;
  }
}

/**
Receive a character from the computer.
*/
void receiveCharFromComputer() {
  if (Serial1.available()) {
    computerChar = (char) Serial1.read();
    Serial.print(computerChar);
  }
}

/**
Receive a character from the Arduino.
*/
void receiveCharFromArduino() {
  if (Serial.available()) {
    arduinoChar = (char) Serial.read();
    Serial1.print(arduinoChar);
  }
}
