#include <QtWidgets/QApplication>
#include <QDesktopWidget>
#include <QtGlobal>
#include <QtDebug>
#include <QSettings>
#include <QIcon>
#include <QTranslator>
#include <QString>
#include <QLocale>
#include <QRegularExpression>
#include <QObject>
#include <QGuiApplication>
#include <initializer_list>
#include <signal.h>
#include <unistd.h>

#include <iostream>

#include <sys/mman.h>


#include "settings.h"

#include "main.h"
#include "mainwindow.h"
#include "x11_utils.h"
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <QtX11Extras/QX11Info>
#include <X11/Xcursor/Xcursor.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include<X11/extensions/Xrandr.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>

Display* dpy;
int timeout, interval, prefer_blanking, allow_exposures;
bool lockhint = false;
void syncX(){
#if 0
    Display *display = QX11Info::display();
    int screen = XDefaultScreen (display);
    Window w = RootWindow (display, screen);
    Atom id = XInternAtom (display, "AT_SPI_BUS", True);
    if (id != None)
    {
        XDeleteProperty (display, w, id);
        XSync (display, false);
    }

#endif
}



void ignoreUnixSignals(std::initializer_list<int> ignoreSignals) {
    // all these signals will be ignored.
    for (int sig : ignoreSignals)
        signal(sig, SIG_IGN);


}


void setScreensaver(){


    Display *display = QX11Info::display();
    XGetScreenSaver (display, &timeout, &interval, &prefer_blanking, &allow_exposures);
    XForceScreenSaver (display, ScreenSaverActive);
    //XSetScreenSaver (display, 60, 0, ScreenSaverActive, DefaultExposures);
    XSetScreenSaver (display, 60, 0, ScreenSaverActive, DefaultExposures);

}

void restoreScreensaver(){

    Display *display = QX11Info::display();
    XSetScreenSaver (display, timeout, interval, prefer_blanking, allow_exposures);
}


int xkb_init()
{
    int xkb_opcode, xkb_event, xkb_error;
    int xkb_lmaj = XkbMajorVersion;
    int xkb_lmin = XkbMinorVersion;
    return XkbLibraryVersion( &xkb_lmaj, &xkb_lmin )
            && XkbQueryExtension( dpy, &xkb_opcode, &xkb_event, &xkb_error,
                                  &xkb_lmaj, &xkb_lmin );
}

unsigned int xkb_mask_modifier( XkbDescPtr xkb, const char *name )
{
    int i;
    if( !xkb || !xkb->names )
        return 0;
    for( i = 0;
         i < XkbNumVirtualMods;
         i++ )
    {
        char* modStr = XGetAtomName( xkb->dpy, xkb->names->vmods[i] );
        if( modStr != NULL && strcmp(name, modStr) == 0 )
        {
            unsigned int mask;
            XkbVirtualModsToReal( xkb, 1 << i, &mask );
            return mask;
        }
    }
    return 0;
}

unsigned int xkb_numlock_mask()
{
    XkbDescPtr xkb;
    if(( xkb = XkbGetKeyboard( dpy, XkbAllComponentsMask, XkbUseCoreKbd )) != NULL )
    {
        unsigned int mask = xkb_mask_modifier( xkb, "NumLock" );
        XkbFreeKeyboard( xkb, 0, True );
        return mask;
    }
    return 0;
}

int xkb_set_on()
{
    unsigned int mask;
    if( !xkb_init())
        return 0;
    mask = xkb_numlock_mask();
    if( mask == 0 )
        return 0;
    XkbLockModifiers ( dpy, XkbUseCoreKbd, mask, mask);
    return 1;
}

void numlock_set_on()
{
    xkb_set_on();
}


void openNumlock(void){

    dpy = XOpenDisplay( NULL );
    if( dpy == NULL )
    {

        return;
    }

    numlock_set_on();
    XCloseDisplay( dpy );

}

int getScreenNumber(QScreen *qscreen){
    int index = 0;
    for (int i = 0; i< QGuiApplication::screens().length(); i++){
        if(QGuiApplication::screens()[i] == qscreen){
            index = i;
            break;
        }
    }
    return index;
}


void setRootBackground(QImage img, QScreen *qscreen){

    Display *dis  = QX11Info::display();

    auto screen = getScreenNumber(qscreen);
    Window win = RootWindow(dis, screen);

    QRect screenRect = qscreen->geometry();

    int width = screenRect.width();
    int height = screenRect.height();

    unsigned int depth = (unsigned int)DefaultDepth(dis, screen);

    XFlush(dis);

    Pixmap pix = XCreatePixmap(dis,win, width, height, depth);

    char *tempimage = (char *)malloc(width * height * 4);

    int k = 0;

    for(int i = 0; i< height; i++ ){
        for(int j = 0; j< width; j++){
            *((uint*)tempimage + k) = img.pixel(j,i);
            k++;
        }
    }

    Visual *visual = DefaultVisual(dis, screen);

    XImage *image = XCreateImage(dis, visual, 24, ZPixmap, 0, tempimage, width, height, 32, 0);

    GC gc = XCreateGC(dis, pix, 0, NULL);

    XPutImage(dis, pix, gc, image, 0, 0, 0, 0, width, height);

    XSetWindowBackgroundPixmap(dis, win, pix);

    /* Prevent from x shaped cursor after greeter is closed */
    Cursor c = XcursorLibraryLoadCursor(dis, "arrow");
    XDefineCursor (dis, win, c);

    /* Clear pixmap */
    XFreePixmap(dis, pix);
    XDestroyImage(image);
    // free(tempimage);
    XClearWindow(dis, win);
}
