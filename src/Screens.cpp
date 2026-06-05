#include "Screens.h"
#include <stdio.h>
#include <string.h>

namespace {

void centered(Canvas& c, int y, Scale s, Ink ink, const char* str) {
    int x = (c.width() - c.textWidth(s, str)) / 2;
    if (x < 0) x = 0;
    c.text(x, y, s, ink, str);
}

void rightText(Canvas& c, int rightX, int y, Scale s, Ink ink, const char* str) {
    int x = rightX - c.textWidth(s, str);
    if (x < 0) x = 0;
    c.text(x, y, s, ink, str);
}

void bar(Canvas& c, int x, int y, int w, int h, float pct) {
    c.box(x, y, w, h, Ink::Fg, false);
    int fill = (int)((w - 2) * pct / 100.0f);
    if (fill < 0) fill = 0;
    if (fill > w - 2) fill = w - 2;
    if (fill > 0) c.box(x + 1, y + 1, fill, h - 2, Ink::Accent, true);
}

// Pick the largest title scale that fits this panel width.
Scale titleScale(Canvas& c) {
    return c.width() >= 120 ? Scale::Large : (c.width() >= 80 ? Scale::Medium : Scale::Small);
}

} // namespace

namespace Screens {

void splash(Canvas& c, const char* sub) {
    c.clear();
    Scale ts = titleScale(c);
    int y = c.height() / 2 - c.lineHeight(ts);
    centered(c, y, ts, Ink::Accent, "AI Usage");
    centered(c, y + c.lineHeight(ts) + 2, Scale::Small, Ink::Dim, sub);
    c.present();
}

void portal(Canvas& c, const char* ap, const char* ip) {
    c.clear();
    centered(c, 0, Scale::Medium, Ink::Accent, "SETUP");
    c.hline(0, c.lineHeight(Scale::Medium) + 1, c.width(), Ink::Dim);
    int y = c.lineHeight(Scale::Medium) + 4;
    char line[40];
    snprintf(line, sizeof(line), "WiFi: %s", ap);
    c.text(2, y, Scale::Small, Ink::Fg, line); y += c.lineHeight(Scale::Small) + 2;
    c.text(2, y, Scale::Small, Ink::Dim, "Open in browser:"); y += c.lineHeight(Scale::Small) + 1;
    centered(c, y, Scale::Medium, Ink::Fg, ip);
    c.present();
}

void connecting(Canvas& c, const char* ssid) {
    c.clear();
    c.text(2, 2, Scale::Medium, Ink::Fg, "Connecting...");
    char line[28];
    snprintf(line, sizeof(line), "SSID: %s", ssid);
    c.text(2, 2 + c.lineHeight(Scale::Medium) + 4, Scale::Small, Ink::Dim, line);
    c.present();
}

void dashboard(Canvas& c, const UsageStatus& u, uint32_t now, int rssi, int secsAgo) {
    c.clear();
    int W = c.width();
    Scale lbl = (W >= 100) ? Scale::Medium : Scale::Small;
    int lh = c.lineHeight(lbl);
    char line[20], rst[16];

    // Row 1 — 5H
    int y = 0;
    snprintf(line, sizeof(line), "5H %.0f%%", u.h5Percent);
    c.text(2, y, lbl, Ink::Fg, line);
    formatCountdown(u.h5Reset, now, rst, sizeof(rst));
    rightText(c, W - 2, y, lbl, Ink::Dim, rst);
    bar(c, 2, y + lh, W - 4, 7, u.h5Percent);

    // Row 2 — 7D
    y += lh + 11;
    snprintf(line, sizeof(line), "7D %.0f%%", u.d7Percent);
    c.text(2, y, lbl, Ink::Fg, line);
    formatCountdown(u.d7Reset, now, rst, sizeof(rst));
    rightText(c, W - 2, y, lbl, Ink::Dim, rst);
    bar(c, 2, y + lh, W - 4, 7, u.d7Percent);

    // Status line at the bottom
    snprintf(line, sizeof(line), "%ddBm  %ds ago", rssi, secsAgo);
    c.text(2, c.height() - c.lineHeight(Scale::Small), Scale::Small, Ink::Dim, line);
    c.present();
}

void error(Canvas& c, const char* title, const char* detail) {
    c.clear();
    c.text(2, 2, Scale::Medium, Ink::Warn, title);
    if (detail) c.text(2, 2 + c.lineHeight(Scale::Medium) + 6, Scale::Small, Ink::Dim, detail);
    c.present();
}

} // namespace Screens
