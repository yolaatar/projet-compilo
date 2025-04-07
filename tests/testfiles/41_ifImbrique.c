int main() {
    int a = 3, b = 5, c = 7, res;
    if (a < b) {
        if (b < c) {
            res = 1;
        } else {
            res = 2;
        }
    } else {
        res = 3;
    }
    return res;
}
