#include "nfa.h"

using namespace std;

int main()
{
    NFA fa('a');
    cout << fa.regex << endl;

    fa.parallel(new NFA('b'));
    cout << fa.regex << endl;

    fa.repeat(REPEAT_0_N);
    cout << fa.regex << endl;

    fa.parallel(new NFA('c'));
    cout << fa.regex << endl;

    fa.concat(new NFA('z'));
    cout << fa.regex << endl;

    fa.end()->endType = 1;

    cout << fa.matches("b");
    cout << fa.matches("aaabz");
    cout << fa.matches("cz");

    return 0;
}