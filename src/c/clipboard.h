//
// Created by bczhc on 24/08/22.
//

#ifndef X11_EVENT_MONITOR_CLIPBOARD_H
#define X11_EVENT_MONITOR_CLIPBOARD_H

#include <stddef.h>

void start(void (*callback)(char *data, size_t size));

#endif //X11_EVENT_MONITOR_CLIPBOARD_H
