#pragma once
#include "Canvas.h"
#include "Usage.h"

namespace Screens {
    void splash(Canvas& c, const char* sub);
    void portal(Canvas& c, const char* ap, const char* ip);
    void connecting(Canvas& c, const char* ssid);
    void dashboard(Canvas& c, const char* provider, const UsageStatus& u, uint32_t now, int rssi, int secsAgo);
    void error(Canvas& c, const char* title, const char* detail);
}
