int main() {
    int a = 10;
    int b = 20;
    {
        int a = 5;   
        b = b + a;    
    }
    return a + b;    
}
