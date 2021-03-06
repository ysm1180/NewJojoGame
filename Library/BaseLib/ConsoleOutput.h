#pragma once

#include <string>
#include <vector>

namespace jojogame
{
class CConsoleOutput
{
public:
    CConsoleOutput();
    virtual ~CConsoleOutput();

    virtual void Output(std::wstring msg) = 0;

    static void RegisterConsole(CConsoleOutput *console);

    static void OutputConsoles(std::wstring msg);

private:
    static std::vector<CConsoleOutput *> _consoles;
};
} // namespace jojogame