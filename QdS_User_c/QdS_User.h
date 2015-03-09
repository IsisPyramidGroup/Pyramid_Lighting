// User Console for Quemaduras del Sol
//
// The user console features a four-digit seven-segment readout
// with various decimal points, and five buttons (black in the center,
// blue on top, red on the right, green on the bottom, and yellow on
// the left.
//

// Set up the driver
void user_setup(void);

// Simply display a number on the 7-segment display
void display_number(int number);

// Check the five buttons and return a bitmap value.
// 0 = button not pressed
// 1 = button pressed
//
// 0x01 = center black button
// 0x02 = top blue button
// 0x04 = right red button
// 0x08 = bottom green button
// 0x10 = left yellow button
int scan_buttons(void);
