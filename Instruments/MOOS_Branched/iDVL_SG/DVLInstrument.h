// DVLInstrument.h: interface for the CDVLInstrument class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DVLINSTRUMENT_H__A9401921_A372_4864_959E_C1883F24F90B__INCLUDED_)
#define AFX_DVLINSTRUMENT_H__A9401921_A372_4864_959E_C1883F24F90B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define DVL_BINARY_ENSEMBLE_REPLY_LENGTH 86
#define MAXTRIES 10

class CDVLInstrument : public CMOOSInstrument  
{
public:
    void SetClearToRx(bool bRxOk);
    
    CDVLInstrument();
    virtual ~CDVLInstrument();
    
protected:
    bool m_bClearToRx;
    bool m_bDebug;
    
    double Magnetic2True(double dfMagnetic);
    double True2Yaw(double dfTrueHeading);
    
    
    bool SendDVLCommandGoodReply(std::string &sCommand);
    bool ReadDVLReply(std::string &sReply, double &dfTime, int nWait = 50);
    bool SendDVLCommand(std::string &sCommand);
    void ParseHexRDIReply(std::string &sReply);
    bool ReadHexDVLOutput();
    
    bool ParseRDIReply(std::string & sReply);
    bool InitialiseSensor();
    bool Iterate();
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool OnConnectToServer();
    bool OnStartUp();
    bool GetData();
    bool PublishData();
    
    double m_dfX;
    double m_dfY;
    double m_dfZ;
    double m_dfPitch;
    double m_dfYaw;
    double m_dfSpeed;
    double m_dfHeading;
    
    
private:

    class CHexPD5Reader
    {
	
    public:
        signed char* GetHexBuffer();
        signed char m_pHexBuffer[DVL_BINARY_ENSEMBLE_REPLY_LENGTH];//only supports PD5 mode - 86 chars returned
        void SetHexString(std::string sHexString);
        std::string m_sHexString;
        CHexPD5Reader(std::string sHexString);
        virtual ~CHexPD5Reader();
    private:
        void ParseHexString();
    };

    bool m_bOutputBinary;
    bool SendDVLUpdatedCommands(std::string &sCmdUpdateStatus, std::string sNewCmdString);
    bool StartPinging();
    bool NewCommandsAreSkewed(CMOOSMsg & Msg, double dfTimeNow, std::string sNewCmdString);
    bool GetDVLAttention(bool bHWBreak=true);
    bool ExecuteDVLUpdateCommands(CMOOSMsg & Msg);
    bool IsGoodReply(std::string sReply, std::string sTok = "");

    struct SendTermPair
    {
        SendTermPair(std::string sToSend, std::string sReply) {
            sSend = sToSend;
            sReplyTerm = sReply;
        };

        std::string sSend;
        std::string sReplyTerm;
    };
    
    
    typedef std::list<SendTermPair> TERMPAIRLIST;
    
    void CreateInitialCommands(TERMPAIRLIST &InitCmds);
    
    double m_dfAlignment;
    double m_dfHeadingCorrection;
    double m_dfLastSummary;
    double m_bExposeVelocities;
};

#endif // !defined(AFX_DVLINSTRUMENT_H__A9401921_A372_4864_959E_C1883F24F90B__INCLUDED_)
