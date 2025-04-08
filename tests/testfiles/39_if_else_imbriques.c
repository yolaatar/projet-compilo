#include <stdio.h>
int main() {
    int x = 10;
    int y = 0;
    if (x > 5) {
        if (x > 15) {
            y = 42;
        } else {
            y = -1;
        }
    } else {
        y = -2;
    }
    return y;
}