int main() {
    int x = 1;
    int y = 0;
    int a = x && y;
    int b = 1 && 1;
    int c = a && b || x && 1 || 0 || 1 || 1 && 1 || y;
    return c;
}