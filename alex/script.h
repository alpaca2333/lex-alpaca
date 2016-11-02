//
// Created by alpaca on 16-10-27.
//

#pragma once

#include "stdafx.h"
#include "fa.h"

class Definition;
class Script;

class SyntaxErrorException : std::exception { };

class Script
{
public:
    Script(const char* scriptPath);

    Script(const std::string scriptPath);

    virtual ~Script();

    void addDefinition(std::string name, std::string regex);

    NFA* getNFA();

protected:
    void loadScript();

    void pushToken(std::string token);

    std::ifstream *fin;

    std::ofstream *fout;

    int currMode = DEFINITION;

    static const int DEFINITION = 1;
    static const int AFX = 100;
    static const int ACTION = 2;
    static const int SFX = 3;

    std::vector<int> modeStack;

    void saveAndChange(int mode);

    void changeMode(int mode);

    void restoreMode();

    std::vector<Definition> defs;
};

class Definition
{
public:
    Definition() {}
    Definition(std::string name, std::string regex);

    std::string name = "";
    int id = ++maxid;
    std::string regex;

protected:
    static int maxid;
};