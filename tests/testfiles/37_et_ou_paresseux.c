int main() {
    int x = 0;
    int y = 1;
    int a = x || y;
    int b = 1 && 0;
    int c = a || b && x && 1 && 0 || 1 || 1 && 1 || y;
    return c;
}