/*
 *
 * Author: Peter Donovan
 *
 */

/*
 *
 * RemoteUX.h
 *
 */

#ifndef REMOTEUX_H_INCLUDED
#define REMOTEUX_H_INCLUDED



#include <windows.h>
#include <vector>
#include <tchar.h>

// NLS support
#include "GetResrc.hpp"
#include "UXcommon.h"
#include "RemoteData.h"

namespace ReCBB{

class RemoteOutput;

class RemoteFacade
{

public:
    /**
     * Constructor.
     */
    RemoteFacade(RemoteOutput* log = NULL);
    /**
     * Destructor.
     */
    ~RemoteFacade();
    /**
     * Clean up what we copied on the remote machine.
     */
    void doCleanup();
    /**
     * Connect to remote machine
     * Install and start service on remote machine
     * @param rtSvr Remote machine's IP address or DNS name.
     * @param usrName User name for logining to remote machine.
     * @param pwd Password for logining to remote machine.
     * @param rtPath Path of working directory on the remote machine.                     
     */
    ERUXError doInitialize(const std::string &rtSvr, const std::string &usrName, const std::string &pwd, const std::string &rtPath);
    /**
     * Execute UXLite remotely on the remote machine
     * @param pOutput A pointer to an instance of Class RemoteOutput.
     * @param cmdline Command that client inputs on the command line.
     * @param returnCode ReturnCode that the application executed on the remote machine.                
     */    
    ERUXError doExecute(RemoteOutput *pOutput, const std::string &cmdline,int &returnCode);
    /**
     * Get system information of the remote machine, such as machine type, OS type, and architeture type.
     */
    ERUXError doInventory(std::string &machineType, OSTypeEnum &osType, ArchTypeEnum &archType);
    /**
     * Copy one file from local machine to remote machine.
     */	    
    ERUXError doCopyFileTo(const std::string &fileName, bool recurse= true);
    /**
     * Copy one file from remote machine to local machine.
     */	    
    ERUXError doCopyFileBack(const std::string &fileName, const std::string &localPath, bool recurse= true);
    

private:
    TCHAR szThisMachine[STR_SIZE];

    TCHAR szRemoteMachine[STR_SIZE];
    TCHAR szPassword[STR_SIZE];
    TCHAR szUser[STR_SIZE];
    TCHAR szWorkingDir[STR_SIZE];
    TCHAR szType[STR_SIZE];

    LPCTSTR lpszMachine;
    LPCTSTR lpszPassword;
    LPCTSTR lpszUser;
    LPCTSTR lpszWorkingDir;
    LPCTSTR lpszType;

    LPCTSTR lpszCommandExe;
    LPCTSTR lpszLocalDir;


    BOOL bNoWait;

    // The operating system of the remote machine
    OSTypeEnum g_operatingSystem;
    ArchTypeEnum g_architectureType;
    int g_remoteReturnCode;

    RemoteOutput *pRemoteOutput;
    RemoteOutput *pRemoteLog;


    HANDLE hCmdPipe;
    HANDLE hRemOutPipe;
    HANDLE hRemStdInPipe;
    HANDLE hRemStdErrPipe;

    // Use this for debugging output
    TCHAR tmpBuf[1024];

    // General status run life of program
    BOOL isSuccess;

    // Are the pipes connected
    BOOL isConnected;

    // Is the remote service alive.  
    BOOL isRemoteAlive;

    // Should the remote machine reboot
    BOOL shouldReboot;

    BOOL logUxEvents;

    /**
     * NLS support
     */
    // Create object needed to return strings.
    GetResource GetString;

    /**
     * Primitive semaphore to detect if displaying firmware version information
     * Use this because printf is not reentrant and we don't want to simultaneously call printf
     * from ListenRemoteStdOutput() thread.
     */
    int g_DisplayVersionComp;
    std::vector<std::string> m_copiedVec;

private:

