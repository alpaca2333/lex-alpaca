//
// Created by alpaca on 16-10-27.
//

#include "script.h"
#include "regex.h"

using namespace std;

int Definition::maxid = 0;


Script::Script(const char *scriptPath)
{
    fin = new ifstream(scriptPath);
    fout = new ofstream("scanner.cpp");
    loadScript();
}

Script::Script(const std::string scriptPath) : Script(scriptPath.data()) { }

Script::~Script()
{
    SAFE_RELEASE(fin)
}

void Script::pushToken(std::string token)
{
    string name = token;
    char re[65536];
    fin->getline(re, 65535);
    string regex = re;
    addDefinition(name, regex);
}

void Script::saveAndChange(int mode)
{
    modeStack.push_back(currMode);
    currMode = mode;
}

void Script::changeMode(int mode)
{
    currMode = mode;
}

void Script::restoreMode()
{
    currMode = modeStack.back();
    modeStack.pop_back();
}

void Script::addDefinition(std::string name, std::string regex)
{
    this->defs.push_back(Definition(name, regex));
}

void Script::loadScript()
{
    while (!fin->eof())
    {
        string token;
        *fin >> token;
        pushToken(token);
    }
}

NFA *Script::getNFA()
{
    NFA* result = NULL;
    for (Definition def: defs)
    {
        if (def.regex == "") break;
        Regex re = Regex(def.regex.substr(1).data());
        NFA* tmp = re.getNFA();
        tmp->setEndType(def.id, def.id, def.name);
        if (!result) result = tmp;
        else
        {
            result->parallel(tmp);
        }
    }
    return result;
}

Definition::Definition(std::string name, std::string regex)
{
    this->name = name;
    this->regex = regex;
}
