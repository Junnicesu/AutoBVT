#include <iostream>
#include <vector>
#include <stdio.h>
#include <pty.h>

#include "RemoteOutput.h"
#include "RemoteInterface.h"

using namespace std;
using namespace ReCBB;
using ReCBB::RemoteOutput;
using ReCBB::RemoteInterface;


class SimpleOutput : public RemoteOutput{
public:
    SimpleOutput(){
    }
    ~SimpleOutput(){
    }
    virtual void printOut(const string& x){
        fprintf(stdout, "%s", x.c_str());
        fflush(stdout);
    }
    virtual void printError(const string& x){
        fprintf(stderr, "%s", x.c_str());
        fflush(stderr);
    }

};


SimpleOutput out;

extern int mkDirToRemote(string dirName);
extern string getPureName(string strIn);


int main(int argc, char** argv)
{
    if(argc < 5) {
        cout << "Usage:" << endl;
        cout << "./rX.bin <hostname> <user> <password> <remoteTmpDir> [cmdlineToRemote] -t [file1] [file2] -b [file3]..." << endl;
        cout << "Make sure remoteTmpDir exist in the remote machine. Or the remoteux.so will not create one." << endl;
        cout << "The cmdlineToRemote can be system cmd, eg: \"uname -a\", else it need absolute pathname" <<endl;
        cout << "eg: ./rX.bin 9.186.10.156 root ecctest123 /tmp \"/tmp/helloworld -t 10\" ./helloworld /tmp/uxsp /tmp/abc.file" << endl;
        return -1;
    }
    RemoteInterface* rtui= RemoteInterface::getInstance();
    ERUXError errCode= rtui->initialize(argv[1], argv[2], argv[3], argv[4]);

    if(errCode != 0){
        cout << "error accured in initialize(), errCode=" << errCode << endl;
    }

    //get remote machine info 
    string machingTp;
    OSTypeEnum osTp;
    ArchTypeEnum archType;
    rtui->getSystemInfo(machingTp, osTp, archType);  //do the inventory

    switch(archType){
default:
case eNoArch:
    printf("archType=%d", archType);
    printf("The arch type is unknown.\n");
    break;
case e32bit:
    printf("The arch type is ix86.\n");
    break;
case e64bit:
    printf("The arch type is x86_64.\n");
    break;
    }

    switch(osTp){
default:
case eNone:
    printf("The os type is unknown.\n");
    break;
case eRHEL3:
    printf("The os type type is Redhat3.\n");
    break;
case eRHEL4:
    printf("The os type type is Redhat4.\n");
    break;
case eRHEL5:
    printf("The os type type is Redhat5.\n");
    break;
case eRHEL6:
    printf("The os type type is Redhat6.\n");
    break;
case eSLES8:
    printf("The os type type is SuSE8.\n");
    break;
case eSLES9:
    printf("The os type type is SuSE9.\n");
    break;
case eSLES10:
    printf("The os type type is SuSE10.\n");
    break;
case eSLES11:
    printf("The os type type is SuSE11.\n");
    break;
    }

    printf("The machine type is %s\n", machingTp.c_str());

    /*exe the cmd*/	
    int rtReturnCode;
    string cmd("ifconfig;who");
    if(argc >= 6){
        cmd= argv[5];
        if(cmd == "copyBack" || cmd == "copyTo" )
            cmd = string("ifconfig;who");
    }

    vector<string> cpToFileLst;
    vector<string> cpBackFileLst;
    bool isCpTo = false;
    bool isCpBack = false;
    for(int i=6; i< argc; i++){
        if( string(argv[i]) == string("-t") ){
            isCpTo = true; isCpBack = false;
            continue;
        }
        if( string(argv[i]) == string("-b") ){
            isCpTo = false; isCpBack = true;
            continue;
        }
        if(isCpTo){
            cpToFileLst.push_back(string(argv[i]));
        }
        if(isCpBack){
            cpBackFileLst.push_back(string(argv[i]));
        }
    }
    //copy file to Remote
    if(!cpToFileLst.empty()){
        rtui->copyFilesToRemote(cpToFileLst);  
    }

    //no matter what operation is given, execute one command on the target machine.
    rtui->execute(&out, cmd, rtReturnCode);  

    //copy files back to local
    if(!cpBackFileLst.empty()){
        rtui->copyFilesToLocal(cpBackFileLst, "tmp");  
    }  	

    cout << "Remote ReturnCode= " << rtReturnCode << endl;

    delete rtui;
    return 0;
}
