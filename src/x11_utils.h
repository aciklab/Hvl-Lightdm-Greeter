#ifndef X11_UTILS_H
#define X11_UTILS_H
void setScreensaver();
void restoreScreensaver();

void openNumlock(void);
void syncX();
void setRootBackground(QImage img, QScreen *qscreen);
#endif // X11_UTILS_H
