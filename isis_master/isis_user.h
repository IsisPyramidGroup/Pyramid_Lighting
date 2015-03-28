// User Console for Isis Pyramid 1.1
// based on Quemaduras del Sol and Caroushell
//
// The user console features a four-digit seven-segment readout
// with various decimal points, and five buttons (black in the center,
// blue on top, red on the right, green on the bottom, and yellow on
// the left.
//

#include <Arduino.h>

#define  BUTTON_GREEN   1
#define  BUTTON_RED     2
#define  BUTTON_BLACK   4
#define  BUTTON_YELLOW  8
#define  BUTTON_BLUE   16

#define  SSEG_RESET       0x76
#define  SSEG_DECIMALS    0x77
#define  SSEG_BRIGHTNESS  0x7A
#define  SSEG_DIGITS      0x7B
#define  SSEG_DIGIT1      0x7B
#define  SSEG_DIGIT2      0x7C
#define  SSEG_DIGIT3      0x7D
#define  SSEG_DIGIT4      0x7E


// Set up the driver
void user_setup(void);

// Simply display a number on the 7-segment display
void display_number(int number);

// Check the five buttons and return a bitmap value.
// 0 = button not pressed
// 1 = button pressed
uint8_t scan_buttons(void);
