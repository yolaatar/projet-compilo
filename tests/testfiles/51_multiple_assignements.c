int main() {
    int a = 5;
    int b = 2;
    int c = 4;
    int d = 10;
    a *= b;
    a /= d;
    a += c;
    a -= 5;
    a += b + a * c - d;
    return a;
}