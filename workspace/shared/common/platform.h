#ifndef PLATFORM_H
#define PLATFORM_H

int getBatteryLevel(void);

void enterSleep(void);
void exitSleep(void);

int preventAutosleep(void);

#endif