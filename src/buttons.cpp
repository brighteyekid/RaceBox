#include "buttons.h"

ButtonManager::ButtonManager() : 
    nextButtonState(false),
    selectButtonState(false),
    lastNextButtonState(false),
    lastSelectButtonState(false),
    lastNextDebounceTime(0),
    lastSelectDebounceTime(0) {
}

void ButtonManager::begin() {
    pinMode(BUTTON_NEXT_PIN, INPUT_PULLUP);
    pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);
    
    // Initialize button states
    nextButtonState = digitalRead(BUTTON_NEXT_PIN) == LOW;
    selectButtonState = digitalRead(BUTTON_SELECT_PIN) == LOW;
    lastNextButtonState = nextButtonState;
    lastSelectButtonState = selectButtonState;
    
    Serial.println("Button manager initialized");
    Serial.printf("Next button pin: %d, Select button pin: %d\n", BUTTON_NEXT_PIN, BUTTON_SELECT_PIN);
}

ButtonEvent ButtonManager::update() {
    ButtonEvent event = BUTTON_NONE;
    
    // Check next button
    if (debounceButton(BUTTON_NEXT_PIN, nextButtonState, lastNextButtonState, lastNextDebounceTime)) {
        if (nextButtonState) { // Button pressed (LOW = pressed due to pullup)
            event = BUTTON_NEXT_PRESSED;
            Serial.println("Next button pressed");
        }
    }
    
    // Check select button (only if next button wasn't pressed)
    if (event == BUTTON_NONE) {
        if (debounceButton(BUTTON_SELECT_PIN, selectButtonState, lastSelectButtonState, lastSelectDebounceTime)) {
            if (selectButtonState) { // Button pressed (LOW = pressed due to pullup)
                event = BUTTON_SELECT_PRESSED;
                Serial.println("Select button pressed");
            }
        }
    }
    
    return event;
}

bool ButtonManager::debounceButton(int pin, bool& currentState, bool& lastState, unsigned long& lastDebounceTime) {
    bool reading = (digitalRead(pin) == LOW); // LOW = pressed due to pullup resistor
    bool stateChanged = false;
    
    // If the switch changed, due to noise or pressing
    if (reading != lastState) {
        lastDebounceTime = millis();
    }
    
    // If enough time has passed since the last state change
    if ((millis() - lastDebounceTime) > BUTTON_DEBOUNCE_MS) {
        // If the button state has changed
        if (reading != currentState) {
            currentState = reading;
            stateChanged = true;
        }
    }
    
    lastState = reading;
    return stateChanged;
}