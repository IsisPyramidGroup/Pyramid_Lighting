// User console driver for Isis Pyramid 1.1
// Based on Quemaduras del Sol and CarouShell
#include <SoftwareSerial.h>

/* Set up a serial port to talk to user console's display */
SoftwareSerial userSerial(9,8);

void user_setup(void) {
  
  // The 7-segment display is commanded via serial port 1.
  userSerial.begin(9600);          // user console display
  userSerial.write(SSEG_RESET);
  userSerial.write(SSEG_BRIGHTNESS);
  userSerial.write((uint8_t)0);    // maximum brightness
  userSerial.write(SSEG_DECIMALS);
  userSerial.write((uint8_t)0);    // decimal points off
  
  // The buttons are wired up to individual I/O pins.
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  digitalWrite(3, HIGH);    // enable pullups
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(7, HIGH);
  
}

// Simply display a 4-digit number on the 7-segment display
void display_number(int number)
{
  if (number < 0 || number > 9999)
    return;
    
  userSerial.write(SSEG_RESET);
  if (number < 10)
    userSerial.write(' ');
  if (number < 100)
    userSerial.write(' ');
  if (number < 1000)
    userSerial.write(' ');
  userSerial.print(number);
}

// Check the five buttons and return a bitmap value.
// 0 = button not pressed
// 1 = button pressed
uint8_t scan_buttons(void) {
  uint8_t bitmap = 0;
    
  if (digitalRead(3) == 0) bitmap |= BUTTON_BLUE;   //Serial.println("Blue button on pin 3");
  if (digitalRead(4) == 0) bitmap |= BUTTON_BLACK;  //Serial.println("Black button on pin 4");
  if (digitalRead(5) == 0) bitmap |= BUTTON_YELLOW; //Serial.println("Yellow button on pin 5");
  if (digitalRead(6) == 0) bitmap |= BUTTON_GREEN;  //Serial.println("Green button on pin 6");
  if (digitalRead(7) == 0) bitmap |= BUTTON_RED;    //Serial.println("Red button on pin 7");
  
  return bitmap;
}

