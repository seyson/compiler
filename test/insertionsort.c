int a[10] = {57, 600, -23, 443, -70, 12, 43, 0, 1, -50};

void printarr()
{
    int i = 0;
    do {
        print a[i];
        i = i + 1;
    } while (i < 10);
}

void insertionsort()
{
    int i = 1, j, ai;
    while (i < 10) {
        ai = a[i];
        j = i - 1;
        while (a[j] > ai && j >= 0) {
            a[j+1] = a[j];
            j = j - 1;
        }
        a[j+1] = ai;
        i = i + 1;
    }
}

void main()
{
    insertionsort();
    printarr();
}
