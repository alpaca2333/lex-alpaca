#include "fa.h"
#include "script.h"
#include "regex.h"

using namespace std;

int main(int argc, char** argv)
{
    if (argc < 3) cerr << "Expecting 2 params.\n ['*.l' file path] [to-be-scanned file path]" << endl;
    ifstream *fin = new ifstream(argv[2]);
    string content;
    while(!fin->eof())
    {
        char line[65536];
        fin->getline(line, 65536);
        content += line;
        content += " ";
    }

    Script script(argv[1]);
    NFA* nfa = script.getNFA();
    nfa->onTokenAccepted = [](int type, const char* desc, const char* token)
    {
        cout << desc << ":\t" << token << endl;
    };
    nfa->read(content.data());


    return 0;
}
