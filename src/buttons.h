#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>
#include "config.h"

enum ButtonEvent {
    BUTTON_NONE,
    BUTTON_NEXT_PRESSED,
    BUTTON_SELECT_PRESSED
};

class ButtonManager {
public:
    ButtonManager();
    void begin();
    ButtonEvent update();
    
private:
    bool nextButtonState;
    bool selectButtonState;
    bool lastNextButtonState;
    bool lastSelectButtonState;
    unsigned long lastNextDebounceTime;
    unsigned long lastSelectDebounceTime;
    
    bool debounceButton(int pin, bool& currentState, bool& lastState, unsigned long& lastDebounceTime);
};

#endif // BUTTONS_H