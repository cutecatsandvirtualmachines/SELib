#include "SELib/Svm.h"

#include <Windows.h>

void svm::Vmcb::InjectException(unsigned long long Vector)
{
    EventInj EventInjection{ 0 };
    EventInjection.layout.Vector = Vector;
    EventInjection.layout.Type = 3;
    EventInjection.layout.Valid = TRUE;
    EventInjection.layout.ErrorCodeValid = FALSE;
    ControlArea.EventInjection.raw = EventInjection.raw;
}

void svm::Vmcb::InjectException(unsigned long long Vector, unsigned long long ErrorCode)
{
    EventInj EventInjection{ 0 };
    EventInjection.layout.Vector = Vector;
    EventInjection.layout.Type = 3;
    EventInjection.layout.Valid = TRUE;
    EventInjection.layout.ErrorCodeValid = TRUE;
    EventInjection.layout.ErrorCode = ErrorCode;
    ControlArea.EventInjection.raw = EventInjection.raw;
}
