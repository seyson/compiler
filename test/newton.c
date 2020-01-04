/* Newton's method for computing the square root of a number */
void main()
{
    int x = 2;
    float d = 0.00001;
    float guess = 1.0;
    float error = guess*guess - x;
    if (error < 0)
        error = -error;
    while (error >= d) {
        guess = (x/guess + guess) / 2.0;
        error = guess*guess - x;
        if (error < 0)
            error = -error;
    }
    print guess;
}
