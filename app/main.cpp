// #include "mainwindow.h"

// #include <QApplication>
// #include <QSystemTrayIcon>
// #include <QMenu>

// #include "models/launcherinfo.h"

// int main(int argc, char *argv[])
// {
//     QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths() << ":/imgs/elementary_icons");
//     qRegisterMetaType<QList<LauncherInfo*>>("QList<LauncherInfo*>");
//     qRegisterMetaType<QSet<LauncherInfo*>>("QSet<LauncherInfo*>");
//     qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");

//     QApplication a(argc, argv);
//     QSystemTrayIcon tray;
//     tray.setIcon(QIcon(":tray_light"));
//     tray.show();
// #ifdef Q_OS_MAC
//     qApp->setWindowIcon(QIcon(":/imgs/icon_mac"));
// #else
//     qApp->setWindowIcon(QIcon(":/imgs/icon"));
// #endif

//     QMenu menu;
//     tray.setContextMenu(&menu);
//     MainWindow w(&tray, &menu);
//     w.show();
//     a.setQuitOnLastWindowClosed(false);
//     return a.exec();
// }

#include "mainwindow.h"

#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>


#ifdef Q_OS_MAC
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOMessage.h>

MainWindow* w;
bool init = false;

void DeviceAdded(void* refCon, io_iterator_t iterator) {
    io_service_t service;
    while ((service = IOIteratorNext(iterator))) {
        CFStringRef deviceName = (CFStringRef) IORegistryEntryCreateCFProperty(service, CFSTR(kUSBProductString), kCFAllocatorDefault, 0);
        if (deviceName) {
            char deviceNameCString[256];
            CFStringGetCString(deviceName, deviceNameCString, 256, kCFStringEncodingUTF8);
            qDebug() << "Device connected: " << deviceNameCString;
            if(init){
                w->requestLoadDevices();
            }else{
                init = true;
            }
            CFRelease(deviceName);
        }
        IOObjectRelease(service);
    }
}

void startDeviceMonitor() {
    mach_port_t masterPort;
    IOMasterPort(MACH_PORT_NULL, &masterPort);

    CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
    if (!matchingDict) {
        qCritical() << "Failed to create matching dictionary.";
        return;
    }

    io_iterator_t addedIter;
    IONotificationPortRef notifyPort = IONotificationPortCreate(masterPort);
    CFRunLoopSourceRef runLoopSource = IONotificationPortGetRunLoopSource(notifyPort);

    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);

    kern_return_t kr = IOServiceAddMatchingNotification(notifyPort,
                                                        kIOFirstMatchNotification,
                                                        matchingDict,
                                                        DeviceAdded,
                                                        nullptr,
                                                        &addedIter);

    if (kr != KERN_SUCCESS) {
        qCritical() << "Failed to add matching notification.";
        return;
    }

    // Call the callback once for devices that are already connected
    DeviceAdded(nullptr, addedIter);

    // Start the run loop to keep the program running
    CFRunLoopRun();
}

#elif defined(Q_OS_WIN)
#include <windows.h>
#include <dbt.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_DEVICECHANGE:
        if (wParam == DBT_DEVICEARRIVAL) {
            qDebug() << "New device connected!";
            w->requestLoadDevices();
        }
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void startDeviceMonitor() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "Sample Window Class", NULL };
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName, "Device Notification Window", WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, NULL, NULL, wc.hInstance, NULL);

    // Register for device notifications
    DEV_BROADCAST_DEVICEINTERFACE notificationFilter;
    ZeroMemory(&notificationFilter, sizeof(notificationFilter));
    notificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    notificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

    HDEVNOTIFY hDevNotify = RegisterDeviceNotification(hwnd, &notificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnregisterDeviceNotification(hDevNotify);
}

#elif defined(Q_OS_LINUX)
#include <iostream>
#include <fstream>
#include <cstdlib>

void startDeviceMonitor() {
    std::ofstream ruleFile("/etc/udev/rules.d/99-android.rules");
    if (!ruleFile.is_open()) {
        qCritical() << "Failed to create udev rule file.";
        return;
    }

    // Write the udev rule to detect Android devices
    ruleFile << "SUBSYSTEM==\"usb\", ATTR{idVendor}==\"18d1\", MODE=\"0666\", ACTION==\"add\", RUN+=\"/path/to/your/script.sh\"" << std::endl;
    ruleFile.close();

    // Reload udev rules
    system("sudo udevadm control --reload-rules");
    system("sudo service udev restart");

    qDebug() << "udev rule created and service restarted. Now listening for Android device connections.";
}

#endif

int main(int argc, char *argv[]) {
    QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths() << ":/imgs/elementary_icons");
    qRegisterMetaType<QList<LauncherInfo*>>("QList<LauncherInfo*>");
    qRegisterMetaType<QSet<LauncherInfo*>>("QSet<LauncherInfo*>");
    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");

    QApplication a(argc, argv);
    QSystemTrayIcon tray;
    tray.setIcon(QIcon(":tray_light"));
    tray.show();
#ifdef Q_OS_MAC
    qApp->setWindowIcon(QIcon(":/imgs/icon_mac"));
#else
    qApp->setWindowIcon(QIcon(":/imgs/icon"));
#endif

    QMenu menu;
    tray.setContextMenu(&menu);
    w = new MainWindow (&tray, &menu);
    w->show();
    a.setQuitOnLastWindowClosed(false);

    QTimer::singleShot(5000, []{
#ifdef Q_OS_MAC
        std::thread monitorThread(startDeviceMonitor);
        monitorThread.detach();
#elif defined(Q_OS_WIN)
        std::thread monitorThread(startDeviceMonitor);
        monitorThread.detach();
#elif defined(Q_OS_LINUX)
        startDeviceMonitor();
#endif
    });


    return a.exec();
}
