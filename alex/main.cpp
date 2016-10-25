#include <algorithm>
#include "nfa.h"
#include "regex.h"
using namespace std;

int main()
{

//    NFA fa('a');
//
//    fa.parallel(new NFA('b'));
//
//    fa.repeat(REPEAT_0_N);
//
//    fa.parallel(new NFA('c'));
//
//    fa.concat(new NFA('z'));
//
//    /* assign a non-negative integer to endType
//     * to ensure the nfa considers itself finished
//     */
//    fa.end()->endType = 1;
//
//
//    cout << fa.regex << endl;
//
//    cout << fa.matches("b");
//    cout << fa.matches("aaabz");
//    cout << fa.matches("ccz");
//    cout << endl;

#define TOKEN_ID 1
#define TOKEN_BLANK 2

    Regex re("[a-zA-Z_][a-zA-Z_0-9]*");

    NFA* nfa = re.getNFA();
    NFA* nfa2 = new NFA('+');
    nfa->parallel(nfa2);

    nfa->setEndType(TOKEN_ID, 1);
    nfa2->setEndType(TOKEN_BLANK, 2);

    nfa->onTokenAccepted = [](int type, const char* token) -> void
    {
        cout << token << endl;
    };

    nfa->read("a123123+_zhe_shi_id");

    SAFE_RELEASE(nfa)
    return 0;
}
