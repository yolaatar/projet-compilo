int recu(int n){
    if (n>0){
        return recu(n-1);
    }
    return 0;
}

int main()
{
    return recu(4);
}