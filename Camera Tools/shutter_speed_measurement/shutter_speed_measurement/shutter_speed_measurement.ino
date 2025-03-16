#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

// Initialize LCD Shield
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// Define Buttons
#define BUTT_SELECT 0
#define BUTT_LEFT   1
#define BUTT_UP     2
#define BUTT_DOWN   3
#define BUTT_RIGHT  4
#define NO_BUTTON     -1

// These #defines make it easy to set the backlight color
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

// Define Pins
#define ANALOG_PIN A0     // Analog input pin
#define CONTROL_PIN 8     // Digital output pin for alignment mode
#define THRESHOLD 512     // ADC threshold (512 = ~2.5V)
#define TIMEOUT 1000000   // Timeout in microseconds (1s)


// Menu State
int menuIndex = 0;
const char* menuItems[] = {
    "1) Align System",
    "2) Test Once",
    "3) Test 5",
};
const int menuSize = sizeof(menuItems) / sizeof(menuItems[0]);

void setup() {
    lcd.begin(16, 2);  // Initialize LCD
    pinMode(CONTROL_PIN, OUTPUT);
    digitalWrite(CONTROL_PIN, LOW); // Ensure control pin starts low
    lcd.setBacklight(WHITE);
    displayMenu();
}

void loop() {
    int button = getButtonPress();

    if (button == BUTT_DOWN) {
        menuIndex = (menuIndex + 1) % menuSize;
        displayMenu();
    } 
    else if (button == BUTT_UP) {
        menuIndex = (menuIndex - 1 + menuSize) % menuSize;
        displayMenu();
    } 
    else if (button == BUTT_SELECT) {
        executeMenuOption();
        displayMenu();
    }
}

// Display current menu option
void displayMenu() {
    lcd.clear();
    lcd.setBacklight(WHITE);
    lcd.setCursor(0, 0);
    lcd.print("Select Option:");
    lcd.setCursor(0, 1);
    lcd.print(menuItems[menuIndex]);
}

// Read button input
int getButtonPress() {
    uint8_t buttons = lcd.readButtons();
    if (buttons & BUTTON_UP) return BUTT_UP;
    if (buttons & BUTTON_DOWN) return BUTT_DOWN;
    if (buttons & BUTTON_LEFT) return BUTT_LEFT;
    if (buttons & BUTTON_RIGHT) return BUTT_RIGHT;
    if (buttons & BUTTON_SELECT) return BUTT_SELECT;
    return NO_BUTTON;
}

// Execute the selected menu option
void executeMenuOption() {
    switch (menuIndex) {
        case 0: alignSystem(); break;
        case 1: testOnce(); break;
        case 2: testFive(); break;
    }
}

// Align System: Hold control pin HIGH until user presses a button
void alignSystem() {
    lcd.setBacklight(RED);
    delay(250);  //need this as there is a button press in the queue / debounce
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Aligning...");
    digitalWrite(CONTROL_PIN, HIGH);

    while (getButtonPress() == NO_BUTTON); // Wait for button press
    digitalWrite(CONTROL_PIN, LOW);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alignment Done!");
    delay(1000);
}

// Test Once: Measure signal duration once and display result
void testOnce() {
    lcd.setBacklight(VIOLET);
    delay(250);
    unsigned long duration = measureAnalogDuration();

    

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Duration:");
    lcd.setCursor(0, 1);
    lcd.print(duration);
    lcd.print(" us");

    waitForButtonPress();
}

// Test 5: Measure signal duration 5 times and display stats
void testFive() {
    unsigned long durations[5];
    unsigned long sum = 0;
    double mean, variance = 0, stddev;

    lcd.setBacklight(BLUE);
    delay(250);
    for (int i = 0; i < 5; i++) {
        durations[i] = measureAnalogDuration();
        sum += durations[i];

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Test ");
        lcd.print(i + 1);
        lcd.print(":");
        lcd.setCursor(0, 1);
        lcd.print(durations[i]);
        lcd.print(" us");

        waitForButtonPress();
    }

    mean = sum / 5.0;
    for (int i = 0; i < 5; i++) {
        variance += pow(durations[i] - mean, 2);
    }
    stddev = sqrt(variance / 5.0);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Avg: ");
    lcd.print(mean);
    lcd.print(" us");

    lcd.setCursor(0, 1);
    lcd.print("Std Dev: ");
    lcd.print(stddev);
    lcd.print(" us");

    waitForButtonPress();
}

// Measure how long the analog signal stays above the threshold
unsigned long measureAnalogDuration() {
    unsigned long startTime = 0, duration = 0;

    // Wait for signal to rise above threshold
    while (analogRead(ANALOG_PIN) < THRESHOLD) {
        if (micros() - startTime > TIMEOUT) return 0;
    }

    startTime = micros();

    // Measure time above threshold
    while (analogRead(ANALOG_PIN) >= THRESHOLD) {
        if (micros() - startTime > TIMEOUT) break;
    }

    duration = micros() - startTime;
    return duration;
}

// Wait for user button press before continuing
void waitForButtonPress() {
    while (getButtonPress() == NO_BUTTON);
    delay(300); // Debounce delay
}
