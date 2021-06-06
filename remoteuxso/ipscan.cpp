#include <iostream>
#include <vector>
#include <stdio.h>
//#include <pty.h>

#include "RemoteOutput.h"
#include "RemoteInterface.h"

using namespace std;
using namespace ReCBB;
using ReCBB::RemoteOutput;
using ReCBB::RemoteInterface;


class StrOut : public RemoteOutput{
public:
    string output;
    StrOut(){
    }
    ~StrOut(){
    }
    virtual void printOut(const string& x){
        output += x;
    }
    virtual void printError(const string& x){
        output += x;
    }

};


StrOut strs;

extern int mkDirToRemote(string dirName);
extern string getPureName(string strIn);

string osType2str( OSTypeEnum osTp);
string archType2str( ArchTypeEnum archType);

int main(int argc, char** argv)
{
    RemoteInterface* rtui= RemoteInterface::getInstance( &strs);
    ERUXError errCode= rtui->initialize(argv[1], "root", "SYS2009health", "/tmp");
    
    if(errCode != 0){
		return -1;
        cout << "error accured in initialize(), errCode=" << errCode << endl;
    }

    string machingTp;
    OSTypeEnum osTp;
    ArchTypeEnum archType;
    rtui->getSystemInfo(machingTp, osTp, archType);  //do the inventory
	
	string strOSTp, strArchTp;
	strOSTp = osType2str(osTp);
	strArchTp = archType2str(archType);
	
    printf("%15s / %8s / %8s / %8s\n", "IP", "MT", "OS", "Arch");
	printf("%15s / %8s / %8s / %8s\n", argv[1], machingTp.c_str(), strOSTp.c_str(), strArchTp.c_str());
	
	cout << strs.output << endl;
}


string archType2str( ArchTypeEnum archType){
	string out;
    switch(archType){
	default:
	case eNoArch:
		out = "unknown";
		break;
	case e32bit:
		out = "ix86";
		break;
	case e64bit:
		out = "x86_64";
		break;
	}
	return out;
}

string osType2str( OSTypeEnum osTp) {
	string out;
    switch(osTp){
	default:
	case eNone:
		out = "unknown";
		break;
	case eRHEL3:
		out = "Redhat3";
		break;
	case eRHEL4:
		out = "Redhat4";
		break;
	case eRHEL5:
		out = "Redhat5";
		break;
	case eRHEL6:
		out = "Redhat6";
		break;
	case eSLES8:
		out = "SuSE8";
		break;
	case eSLES9:
		out = "SuSE9";
		break;
	case eSLES10:
		out = "SuSE10";
		break;
	case eSLES11:
		out = "SuSE11";
		break;
	}
	return out;
}