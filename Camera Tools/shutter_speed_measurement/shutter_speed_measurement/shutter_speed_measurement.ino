#include <Wire.h>
#include <Adafruit_LiquidCrystal.h>

// Initialize LCD Shield
Adafruit_LiquidCrystal lcd(0);

// Define Buttons
#define BUTTON_SELECT 0
#define BUTTON_LEFT   1
#define BUTTON_UP     2
#define BUTTON_DOWN   3
#define BUTTON_RIGHT  4
#define NO_BUTTON     -1

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
    "3) Test 5"
};
const int menuSize = sizeof(menuItems) / sizeof(menuItems[0]);

void setup() {
    lcd.begin(16, 2);  // Initialize LCD
    pinMode(CONTROL_PIN, OUTPUT);
    digitalWrite(CONTROL_PIN, LOW); // Ensure control pin starts low
    lcd.setBacklight(HIGH);
    displayMenu();
}

void loop() {
    int button = getButtonPress();

    if (button == BUTTON_DOWN) {
        menuIndex = (menuIndex + 1) % menuSize;
        displayMenu();
    } 
    else if (button == BUTTON_UP) {
        menuIndex = (menuIndex - 1 + menuSize) % menuSize;
        displayMenu();
    } 
    else if (button == BUTTON_SELECT) {
        executeMenuOption();
        displayMenu();
    }
}

// Display current menu option
void displayMenu() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Select Option:");
    lcd.setCursor(0, 1);
    lcd.print(menuItems[menuIndex]);
}

// Read button input
int getButtonPress() {
    int adc_key_in = analogRead(A1);
    if (adc_key_in < 50) return BUTTON_SELECT;
    if (adc_key_in < 250) return BUTTON_LEFT;
    if (adc_key_in < 450) return BUTTON_DOWN;
    if (adc_key_in < 650) return BUTTON_UP;
    if (adc_key_in < 850) return BUTTON_RIGHT;
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
