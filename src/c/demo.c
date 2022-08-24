//
// Created by bczhc on 24/08/22.
//

#include <stdio.h>
#include "clipboard.h"

void printCallback(char *data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        fwrite_unlocked(data + i, 1, 1, stdout);
    }
    fflush(stdout);
}

int main() {
    start(printCallback);
}