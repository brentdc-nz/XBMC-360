// cmdlineargs.h - Command line argument parsing
// Simplified version for XBMC360Tex

#ifndef CMDLINEARGS_H
#define CMDLINEARGS_H

#include <windows.h>
#include <vector>
#include <string>

typedef LPSTR PSZ;

class CmdLineArgs : public std::vector<char*>
{
public:
    CmdLineArgs()
    {
        PSZ cmdline = GetCommandLine();
        m_cmdline = new char[strlen(cmdline) + 1];
        if (m_cmdline)
        {
            strcpy(m_cmdline, cmdline);
            ParseCmdLine();
        }
    }

    CmdLineArgs(const int argc, const char** argv)
    {
        std::string cmdline;
        for (int i = 0; i < argc; i++)
        {
            cmdline += std::string(argv[i]);
            if (i != (argc - 1))
                cmdline += " ";
        }
        m_cmdline = new char[cmdline.length() + 1];
        if (m_cmdline)
        {
            strcpy(m_cmdline, cmdline.c_str());
            ParseCmdLine();
        }
    }

    ~CmdLineArgs()
    {
        delete[] m_cmdline;
    }

private:
    PSZ m_cmdline;

    void ParseCmdLine()
    {
        enum { TERM = '\0', QUOTE = '\"' };
        bool bInQuotes = false;
        PSZ pargs = m_cmdline;

        while (*pargs)
        {
            while (isspace(*pargs))
                pargs++;
            if (*pargs == '\0')
                break;

            if (*pargs == QUOTE)
            {
                pargs++;
                push_back(pargs);
                while (*pargs != '\0' && *pargs != QUOTE)
                    pargs++;
                if (*pargs == QUOTE)
                    *pargs++ = '\0';
            }
            else
            {
                push_back(pargs);
                while (*pargs != '\0' && !isspace(*pargs))
                    pargs++;
            }
            if (*pargs)
                *pargs++ = '\0';
        }
    }
};

#endif // CMDLINEARGS_H
