#include <stdio.h>
#include <stdbool.h>

//
//   Your other functions
//   and variables
//
//   should go here
//

float evaluate (const char* expr, bool immediate_result)
{
    return 0.0;  /* replace it with your own */
}

int main()
{

    //
    // you may add more code here
    //

    /* 8< 8< 8< begin-cut-here >8 >8 >8 */
    printf ("Test 1 %f\n", evaluate ("2.0 * 3.0", true));
    printf ("Test 2 %f\n", evaluate ("200.0 + 300.0", true));
    printf ("Test 3 %f\n", evaluate ("10.0 / 5.0", true));
    printf ("Test 4 %f\n", evaluate ("16.0 - 10.5", true));
    /* add more test of your own */
    /* 8< 8< 8< end-cut-here >8 >8 >8 */
    return 0;
}

