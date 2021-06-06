#ifndef REMOTEOUTPUT_H_
#define REMOTEOUTPUT_H_



#ifdef FDR_OS_WINDOWS
    #ifdef REMOTE_EXPORTS
    #define EXPORT_CLASS __declspec(dllexport)
    #else
    #define EXPORT_CLASS __declspec(dllimport)
    #endif
#else
    #define EXPORT_CLASS
#endif


#include <string>

namespace ReCBB{

class EXPORT_CLASS RemoteOutput
{
public:
    RemoteOutput(void){}
    virtual ~RemoteOutput(void){}
    virtual void printOut(const std::string& x) =0;
    virtual void printError(const std::string& x) =0;
};


}//namespace ReCBB

#endif



