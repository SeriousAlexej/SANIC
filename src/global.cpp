#include "global.h"

egg& egg::getInstance()
{
    static egg instance;
    return instance;
}
