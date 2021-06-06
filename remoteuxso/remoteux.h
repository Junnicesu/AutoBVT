#ifndef REMOTEUX_H_
#define REMOTEUX_H_

#include <string>
#include <vector>
#include "RemoteData.h"
//#include "remote.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <pty.h>
#include <utmp.h>
#include <stdarg.h>
#include <sys/time.h>

namespace ReCBB{

using std::string;
using std::vector;


#define TRUE    ((int) 1)
#define FALSE   ((int) 0)
#define FUPDATE 	1	/*Full update*/
#define UXENUM		2	/*get version on remote system*/
#define UXPKG		3	/* Update with package */
#define BSIZE		1023	/* buffer size */
#define SBUFF		255	/* std string buffer size 		*/
#define STR_SIZE	512
#define SYSCFG      "SGUIDE/INI/SYSCFG.INI"
#define FLASHIT     "SGUIDE/INI/FLASHIT.INI"
#define CMDFILE		"pqvdisk.cmd"
#define MAXLINE		4096
#define MAXPKG      10
#define BIOSINFO  "biosinfo"
#define EXETIMEOUT 600

struct UXcmd {
    int  updatetype;	/* FUPDATE|UXENUM|UXPKG 		*/
    char pkgname[SBUFF];	/* Name of package (with -P option) 	*/
    char hostname[SBUFF];	/* Target Hostname 			*/
    char username[32];	/* I *hope* no one has a username longer 
                   than 32 char 			*/
    char passwd[SBUFF];	/* password for target system 		*/
    char tmpdir[SBUFF];	/* Local temporary working dir 		*/
    char remdir[SBUFF];	/* Working dir on remote system 	
                   Note: This MUST be ext2/ext3		*/
    char localimg[SBUFF];	/* Dir where UX CD/Image is mounted	*/
    char options[SBUFF];	/* options to be passed to package - used
                   with the -P option			*/
    int verboseflag;	/* verbose/debug option			*/
    int nodos;		/* Commandline nodos flag to disable 
                   common portion of the pq update	*/
};
typedef struct UXcmd UXcmd;

struct node {
    char dname[SBUFF];
    struct node *next;
};
typedef struct node DNode;

enum EFailReason{
    EConnectSuccess     =  0,
    ENoRoutToHost       =  1,
    ENoPermission       =  2,
    EConnectRefused     =  3,
    EHostKeyNoVerified  =  4,
    ENameSvcUnknown     =  5
};

struct syscfg {
    char mtype[5];		/* 4 digit machine type 		*/
    char mname[SBUFF];	/* system full name			*/
//	char dname[SBUFF];	/* Directory name			*/
    DNode *dname;		/* List of directories			*/
    char pkglist[MAXPKG][SBUFF];/* Array of pkgs for pkgbased update    */
    int pkgindex;           /* index for pkg array			*/
    int nodos;		/* Flag to indicate no pq updates	*/
};
typedef struct syscfg syscfg;
  
class RemoteOutput;
class RemoteFacade{
public:
    RemoteFacade(RemoteOutput* log=NULL);
    ~RemoteFacade();
    void doCleanup();
    ERUXError doInitialize(const string &rtSvr, const string &usrName, const string &pwd, const string &rtPath);
    ERUXError doExecute(RemoteOutput *pOutput, const string cmdline,int &returnCode);
    ERUXError doInventory(string &machineType, OSTypeEnum &osType, ArchTypeEnum &archType);
    ERUXError doCopyFileTo(string fileName, bool recurse= true);
    ERUXError doCopyFileBack(string fileName, string localPath, bool recurse= true);
private:
    vector<string> m_listCopied;
    UXcmd m_cmdopts;
    RemoteOutput *m_pRemoteOutput;
    RemoteOutput *m_pRemoteLog;
private:
    int mkDirToRemote(string dirName);
    int deltreeOfRemote(string pathName);
    int isRemoteDir(const char* filename);
//the main functions
    FILE *sshopen( UXcmd, char *rcommand );
    void sshwait(FILE *sshfp, UXcmd cmdopt);
    bool scpfiles(UXcmd, char *fname, char *destdir);
    bool scpfilefromRem(UXcmd cmdopt, char *fname, char *localdir, bool recurse);	
    void uxerror(const char* format, ...);
    void uxdebug(const char* format, ...);
    int getOSType(UXcmd cmdopts);
    int getBiosInfo(UXcmd cmdopts, char* sType);
    int getPPCMTM(UXcmd cmdopts, char* sType);
    int getArchType(UXcmd cmdopts);
//the main functions
    DNode *split ( char *src, char *sep );
    char *chomp( char *src );
    bool m_isConnectFailed;
    EFailReason m_eFailedReason;
};

string getPureName(string strIn);
int doExecute(string &commandLine, int& returnvalue);
int isFile(const char* chFileName);
char* addSlashToPath(char* path);


}//namespace ReCBB

#endif 