    DWORD DisplayErrorMessage();
    /**
     * Get the first parameter which is the remote server name
     */
    LPCTSTR GetRemoteMachineName(); 
    /**
     * rename to stop service if running
     */
    BOOL StopServiceIfRunning (void);
    /**
     * Connect to the remote machine
     */
    ERUXError RuxConnect(LPCTSTR lpszRemote, LPCTSTR lpszResource); 
    /**
     * Disconnect from the remote machine
     * Return TRUE if success
     */
    BOOL RuxDisconnect(LPCTSTR lpszRemote, LPCTSTR lpszResource);
    BOOL PathExists(LPTSTR lpszCur);
    /**
     * Check if the OS is 64-bit.
     */
    BOOL Is64BitOS();
    /**
     * Copy the service executable to remote machine's System32/SysWOW64 directory
     */
    BOOL CopySvcExeToRemoteMachine(TCHAR *szSvcExePath);
    /**
     * Delete the ruxSvc.exe from the remote machine when we are done
     */
    BOOL DeleteFileOnRemoteMacine(char *fileToDelete);
    /**
     * Install and start the service on the remote machine
     */
    BOOL InstallAndStartRemoteService();
    /**
     * Connect to the remote service
     * @param dwRetry Times for which Remoteux may try to connect.
     * @param dwRetryTimeOut Time for which Remoteux may sleep while connecting each time.
     */
    BOOL ConnectRemoteService(DWORD dwRetry, DWORD dwRetryTimeOut);
    /**
     * Format the communication message structure
     * This structure is sent to the remote machine
     * @param pMsg Pointer to message structure.
     * @param cmd Command type.
     */
    BOOL FormatMsg(rUxMessage* pMsg, DWORD cmd);
    /**
     * Set up to listen on remote Std I/O pipes
     */
    void ListenOnRemoteStdIoPipes();
    /**
     * Connect to the remote Std Out and Err pipes, including 3 pipes: stdOut, stdErr and stdIn.
     * @param dwRetryCount Times for which Remoteux may try to connect.
     * @param dwRetryTimeOut Time for which Remoteux may sleep while connecting each time.
     */
    BOOL ConnectRemoteStdOutPipe(DWORD dwRetryCount, DWORD dwRetryTimeOut);
    BOOL ConnectRemoteStdErrPipe(DWORD dwRetryCount, DWORD dwRetryTimeOut);
    /**
     * This function will do:
     * (1) Send the message to remote service
     * (2) Connects to remote pipes
     * (3) Waiting for finishing remote process
     * @param cmd Command type
     */
    ERUXError RunRemoteCmd(DWORD cmd);
    void LogInfo(const char * format, ...);
    void LogErr(const char * format, ...);



public:

friend void ListenRemoteStdOutputPipeThread(void *p); 
friend void ListenRemoteStdInputPipeThread(void *p);
friend void ListemRemoteStdErrorPipeThread(void *p);

// Use these flags for controlling the network broken down.
BOOL beExitThread; //when finish doExecute, it will be TRUE.
BOOL flagOutputThread; //when listen output pipe thread exited, it will be TRUE.
BOOL flagErrorThread;  //when listen error pipe thread exited, it will be TRUE.

};


typedef	WIN32_FIND_DATA	FInfo;
/**
 * Analys the string Path name, split the name with the path. 
 * Return the pure filename,eg. c:\dir\subdirA\a.txt ->  a.txt & c:\dir\subdirA\
 */
std::string GetPureName(std::string inPathName, std::string& purePath);
/**
 * Get the sub dir and files under "path" recursively if "recurse" is true. 
 * The return value is like this dir/subdirA1/subdirB2/a.file
 */
void GetSubDir(std::vector<std::string>& dirVec, std::vector<std::string>& fileVec,std::string path, std::string fatherPath, bool recurse);
/**
 * check the dirname, add slash to it if there is not slash at the end.
 */
void CheckDir(std::string& dir);
/**
 * create the the subdir by name like ".\dir\subdirA\subdirB" or "X:\dir\subdir"
 */
bool CreatSubDirs(std::vector<std::string>& dirVec, std::string destPath);
/**
 * Change the file ReadOnly Attribute if it exists.
 */
void ChangeROAttrIfExist(std::string file);
/**
 * Change the local made path to remote path, eg. \\10.0.0.1\x:\shared\ -> \\10.0.0.1\x$\shared\ ...
 */
std::string MkRemotePathName(std::string name);
/**
 * Delete a directory recursively.
 */
bool DeleteTree(std::string sFileName);
void CheckSlash(std::string& path);
}//namespace ReCBB

#endif
