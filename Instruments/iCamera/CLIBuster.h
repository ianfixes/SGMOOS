#include <string>
#include <iostream>
#include <tcl8.4/expect.h>

using namespace std;


class CLIBuster 
{
public:

    CLIBuster()
    {
        m_fp = NULL;
    }

    CLIBuster(std::string command)
    {
        CLIBuster();
        m_fp = exp_popen(const_cast<char *>(command.c_str()));
    }

    void init(std::string command)
    {
        if (m_fp) fclose(m_fp);
        m_fp = exp_popen(const_cast<char *>(command.c_str()));
    }

    void send(std::string command)
    {
        fprintf(m_fp, "%s", command.c_str());
        fflush(m_fp);
    }

    void tSend(std::string command)
    {
        m_buffer = "";
        send(command);
    }

    bool waitFor(std::string pattern)
    {
        int ret;
        ret = exp_fexpectl(m_fp, 
                           exp_glob, pattern.c_str(), 0, 
                           exp_end);

        m_buffer += exp_buffer;

        return ret;
    }

    bool waitFor(std::string pattern, int timeout_s)
    {
        bool ret;
        int tmp = exp_timeout;

        exp_timeout = timeout_s;
        waitFor(pattern);
        exp_timeout = tmp;
        return ret;
    }

    bool vWaitFor(std::string pattern, 
                  std::string msg_before,
                  std::string msg_after,
                  std::string msg_fail, 
                  int timeout_s)
    {
        bool ret;
        printf("%s", msg_before.c_str());
        ret = waitFor(pattern, timeout_s);
        printf("%s", ret ? msg_after.c_str() : msg_fail.c_str());
        return ret;
    }

    bool vWaitFor(std::string pattern, 
                  std::string msg_before,
                  std::string msg_after,
                  std::string msg_fail)
    {
        return vWaitFor(pattern, msg_before, msg_after, msg_fail, exp_timeout);
    }
    
    void setTimeout(int timeout_s) 
    {
        exp_timeout = timeout_s;
    }
    
    void setPrompt(std::string prompt_txt) 
    {
        m_prompt = prompt_txt;
    }

    bool waitForPrompt() 
    {
        return waitFor(m_prompt);
    }

    bool waitForPrompt(int timeout_s) 
    {
        return waitFor(m_prompt, timeout_s);
    }

    std::string buffer()
    {
        std::string ret = m_buffer;
        return ret;
    }

    void setDebug(bool yes)
    {
        exp_is_debugging = yes ? 1 : 0;
    }

    void setLogUser(bool yes)
    {
        exp_loguser = yes ? 1 : 0;
    }

    int pid()
    {
        //process id of instance
        return exp_pid;
    }

    bool inBuffer(std::string what)
    {
        return m_buffer.find(what) != string::npos;
    }
    

private:

    FILE* m_fp;
    std::string m_buffer;

    std::string m_prompt;

public:
    static std::string striptags(std::string txt, std::string head, std::string tail)
    {
        size_t p_begin, p_end, p_both;
        
        p_begin = txt.find(head);
        
        if (p_begin == string::npos) return "";
        
        p_both = p_begin + head.size();
        p_end = txt.find(tail, p_both);
        
        if (p_end == string::npos) return "";
        
        return txt.substr(p_both, p_end - p_both);
    }        
};


