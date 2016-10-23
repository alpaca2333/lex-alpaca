#include "nfa.h"

using namespace std;

int main()
{
    NFA fa('a');

    fa.parallel(new NFA('b'));

    fa.repeat(REPEAT_0_N);

    fa.parallel(new NFA('c'));

    fa.concat(new NFA('z'));

    /* assign a non-negative integer to endType
     * to ensure the nfa considers itself finished
     */
    fa.end()->endType = 1;


    cout << fa.regex << endl;

    cout << fa.matches("b");
    cout << fa.matches("aaabz");
    cout << fa.matches("ccz");
    cout << endl;
    return 0;
}