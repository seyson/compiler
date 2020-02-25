void main()
{
    int x;
    x = 1 + 1 * 2; print x; // 3
    x = (1 + 1) * 2; print x; // 4
    x = 5 == 10 / 2; print x; // 1
    x = (5 == 10) / 2; print x; // 0
    x = 5 * 6 == 6; print x; // 0
    x = 5 * (6 == 6); print x; // 5
    x = ((25 + 15) / 10) * 4; print x; // 16
    x = (25 + 15 / 10) + 4; print x; // 30
}