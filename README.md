# lex-alpaca

a personal version of lex designed by alpaca.

@author sunhao njuse 141250115

## Regex
It supports the following grammars and their combinations.

| Header One     | Header Two     |
| :-------------: | :-------------: |
| a*$     | repeat 'a' for 0-n times       |
| a+$     | repeat 'a' for 1-n times       |
| a?$    | repeat 'a' for 0-1 times      |
| [abcd]$     | select one of the character      |
| a&#124;b$     | 'a' or 'b'       |
| (a&#124;b)$     | regex within the bracket will be computed first      |
| [a-z]$     | select one character from 'a'-'z'      |
| .$    | accept any character except '\n'     |
| [^abc0-9]$     | accept characters which is not what behind '^'       |

## Usage
Generally, you can create a Regex like:
```c++
Regex re("[a-zA-Z_][a-zA-Z_0-9]*");
```
Since this is not created for use, but for studying instead🌚, the 'Regex' object does not provide functions to test or parse tokeners. Instead, we should get an NFA object from the regex object:
```c++
NFA* nfa = re.getNFA();
```
Be noticed that the nfa is returned in the form of a pointer. So remember to release it after use.
```c++
SAFE_RELEASE(nfa)
```
the nfa object can be combinated with other nfa objects as follows:
```c++
Regex re1("[a-zA-Z_][a-zA-Z_0-9]*");
Regex re2(" ");

NFA* nfa1 = re1.getNFA();
NFA* nfa2 = re2.getNFA();

/* use '|' to connect two NFAs in parallel */
nfa1->parallel(nfa2);		

/* use '?*+' to repeat an NFA
 * The parameter could be taken as :REPEAT_0_N, REPEAT_0_1, REPEAT_1_N */
nfa1->repeat(REPEAT_0_N);

/* directly concat two NFAs */
nfa1->concat(nfa2);
```
This is useful when you want to keep multiple terminal node in the nfa.
If you want to make the nfa meaningful, you can set it's ending type. The first parameter is to identify the ending state. The second parameter is the priority of the ending state. If a char sequence ends at a state which contains multiple ending node, the node with a smaller priority value will be choosed.
```c++
#define TOKEN_IDS 1
nfa->setEndType(TOKEN_ID, 1);
```
There are two ways to use the nfa object to test a char sequence. First is to simply test whether the string matches the regex.
```c++
cout << nfa->matches("123") << endl;
```
Another way is to 'read' a char sequence and use the callback function to tell the nfa what it should do when a token is recognized and an unaccepted character comes up.
```c++
Regex re("[a-zA-Z_][a-zA-Z_0-9]*");

NFA* nfa = re.getNFA();
NFA* nfa2 = new NFA('+');	// create an nfa which accept merely a '+'

nfa->parallel(nfa2);	// use '|' to connect the nfas

nfa->setEndType(TOKEN_ID, 1);	// set different end types
nfa2->setEndType(TOKEN_BLANK, 2);		

nfa->onTokenAccepted = [](int type, const char* token) -> void 	// a function pointer here is ok.
{
    cout << type << " " << token << endl;
};

nfa->read("a123123+_zhe_shi_id");

SAFE_RELEASE(nfa)
SAFE_RELEASE(nfa2)
return 0;
```
The output should be:
```c++
1 a123123
2 +
1 _zhe_shi_id
```

Read the code for further details.

Enjoy hacking :)
