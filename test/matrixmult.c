int a[16] = {-2, 0, 0, 0,
             0, -2, 0, 0,
             0, 0, -2, 0,
             0, 0, 0, -2};
int b[16] = {1,  2,  3,  4,
             5,  6,  7,  8,
             9,  10, 11, 12,
             13, 14, 15, 16};
int c[16];

void printarr()
{
    int i = 0;
    do {
        print c[i];
        i = i + 1;
    } while (i < 16);
}

void dotprod(int i, int j)
{
    /* compute C[i,j] = the dot product of the ith row of A and the jth column of B */
    c[4*i+j] = 0;
    int k = 0;
    while (k < 4) {
        c[4*i+j] = c[4*i+j] + a[4*i+k] * b[4*k+j];
        k = k + 1;
    }
}

void matmult()
{
    int i = 0, j;
    while (i < 4) {
        j = 0;
        while (j < 4) {
            dotprod(i, j);
            j = j + 1;
        }
        i = i + 1;
    }
}

void main()
{
    matmult();
    printarr();
}
