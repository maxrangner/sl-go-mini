#pragma once
#include "GawiButtons.h"
#include "LedMatrix.h"

class SystemManager {
    Button* _Button;
    ButtonManager _ButtonManager;
    LedMatrix Matrix;
public:
    SystemManager();
    void init();
    void run();
};