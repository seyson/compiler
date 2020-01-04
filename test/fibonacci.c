int A[12] = {0, 1};

void printarr()
{
    int i = 0;
    while (i < 12) {
        print A[i];
        i = i + 1;
    }
}

void fib()
{
    int i = 2;
    while (i < 12) {
        A[i] = A[i - 1] + A[i - 2];
        i = i + 1;
    }
}

void main()
{
    fib();
    printarr();
}
