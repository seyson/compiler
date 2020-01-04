int x = 999;

void square(int x)
{
    print x*x;
}

void sumofsquares(int x, int y)
{
    print x*x + y*y;
}

void p()
{
    print x;
}

void main()
{
    int x = 3;
    int y = 4;
    square(y);
    sumofsquares(x, y);
    p();
}
