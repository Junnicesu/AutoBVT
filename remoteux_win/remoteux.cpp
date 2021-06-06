/*
 *
 * Author: Peter Donovan
 *
 */

/*
 *
 * RemoteUX.cpp
 *
 */

#include "RemoteUX.h"
#include <winsvc.h>
#include <tchar.h>
#include <lmcons.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <iostream> 
#include <shellapi.h>
#include <Dbghelp.h>
#include "resource.h"
#include "RemoteInterface.h"
#include "RemoteOutput.h"
#include <time.h>

using namespace std;

namespace ReCBB
{ 
/**
 * Listen on the remote stdout pipe
 * Remote process will send its stdout to this pipe, in general we just echo
 * what we receive to this local console's stdout.
 * When necessary set state information and parse the received buffer, such as
 * when reading ServeRAID information.
 * Keep listening until we receive a STOP_COMMAND.
 */

void ListenRemoteStdOutputPipeThread(void *p) 
{
    TCHAR szBuffer[BUFFERSIZE] = {0};
    DWORD dwRead;

    RemoteFacade *pRemoteFacade = (RemoteFacade *)p;

    if(!pRemoteFacade->ConnectRemoteStdOutPipe(30, 5000)){
        //failed to connect stdout pipe.
        pRemoteFacade->flagOutputThread = TRUE;
        ::ExitThread(0);
    }

    for (;;) 
    {
        /**
         * JRH - zero out the buffer before populating it so we don't have
         * garbage values left over from previous lines.
         */
        if( pRemoteFacade->beExitThread == TRUE){
            break;
        }

        memset(szBuffer, 0, sizeof(szBuffer));

        // Peek first
        DWORD bytes_avail = 0;
        int rc = PeekNamedPipe (pRemoteFacade->hRemOutPipe, NULL, 0, NULL, &bytes_avail, NULL);
        if (!rc) {
            DWORD dwErr = GetLastError();
            if ((dwErr == ERROR_BROKEN_PIPE) || (dwErr == ERROR_NETNAME_DELETED) || (dwErr == ERROR_UNEXP_NET_ERR))
            {
                CloseHandle(pRemoteFacade->hRemOutPipe);
                pRemoteFacade->hRemOutPipe = INVALID_HANDLE_VALUE;
                pRemoteFacade->LogErr(_T(pRemoteFacade->GetString.ReturnString(RUX_DEBUG_OUT_PIPE_BROKEN, NULL)));
                if ( pRemoteFacade->ConnectRemoteStdOutPipe(30, 5000) )
                {
                    continue;
                }
                else
                {
                    break;
                }
            }
        }
        if (bytes_avail <= 0) {
            Sleep(200);
            continue;
        }
        // Read data coming back from service, break out of thread if there is an error
        if (!ReadFile(pRemoteFacade->hRemOutPipe, szBuffer, sizeof(szBuffer), &dwRead, NULL) || dwRead == 0) 
        {
            DWORD dwErr = GetLastError();
            if (dwErr == ERROR_NO_DATA){
                CloseHandle(pRemoteFacade->hRemOutPipe);
                pRemoteFacade->hRemOutPipe = INVALID_HANDLE_VALUE;
                break;
            }
                
            // Check to see if we have a broken pipe (could be one of many return codes)
            else if ((dwErr == ERROR_BROKEN_PIPE) || (dwErr == ERROR_NETNAME_DELETED) || (dwErr == ERROR_UNEXP_NET_ERR)) 
            {
                CloseHandle(pRemoteFacade->hRemOutPipe);
                pRemoteFacade->hRemOutPipe = INVALID_HANDLE_VALUE;
                pRemoteFacade->LogErr(_T(pRemoteFacade->GetString.ReturnString(RUX_DEBUG_OUT_PIPE_BROKEN, NULL)));
                if ( pRemoteFacade->ConnectRemoteStdOutPipe(30, 5000) )
                {
                    continue;
                }
                else
                {
                    break;
                }
            } 
            else 
            {
                pRemoteFacade->LogErr(_T(pRemoteFacade->GetString.ReturnString(RUX_DEBUG_OUT_PIPE_ERROR, NULL)), dwErr);
                CloseHandle(pRemoteFacade->hRemOutPipe);
                pRemoteFacade->hRemOutPipe = INVALID_HANDLE_VALUE;
                break;
            }
        }

		pRemoteFacade->isRemoteAlive = true;  

        //szBuffer[dwRead / sizeof(TCHAR)] = _T('\0');
        szBuffer[dwRead + 1] = 0;

        // See if the service is telling us to stop
        if (!_stricmp(szBuffer, STOP_COMMAND)) 
        {
            pRemoteFacade->LogInfo(_T(pRemoteFacade->GetString.ReturnString(RUX_DEBUG_OUT_PIPE_QUIT, NULL)));
            break;
        } 
        else 
        {
            if( pRemoteFacade->pRemoteOutput != NULL )
                pRemoteFacade->pRemoteOutput->printOut(szBuffer);
        }
    }

    pRemoteFacade->flagOutputThread = TRUE;
    ::ExitThread(0);
}

/**
 * Listen on the Remote StdError pipe
 * Similar to ListenRemoteStdOutputPipeThread, although only used for error
 * streams.
 */
void ListemRemoteStdErrorPipeThread(void *p) 
{
    TCHAR szBuffer[STR_SIZE] = {0};
    DWORD dwRead;

    RemoteFacade *pRemoteFacade = (RemoteFacade *)p;
    if(!pRemoteFacade->ConnectRemoteStdErrPipe(30, 5000)){
        //failed to connect stderr pipe.
        pRemoteFacade->flagErrorThread = TRUE;
        ::ExitThread(0);
    }

    for (;;) 
    {

        if( pRemoteFacade->beExitThread == TRUE){
            break;
        }

        // Peek first
        DWORD bytes_avail = 0;
        int rc = PeekNamedPipe (pRemoteFacade->hRemStdErrPipe, NULL, 0, NULL, &bytes_avail, NULL);
        if (!rc) {
            DWORD dwErr = GetLastError();
            if ((dwErr == ERROR_BROKEN_PIPE) || (dwErr == ERROR_NETNAME_DELETED) || (dwErr == ERROR_UNEXP_NET_ERR))
            {
                CloseHandle(pRemoteFacade->hRemStdErrPipe);
                pRemoteFacade->hRemStdErrPipe = INVALID_HANDLE_VALUE;
                pRemoteFacade->LogErr(_T(pRemoteFacade->GetString.ReturnString(RUX_DEBUG_ERR_PIPE_BROKEN, NULL)));
                if ( pRemoteFacade->ConnectRemoteStdErrPipe(30, 5000) )
                {
                    continue;
                }
                else
                {
                    break;
                }
            }
        }
        if (bytes_avail <= 0) {

            Sleep(200);
            continue;
        }
        // Read data from the service, break out of thread if there is an error
        if (!ReadFile( pRemoteFacade->hRemStdErrPipe, szBuffer, STR_SIZE, &dwRead, NULL) || dwRead == 0) 
        {
            DWORD dwErr = GetLastError();
            if (dwErr == ERROR_NO_DATA){
                CloseHandle(pRemoteFacade->hRemStdErrPipe);
                pRemoteFacade->hRemStdErrPipe = INVALID_HANDLE_VALUE;
                break;
            }
            // Check to see if we have a broken pipe (could be one of many return codes)
            else if ((dwErr == ERROR_BROKEN_PIPE) || (dwErr == ERROR_NETNAME_DELETED) || (dwErr == ERROR_UNEXP_NET_ERR)) 
            {
                CloseHandle(pRemoteFacade->hRemStdErrPipe);
                pRemoteFacade->hRemStdErrPipe = INVALID_HANDLE_VALUE;
                pRemoteFacade->LogErr(_T(pRemoteFacade->GetString.ReturnString(RUX_DEBUG_ERR_PIPE_BROKEN, NULL)));
                if ( pRemoteFacade->ConnectRemoteStdErrPipe(30, 5000) )
                {
                    continue;
                }
                else
                {
                    break;
                }
            } 
            else 
            {
                CloseHandle(pRemoteFacade->hRemStdErrPipe);
                pRemoteFacade->hRemStdErrPipe = INVALID_HANDLE_VALUE;
                break;
            }
        }

		pRemoteFacade->isRemoteAlive = true;  

        szBuffer[dwRead / sizeof(TCHAR)] = _T('\0');

        // See if the service is telling us to stop
        if (!_stricmp(szBuffer, STOP_COMMAND)) 
        {
            pRemoteFacade->LogInfo(_T(pRemoteFacade->GetString.ReturnString(RUX_DEBUG_ERR_PIPE_QUIT, NULL)));
            break;
        } 
        else 
        {
            // Write it to our stderr
            if( pRemoteFacade->pRemoteOutput != NULL )
                pRemoteFacade->pRemoteOutput->printError(szBuffer);
        }
    }

    pRemoteFacade->flagErrorThread = TRUE;
    ::ExitThread(0);
}

RemoteFacade::RemoteFacade(RemoteOutput* log ) 
: pRemoteLog(log) 
{

    memset(szThisMachine,0,sizeof(szThisMachine)); 
    memset(szRemoteMachine,0,sizeof(szRemoteMachine)); 
    memset(szPassword,0,sizeof(szPassword)); 
    memset(szUser,0,sizeof(szUser)); 

    memset(szWorkingDir,0,sizeof(szWorkingDir)); 
    _tcscpy_s(szWorkingDir, STR_SIZE, _T("C$\\TEMP"));

    memset(szType,0,sizeof(szType)); 

    lpszMachine = szRemoteMachine;
    lpszPassword = szPassword;
    lpszUser = szUser;
    lpszWorkingDir = szWorkingDir;
    lpszType = szType;

    lpszCommandExe = NULL;
    lpszLocalDir = ".\\";


    bNoWait = FALSE;

    // The operating system of the remote machine
    g_operatingSystem = eNone;
    g_architectureType = eNoArch;
    g_remoteReturnCode = 0;

    pRemoteOutput = NULL;


    hCmdPipe = INVALID_HANDLE_VALUE;
    hRemOutPipe = INVALID_HANDLE_VALUE;
    hRemStdInPipe = INVALID_HANDLE_VALUE;
    hRemStdErrPipe = INVALID_HANDLE_VALUE;

     // General status run life of program
    isSuccess = TRUE;

    // Are the pipes connected
    isConnected = FALSE;

    // Should the remote machine reboot
    shouldReboot = FALSE;

    logUxEvents = TRUE; 

    /**
     * Primitive semaphore to detect if displaying firmware version information
     * Use this because printf is not reentrant and we don't want to 
     * simultaneously call printf from ListenRemoteStdOutput() thread.
     */
    g_DisplayVersionComp = FALSE;

    beExitThread = FALSE; 
    flagOutputThread = FALSE;
    flagErrorThread = FALSE;

}

RemoteFacade::~RemoteFacade() 
{

}

// Show the last error's description
DWORD RemoteFacade::DisplayErrorMessage() 
{
    LPVOID lpMsgBuf;
    DWORD rc = GetLastError();

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, rc, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) &lpMsgBuf, 0, NULL);

    LogErr((LPCTSTR)lpMsgBuf);
    
    LogErr(_T(GetString.ReturnString(RUX_LINE_FEED, NULL)));

    LocalFree (lpMsgBuf);
    return rc;
}



// Get the first parameter which is the remote server name
LPCTSTR RemoteFacade::GetRemoteMachineName() 
{
    if (__argc <= 1) 
    {
        return NULL;
    }
    LPCTSTR lpszMachine = __targv[1];

    if (lpszMachine == NULL) 
    {
        return NULL;
    }

    if (_tcsnicmp( lpszMachine, _T("\\\\"), 2 ) == 0) 
    {
        return lpszMachine;
    }
    return NULL;
}


//rename to stop service if running
BOOL RemoteFacade::StopServiceIfRunning (void) 
{
    BOOL rc = FALSE;
    // Open Service Manager on remote machine
    SC_HANDLE hSCM = ::OpenSCManager(lpszMachine, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM == NULL){
        rc =  FALSE; 
    }

    // If Service is already installed,
    SC_HANDLE hService =::OpenService(hSCM, SERVICENAME, SERVICE_ALL_ACCESS);


    if (hService == NULL) 
    {
        rc =  FALSE;
        goto close_handles;
    }
    else
    {
        SERVICE_STATUS_PROCESS ssp;
        DWORD dwStartTime = GetTickCount();
        DWORD dwBytesNeeded;
        DWORD dwTimeout = 30000; // 30-second time-out
        DWORD dwWaitTime;

        if( !QueryServiceStatusEx( hService, SC_STATUS_PROCESS_INFO, 
                                            (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), 
                                            &dwBytesNeeded ) )
        {
            goto close_handles;
        }

        if ( ssp.dwCurrentState == SERVICE_STOPPED )
        {
            goto close_handles;
        }

        while ( ssp.dwCurrentState == SERVICE_STOP_PENDING ) {
            dwWaitTime = ssp.dwWaitHint / 10;
            
            if( dwWaitTime < 1000 )
                dwWaitTime = 1000;
            else if ( dwWaitTime > 10000 )
                dwWaitTime = 10000;

            Sleep( dwWaitTime );

            if( !QueryServiceStatusEx( hService, SC_STATUS_PROCESS_INFO, 
                                                (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), 
                                                &dwBytesNeeded ) )
            {
                goto close_handles;
            }

            if ( ssp.dwCurrentState == SERVICE_STOPPED )
            {
                goto close_handles;
            }

            if ( GetTickCount() - dwStartTime > dwTimeout )
            {
                goto close_handles;
            }
        }

        if ( !ControlService( hService, SERVICE_CONTROL_STOP, 
                                   (LPSERVICE_STATUS) &ssp ) )
        {
            goto close_handles;
        }

        while ( ssp.dwCurrentState != SERVICE_STOPPED ) {
            Sleep( ssp.dwWaitHint );
            if( !QueryServiceStatusEx( hService, SC_STATUS_PROCESS_INFO, 
                                                (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), 
                                                &dwBytesNeeded ) )
            {
                goto close_handles;
            }

            if ( ssp.dwCurrentState == SERVICE_STOPPED ){
                break;
            }

            if ( GetTickCount() - dwStartTime > dwTimeout )
            {
                goto close_handles;
            }
        }
    }
 
close_handles:
    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);

    return rc;
}


// Connect to the remote machine
ERUXError RemoteFacade::RuxConnect(LPCTSTR lpszRemote, LPCTSTR lpszResource) 
{
    TCHAR szRemoteResource[_MAX_PATH] = {0};
    DWORD rc;

    // Remote resource, \\remote\ipc$, remote\admin$, ...
    _stprintf_s(szRemoteResource, _MAX_PATH, _T("%s\\%s"), lpszRemote, lpszResource);

    // Connect to the resource
    NETRESOURCE netResource;
    netResource.dwType = RESOURCETYPE_ANY;
    netResource.lpLocalName = NULL;
    netResource.lpRemoteName = (LPTSTR)&szRemoteResource;
    netResource.lpProvider = NULL;

    /**
     * Connect using username/pwd, okay for these to be null.
     * Note: These are sent a plain text, better to authenticate outside of this
     * application.
     */
    rc = WNetAddConnection2(&netResource, lpszPassword, lpszUser, FALSE);
    switch( rc )
    {
    case NO_ERROR:
        return REMOTE_SUCCESS;
    case ERROR_ACCESS_DENIED:
    case ERROR_INVALID_PASSWORD:
    case ERROR_LOGON_FAILURE:
        SetLastError(rc);
        return ERROR_REMOTE_AUTHENTICATE;
    default:
        SetLastError(rc);
        return ERROR_REMOTE_CONNECT;
    }

}
/**
 * Disconnect from the remote machine
 * Return TRUE if success
 */
BOOL RemoteFacade::RuxDisconnect(LPCTSTR lpszRemote, LPCTSTR lpszResource) 
{
    TCHAR szRemoteResource[_MAX_PATH] = {0};
    DWORD rc;

    // Remote resource, \\remote\ipc$, remote\admin$, ...
    _stprintf_s(szRemoteResource, _MAX_PATH, _T("%s\\%s"), lpszRemote, lpszResource);

    // disconnect - force a close even if jobs are open
    rc = WNetCancelConnection2(szRemoteResource, 0, true);

    if (rc == NO_ERROR)
        return TRUE;

    SetLastError(rc);
    return FALSE;
}




BOOL RemoteFacade::Is64BitOS()
{
    TCHAR szSysWOW64Path[_MAX_PATH] = {0};

        _stprintf_s(szSysWOW64Path, _MAX_PATH, _T("%s\\ADMIN$\\SysWOW64"), lpszMachine);
        HANDLE hDirSysWOW64 = CreateFile(szSysWOW64Path,
                                                                         GENERIC_READ,
                                                                     FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_FLAG_BACKUP_SEMANTICS,
                                     NULL);
        if(hDirSysWOW64 == INVALID_HANDLE_VALUE)
        {
                CloseHandle(hDirSysWOW64);
                return false;
        }

        CloseHandle(hDirSysWOW64);
        return true;
}

// Copy the service executable to remote machine's System32/SysWOW64 directory
BOOL RemoteFacade::CopySvcExeToRemoteMachine(TCHAR *szSvcFile ) 
{
    BOOL ret = FALSE;
	TCHAR *szFileName = szSvcFile;
	
    TCHAR szSvcExePath[_MAX_PATH] = {0};
    _stprintf_s(szSvcExePath, _MAX_PATH, _T("%s\\ADMIN$\\System32\\%s"), lpszMachine, szFileName);

    // Copy the service executable to \\remote\ADMIN$\System32
    ret = CopyFile(szFileName, szSvcExePath, FALSE);
    if( !ret ){
        LogErr(_T(GetString.ReturnString(RUX_COPY_SERVICE_FAIL, NULL)), lpszMachine);
        DisplayErrorMessage();
        isSuccess = false;
        return ret;        
    }
    
    // For 64-bit OS we copy it to a second place also
    if(Is64BitOS()) {
        TCHAR szSvcExePath64[_MAX_PATH] = {0};
        _stprintf_s(szSvcExePath64, _MAX_PATH, _T("%s\\ADMIN$\\SysWOW64\\%s"), lpszMachine, szFileName);
        ret = CopyFile(szFileName, szSvcExePath64, FALSE);
        if ( !ret ){ 
            LogErr(_T(GetString.ReturnString(RUX_COPY_SERVICE_FAIL, NULL)), lpszMachine);
            DisplayErrorMessage();
            isSuccess = false;
            return ret;
        }
    }
	    
    return ret;
}

// Delete the ruxSvc.exe from the remote machine when we are done
BOOL RemoteFacade::DeleteFileOnRemoteMacine(char *fileToDelete) 
{
    TCHAR szFileToDelete[_MAX_PATH] = {0};
    BOOL retval;

    _stprintf_s(szFileToDelete, _MAX_PATH, _T("%s\\ADMIN$\\System32\\%s"), lpszMachine, fileToDelete);
    retval = DeleteFile(szFileToDelete);

    if(Is64BitOS()) 
    {
        _stprintf_s(szFileToDelete, _MAX_PATH, _T("%s\\ADMIN$\\SysWOW64\\%s"), lpszMachine, fileToDelete);
        retval = DeleteFile(szFileToDelete);
    }
    
    if(!retval){
      TCHAR szFileToDeleteTmp[_MAX_PATH]= {0};
      _tcscpy_s(szFileToDeleteTmp, sizeof(szFileToDeleteTmp), szFileToDelete);
      TCHAR tmpNum[128] = {0};
      srand( (unsigned int)time( NULL ) );
      int RANGE_MAX = 9999;
      _stprintf_s(tmpNum, sizeof(tmpNum), _T(".tmp%04d"), rand()%RANGE_MAX );
      _tcscat_s(szFileToDeleteTmp, sizeof(szFileToDeleteTmp),tmpNum);
      ::MoveFile(szFileToDelete, szFileToDeleteTmp);
    }
    // In case the file can not be delete, rename it in order that it will not stop the copytoremote next time.
    //it's just a temporary solution. We will find why can't DeleteFile finnally.

    return retval;
}

// Install and start the service on the remote machine
BOOL RemoteFacade::InstallAndStartRemoteService() 
{
    // Open Service Manager on remote machine
    SC_HANDLE hSCM = ::OpenSCManager(lpszMachine, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM == NULL)
        return FALSE;

    // If Service is already installed, try to run it
    SC_HANDLE hService =::OpenService(hSCM, SERVICENAME, SERVICE_ALL_ACCESS);

    // Service is not present on remote machine
    if (hService == NULL) 
    {
        // Install service
        hService = ::CreateService(hSCM, SERVICENAME, LONGSERVICENAME,
                                   SERVICE_ALL_ACCESS,
                                   SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
                                   SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
                                   _T("%SystemRoot%\\system32\\")RUXSVCEXE,
                                   NULL, NULL, NULL, NULL, NULL );
    }

    if (hService == NULL) 
    {
        ::CloseServiceHandle(hSCM);
        return FALSE;
    }

    // Start service
    if (!StartService(hService, 0, NULL)) 
    {
        ::CloseServiceHandle(hSCM);
        return FALSE;
    }

    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);

    return TRUE;
}

// Connect to the remote service
BOOL RemoteFacade::ConnectRemoteService(DWORD dwRetry, DWORD dwRetryTimeOut) 
{
    TCHAR szPipeName[_MAX_PATH] = _T("");

    // Remote service communication pipe name
    _stprintf_s(szPipeName, _MAX_PATH, _T("%s\\pipe\\%s"), lpszMachine, RUXCOMM);

    SECURITY_ATTRIBUTES securityAtt = {0};
    SECURITY_DESCRIPTOR securityDescr;
    InitializeSecurityDescriptor(&securityDescr, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&securityDescr, TRUE, NULL, TRUE);

    securityAtt.nLength = sizeof(SECURITY_ATTRIBUTES);
    securityAtt.lpSecurityDescriptor = &securityDescr;;
    securityAtt.bInheritHandle = TRUE;

    // Connect to the remote service's communication pipe
    while (dwRetry--) 
    {
        if (WaitNamedPipe( szPipeName, dwRetryTimeOut )) 
        {
            hCmdPipe = CreateFile(szPipeName,
                                  GENERIC_WRITE | GENERIC_READ,
                                  0,
                                  &securityAtt,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL);
            isConnected = TRUE;
            break;
        } 
        else 
        {
            
            // Let's try it again
            Sleep( dwRetryTimeOut );
        }
    }

    return isConnected;
}


/**
 * Format the communication message structure
 * This structure is sent to the remote machine
 */
BOOL RemoteFacade::FormatMsg(rUxMessage* pMsg, DWORD cmd)
{
    pMsg->dwProcessId = GetCurrentProcessId();
    _tcscpy_s(pMsg->szMachine, _MAX_PATH, szThisMachine);
    pMsg->dwCommandType = cmd;
    pMsg->dwOutputBufSize = STR_SIZE;


    _stprintf_s(pMsg->szCommand, 1000, _T("%s"), lpszCommandExe);


    // Priority
    pMsg->dwPriority = NORMAL_PRIORITY_CLASS;

    // No wait
    pMsg->bNoWait = bNoWait;

    // Log events?
    pMsg->logUxEvents = logUxEvents;

    // Working directory
    if (lpszWorkingDir != NULL)
        _tcscpy_s(pMsg->szWorkingDir, _MAX_PATH, lpszWorkingDir);


    return TRUE;
}




// Set up to listen on remote Std I/O pipes
void RemoteFacade::ListenOnRemoteStdIoPipes() 
{
    _beginthread(ListenRemoteStdOutputPipeThread, 0,  (void *)this );
    _beginthread(ListemRemoteStdErrorPipeThread, 0, (void *)this );
}

//connect with remote stdout pipe
BOOL RemoteFacade::ConnectRemoteStdOutPipe(DWORD dwRetryCount, DWORD dwRetryTimeOut) 
{
    TCHAR szStdOut[_MAX_PATH] = {0};

    SECURITY_ATTRIBUTES securityAtt = {0};
    SECURITY_DESCRIPTOR securityDescr;
    InitializeSecurityDescriptor(&securityDescr, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&securityDescr, TRUE, NULL, FALSE);

    securityAtt.nLength = sizeof(SECURITY_ATTRIBUTES);
    securityAtt.lpSecurityDescriptor = &securityDescr;;
    securityAtt.bInheritHandle = TRUE;

    hRemOutPipe = INVALID_HANDLE_VALUE;

    // StdOut pipe name
    _stprintf_s(szStdOut, _MAX_PATH, _T("%s\\pipe\\%s%s%d"), lpszMachine, RUXSTDOUT,
              szThisMachine, GetCurrentProcessId());

    while ( dwRetryCount--) {
        if (hRemOutPipe == INVALID_HANDLE_VALUE) 
        {
            if (WaitNamedPipe(szStdOut, dwRetryTimeOut)) 
            {
                LogInfo(_T(GetString.ReturnString(RUX_DEBUG_WAIT_STDOUT, NULL)));
  
                hRemOutPipe = CreateFile(szStdOut, GENERIC_READ, 0,
                                         &securityAtt, OPEN_EXISTING,
                                         FILE_ATTRIBUTE_NORMAL, NULL);
            } 
            else 
            {
                LogInfo(_T(GetString.ReturnString(RUX_DEBUG_WAIT_STDOUT_ERROR, NULL)), GetLastError());
                hRemOutPipe = CreateFile(szStdOut, GENERIC_READ, 0,
                                         &securityAtt, OPEN_EXISTING,
                                         FILE_ATTRIBUTE_NORMAL, NULL);       				

                
            }
            if( beExitThread ){
                return FALSE;
            }
            Sleep(dwRetryTimeOut);
        }

        if (hRemOutPipe != INVALID_HANDLE_VALUE) 
        {
            LogInfo(_T(GetString.ReturnString(RUX_DEBUG_VALID_HANDLES, NULL)));
            break;
        }
    }

    if (hRemOutPipe == INVALID_HANDLE_VALUE)
    {
        LogErr(_T(GetString.ReturnString(RUX_DEBUG_INVALID_HANDLES, NULL)));
        CloseHandle(hRemOutPipe);
        return FALSE;
    }

    return TRUE;
}

//connect with remote stderr pipe
BOOL RemoteFacade::ConnectRemoteStdErrPipe(DWORD dwRetryCount, DWORD dwRetryTimeOut) 
{
    TCHAR szStdErr[_MAX_PATH] = {0};

    SECURITY_ATTRIBUTES securityAtt = {0};
    SECURITY_DESCRIPTOR securityDescr;
    InitializeSecurityDescriptor(&securityDescr, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&securityDescr, TRUE, NULL, FALSE);

    securityAtt.nLength = sizeof(SECURITY_ATTRIBUTES);
    securityAtt.lpSecurityDescriptor = &securityDescr;;
    securityAtt.bInheritHandle = TRUE;

    hRemStdErrPipe = INVALID_HANDLE_VALUE;

    // StdErr pipe name
    _stprintf_s(szStdErr, _MAX_PATH, _T("%s\\pipe\\%s%s%d"), lpszMachine, RUXSTDERR,
              szThisMachine, GetCurrentProcessId());

    while ( dwRetryCount-- ) {
        // Connects to LogErr pipe
        if (hRemStdErrPipe == INVALID_HANDLE_VALUE) 
        {
            if (WaitNamedPipe(szStdErr, dwRetryTimeOut)) 
            {
                LogInfo(_T(GetString.ReturnString(RUX_DEBUG_WAIT_STDERR, NULL)));
                hRemStdErrPipe = CreateFile(szStdErr, GENERIC_READ, 0,
                                            &securityAtt, OPEN_EXISTING,
                                            FILE_ATTRIBUTE_NORMAL, NULL);
            } 
            else 
            {
                LogInfo(_T(GetString.ReturnString(RUX_DEBUG_WAIT_STDERR_ERROR, NULL)), GetLastError());
                hRemStdErrPipe = CreateFile(szStdErr, GENERIC_READ, 0,
                                            &securityAtt, OPEN_EXISTING,
                                            FILE_ATTRIBUTE_NORMAL, NULL);				
    
            }

            if( beExitThread ){
                return FALSE;
            }

            Sleep(dwRetryTimeOut);
        }

        if (hRemStdErrPipe != INVALID_HANDLE_VALUE) 
        {
            LogInfo(_T(GetString.ReturnString(RUX_DEBUG_VALID_HANDLES, NULL)));
            break;
        }
    }

    if (hRemStdErrPipe == INVALID_HANDLE_VALUE)
    {
        LogErr(_T(GetString.ReturnString(RUX_DEBUG_INVALID_HANDLES, NULL)));
        return FALSE;
    }
    return TRUE;
}

/**
 * 1. Send the message to remote service
 * 2. Connects to remote pipes
 * 3. Waiting for finishing remote process
 */
ERUXError RemoteFacade::RunRemoteCmd(DWORD cmd) 
{
	DWORD dwTemp = 0;
	rUxMessage msg;
	rUxResponse response;

	::ZeroMemory(&msg, sizeof(msg));
	::ZeroMemory(&response, sizeof(response));

	// DEBUG
	LogInfo(_T(GetString.ReturnString(RUX_DEBUG_RUNNING_CMD, NULL)), cmd);

	// Talk to the Remote service and get it to do the work item
	FormatMsg(&msg, cmd); 

	// Send message to service and check for an error
	if (!WriteFile(hCmdPipe, &msg, sizeof(msg), &dwTemp, NULL)) 
	{
		dwTemp = GetLastError();
		LogErr(_T(GetString.ReturnString(RUX_SEND_ERROR, NULL)), dwTemp);
		// Set the boolean so we know that we are not connected
		isConnected = FALSE;
		return ERROR_REMOTE_CONNECT;
	}


	/**
	* DEBUG
	* Use a sleep so we can go unplug the network cable from
	* the remote machine to test
	*/

	// If we are connected to all of the pipes
	if (isConnected) 
	{
		/**
		* Read the information coming back from the service and check
		* for an error
		*/
		//LogInfo(_T(GetString.ReturnString(RUX_DEBUG_IS_CONNECTED, NULL)));
		if (!ReadFile( hCmdPipe, &response, sizeof(response), &dwTemp, NULL )) 
		{
			/**
			* If we are running a package and they specified the -r (reboot)
			* parameter, we WILL get a broken pipe and it is expected
			* Display a nice message and quit
			*/
			if (cmd == RUX_CMD_RUNUX && shouldReboot) 
			{
				LogInfo(_T(GetString.ReturnString(RUX_REMOTE_REBOOTING, NULL)));
				LogInfo(_T(GetString.ReturnString(RUX_VIEW_LOG, NULL)));
				LogInfo(_T(GetString.ReturnString(RUX_RUN_LAUNCHRUX_COMPLETE, NULL)));
				// Set the boolean so we know that we are not connected
				isConnected = FALSE;
				return REMOTE_SUCCESS;
			}


			dwTemp = GetLastError();
			LogErr(_T(GetString.ReturnString(RUX_DEBUG_READ_ERROR, NULL)), dwTemp);

			/**
			* Check to see if we have a broken pipe (could be one of many return codes)
			* If we are updating a network driver on Windows 2000,
			* the network will go down and then come back up in a few
			* seconds.  If that happens, attempt to re-establish
			* communications with the service
			*/
			if ((dwTemp == ERROR_BROKEN_PIPE) || (dwTemp == ERROR_NETNAME_DELETED) || (dwTemp == ERROR_UNEXP_NET_ERR)) 
			{
				// Set the boolean so we know that we are not connected
				isConnected = FALSE;
				hCmdPipe = NULL;
				// Try to connect to the service again
				LogInfo(_T(GetString.ReturnString(RUX_LOST_COMM, NULL)));

				isRemoteAlive = true;
				while ( isRemoteAlive )
				{
					// If we recevied messages from remote service, this valuable will be set to true;
					isRemoteAlive = false;  
					if (ConnectRemoteService(30, 5000)) 
					{
						// Connected to service. Try to reconnect to the pipes
						isConnected = TRUE;
						if (!ReadFile( hCmdPipe, &response, sizeof(response), &dwTemp, NULL )) 
						{
							// If we get an error here, we are done. Display and error and return
							LogErr(_T(GetString.ReturnString(RUX_RESPONSE_ERROR, NULL)));
							LogErr(_T(GetString.ReturnString(RUX_DEBUG_RETURN_CODE, NULL)), GetLastError());
							// Set the boolean so we know that we are not connected
							isConnected = FALSE;
							break;
						}
						else
						{
							LogInfo(_T(GetString.ReturnString(RUX_RECONNECTED, NULL)));
                            Sleep(1000*30);//sj sleep 30 seconds when the network broken and reconnect.
							break;
						}
					}
					// Error reconnecting to the service
					else 
					{
						LogErr(_T(GetString.ReturnString(RUX_RECONNECT_ERROR, NULL)));
					}		   

				}
				if ( isConnected == FALSE)
					return ERROR_REMOTE_CONNECT;

			}
			// Unknown error reading data from the service. We should still be connected
			else 
			{
				LogErr(_T(GetString.ReturnString(RUX_UNKNOWN_READ_ERROR, NULL)), dwTemp);
				return ERROR_REMOTE_CONNECT;
			}
		}
	}
	// Pipes are not connected??? This should never happen
	else 
	{
		LogErr(_T(GetString.ReturnString(RUX_PIPES_NOT_CONNECTED, NULL)));
		return ERROR_REMOTE_CONNECT;
	}

	if (response.dwErrorCode != 0) 
	{
		if (response.dwErrorCode == RUX_PQUEST_RUNNING_ERROR) 
		{
			LogErr(_T("An update has already been scheduled...exiting\n"));
			RunRemoteCmd(RUX_CMD_END);
		}
		else
		{
			_tprintf(_T(GetString.ReturnString(RUX_REMOTE_COMM_START_FAIL, NULL)),
				response.dwErrorCode,
				response.dwErrorCode);
			_tprintf(_T(GetString.ReturnString(RUX_RETURN_CODE, NULL)),
				response.dwReturnCode,
				response.dwReturnCode);
		}

		if (cmd == RUX_CMD_GETMACH) 
		{
			return ERROR_REMOTE_INVENTARY;
		}
		else if ( cmd == RUX_CMD_RUNUX ) 
		{ 
			return ERROR_REMOTE_EXECUTION;
		}
	}

	if (cmd == RUX_CMD_GETMACH) 
	{
		_tcscpy_s(szType, STR_SIZE, response.szMachineType);
		// Just take the first four chars
		szType[4] = '\0';

		g_operatingSystem = (OSTypeEnum)response.operatingSystem;
		g_architectureType = (ArchTypeEnum)response.architectureType;

		// DEBUG
		LogInfo(_T(GetString.ReturnString(RUX_DEBUG_RETURN_FROM_CMD, NULL)), cmd);

		return REMOTE_SUCCESS;
	}
	else if ( cmd == RUX_CMD_RUNUX ) 
	{
		g_remoteReturnCode = response.dwReturnCode;
		return REMOTE_SUCCESS;
	}
	return REMOTE_SUCCESS;
}


// The following functions are used by RemoteInterface and other functions are invisible to it.
void RemoteFacade::LogInfo(const char * format, ...) 
{

	char buffer[1024];
	memset(buffer,0,1024);
	va_list ap;
	va_start(ap, format);
	vsprintf_s(buffer, 1024, format, ap);
	va_end(ap);

	if (logUxEvents) 
	{
		if ( pRemoteLog != NULL )
			pRemoteLog->printOut(buffer);
	}
	return;  

}

void RemoteFacade::LogErr(const char * format, ...) 
{
    char buffer[1024];
    memset(buffer,0,1024);
    va_list ap;
    va_start(ap, format);
    vsprintf_s(buffer, 1024, format, ap);
    va_end(ap);
    
    if (logUxEvents) 
    {
        if ( pRemoteLog != NULL )
            pRemoteLog->printError(buffer);
    }

    return;  
}


ERUXError RemoteFacade::doInitialize(const std::string &rtSvr, const std::string &usrName, const std::string &pwd, const std::string &rtPath) 
{
    if( rtSvr.length() > STR_SIZE - 1 )
    {
        return ERROR_REMOTE_ARGUMENT;
    }
    if( usrName.length() > STR_SIZE - 1 )
    {
        return ERROR_REMOTE_ARGUMENT;
    }
    if( pwd.length() > STR_SIZE - 1 )
    {
        return ERROR_REMOTE_ARGUMENT;
    }
    if( rtPath.length() > STR_SIZE - 1 )
    {
        return ERROR_REMOTE_CREATE_TMP_FILE;
    }

    bool isIPv6= false;
    char oriSvrName[STR_SIZE] = {0};
	char oriIPv6SvrName[STR_SIZE] = {0};
    char* pureSvrName = NULL;
    if (rtSvr.find(string(":")) != string::npos){
        isIPv6= true;
    }

    _tcscpy_s(oriSvrName, STR_SIZE, rtSvr.c_str());
	if(oriSvrName[0]=='['&&strlen(oriSvrName)>=2){
		strncpy(oriIPv6SvrName,oriSvrName+1, strlen(oriSvrName)-2);
		oriIPv6SvrName[strlen(oriSvrName)-2]='\0';
		pureSvrName=oriIPv6SvrName;
	}else{
		pureSvrName= oriSvrName;
	}	
    while( *pureSvrName!='\0' )
    {
        if( *pureSvrName!= '/' && *pureSvrName!= '\\' ) break;
        pureSvrName++;
    }

    //if it's original IPv6 address, process the IP address, convert it to IPv6 literal address.
    if(isIPv6){
        char* pCh = pureSvrName;
        while( *pCh != '\0'){
            if( *pCh == ':' ){
                *pCh = '-';
            }
            pCh++;
        }
    }

    //adding necessary prefix and subfix to make the final szRemoteMachine
    //eg. 10.0.0.1 -> \\10.0.0.1, if IPv6 add ".ipv6-literal.net" as the subfix.
    if(pureSvrName!= NULL)
    {
        _tcscpy_s(szRemoteMachine, STR_SIZE, _T("\\\\"));
        _tcscat_s(szRemoteMachine, STR_SIZE, pureSvrName );
        if(isIPv6){
            _tcscat_s(szRemoteMachine, STR_SIZE, _T(".ipv6-literal.net") );	
        }
    }
    

    _tcscpy_s(szUser, STR_SIZE, usrName.c_str());
    _tcscpy_s(szPassword, STR_SIZE, pwd.c_str());
    if( !rtPath.empty() )
    {
        std::string netPath = MkRemotePathName(rtPath);
        _tcscpy_s( szWorkingDir, STR_SIZE, netPath.c_str() );
    }

    SetCurrentDirectory(lpszLocalDir);

    DWORD dwTemp = STR_SIZE;
    GetComputerName(szThisMachine, &dwTemp);

    //prepare 2 net conncction between local and remote.
    ERUXError ret = RuxConnect( lpszMachine, _T("ADMIN$"));
    if (  ret != REMOTE_SUCCESS ) 
    {
        LogErr(_T(GetString.ReturnString(RUX_CONNECT_ADMIN_FAIL, NULL)), lpszMachine);
        DisplayErrorMessage();
        isSuccess = false;
        return ret;
    }
    ret = RuxConnect(lpszMachine, _T("IPC$"));
    if (  ret != REMOTE_SUCCESS ) 
    {
        LogErr(_T(GetString.ReturnString(RUX_CONNECT_IPC_FAIL, NULL)), lpszMachine);
        DisplayErrorMessage();
        isSuccess = false;
        return ret;
    }

    StopServiceIfRunning(); //stop the service b4 doing the copy to remote system
    ::Sleep(2500); //sj: sleep 2.5 seconds to wait for service stopped.
    DeleteFileOnRemoteMacine(RUXSVCEXE); //delete service on the remote system
    if( !CopySvcExeToRemoteMachine(RUXSVCEXE) ){
        return ERROR_REMOTE_FILE_COPY;
	}//copy the service file to the remote machine.
    
    // Install and start service on remote machine
    if (!InstallAndStartRemoteService()) 
    {
        LogErr(_T(GetString.ReturnString(RUX_START_SERVICE_FAIL, NULL)));
        DisplayErrorMessage();
        isSuccess = false;
        return ERROR_REMOTE_CONNECT;
    }

    // Try to connect again
    if (!ConnectRemoteService(5, 5000)) 
    {
        LogErr(_T(GetString.ReturnString(RUX_CONNECT_SERVICE_FAIL, NULL)));
        DisplayErrorMessage();
        isSuccess = false;
        return ERROR_REMOTE_CONNECT;
    }
    
    ListenOnRemoteStdIoPipes();
    
    //after connection, make the work dir on the remote machine.
    string remoteWorkDir(lpszMachine);
    CheckDir(remoteWorkDir);
    remoteWorkDir += szWorkingDir;
    if( !MakeSureDirectoryPathExists(remoteWorkDir.c_str()) )
    {
      LPVOID lpMsgBuf;
      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,\
               NULL, GetLastError(), 0, (LPTSTR) &lpMsgBuf,	0,	NULL );
      LogErr((LPCTSTR)lpMsgBuf );
      LocalFree(lpMsgBuf);      
    }
    
    return REMOTE_SUCCESS;
}

ERUXError RemoteFacade::doExecute(RemoteOutput* pOutput,const std::string &cmdline, int &returnCode) 
{

    pRemoteOutput = pOutput;
    lpszCommandExe= cmdline.c_str(); 

    LogInfo("Command: %s \n",lpszCommandExe);

    // If run remote program successfully, return REMOTE_SUCCESS and set returnCode parameters. Otherwise return ERROR_REMOTE_EXECUTION.
    ERUXError ret = RunRemoteCmd(RUX_CMD_RUNUX);
    if ( ret != REMOTE_SUCCESS ) 
    {
        pRemoteOutput = NULL;

    }
    else
    {
        returnCode = g_remoteReturnCode;
        pRemoteOutput = NULL;
        ret = REMOTE_SUCCESS;
    }

    Sleep(5000);  //Wait for 5 sec to let finsh receiving remote pipe.
    beExitThread = TRUE;

    static int time = 0;
    while(flagOutputThread == FALSE || flagErrorThread == FALSE){
        Sleep(500);
        time++;
        if( time > 600) break; //wait for 300 sec to break out .
    }

    return ret;
}

void RemoteFacade::doCleanup() 
{
    // Only try to clean up if everything worked
    if (isSuccess) 
    {
        RunRemoteCmd(RUX_CMD_END);
    }

    LogInfo(_T(GetString.ReturnString(RUX_CLEAN_REMOTE, NULL)), lpszMachine);
    
    // Give the service half a second to release any processes before we try to clean up after ourselves
    Sleep(500);

    //remove what we copied.
    std::string::size_type cntCped= m_copiedVec.size();
    for(std::string::size_type i=0; i<cntCped; i++)
    {
        if( !DeleteTree(m_copiedVec[i]) )
        {
            LPVOID lpMsgBuf;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,\
                     NULL,GetLastError(), 0, (LPTSTR) &lpMsgBuf,	0,	NULL );
            LogErr((LPCTSTR)lpMsgBuf );
            LocalFree(lpMsgBuf);
        }
    }
   
    // Give the service half a second to clean itself up and exit before we try to delete it
    Sleep(500);
    StopServiceIfRunning();
    DeleteFileOnRemoteMacine(RUXSVCEXE);

    // Disconnect from remote machine
    RuxDisconnect(lpszMachine, _T("ADMIN$"));
    RuxDisconnect(lpszMachine, _T("IPC$"));

}



ERUXError RemoteFacade::doInventory(std::string &machineType, OSTypeEnum &osType, ArchTypeEnum &archType) 
{
    ERUXError ret = RunRemoteCmd(RUX_CMD_GETMACH);

    if ( ret != REMOTE_SUCCESS ) 
    {
        return ERROR_REMOTE_INVENTARY;
    }
    else 
    {
        machineType.clear();
        machineType.append( szType );
        osType = g_operatingSystem;
        archType = g_architectureType;
        return REMOTE_SUCCESS;
    }
}


ERUXError RemoteFacade::doCopyFileTo(const std::string &fileName, bool recurse)
{
    LogInfo("Copy file: %s to the remote machine. \n", fileName.c_str() );
    //get and check the fileName, see if it's pure Name or pathName, or relavent name.
    string pureName, filePath;
    pureName= GetPureName(fileName, filePath); //fileName= filePath+ pureName;
    CheckDir(filePath);
    //make sure the filePath has '\\' as the last charactor.

    /**
     * need the judge if the lpszMachine has "\\\\" prefix
     * need to check if the lpszWorkingDir is a translated path or a origenal path,(c:\xx or c$\xx)
     */
    //TCHAR destPathName[MAX_PATH];
    //_stprintf_s(destPathName, _MAX_PATH, _T("%s\\%s%s"), lpszMachine, lpszWorkingDir, pureName.c_str()); //set the father dest path 
    string destPathName;
    destPathName= lpszMachine;
    CheckDir(destPathName);
    destPathName+= lpszWorkingDir;
    CheckDir(destPathName);
    destPathName+= pureName;
    CheckSlash(destPathName);

    //judge if it's a file ,dir or not exist.
    DWORD attr = GetFileAttributes(fileName.c_str());
    if(0xFFFFFFFF == attr)
    {
        LogErr("Local file: %s does not exist. \n", fileName.c_str() );
        return ERROR_REMOTE_FILE_NOT_EXIST;
    }
    else if(FILE_ATTRIBUTE_DIRECTORY & attr)
    {
        CheckDir(destPathName);
        string srcFilePath= fileName;
        CheckDir(srcFilePath);
        CheckSlash(srcFilePath);
        //if is dir, get the sub file or dir, by the argument recurse .
        std::vector<std::string> dirVec, fileVec;
        GetSubDir(dirVec, fileVec,fileName,"", recurse);
        //creat the sub dir if need
        if( 0 == ::CreateDirectory(destPathName.c_str(),NULL))
            return ERROR_REMOTE_FILE_COPY;
        if(recurse)
        {
            if( !CreatSubDirs(dirVec, destPathName) )
            {
                LPVOID lpMsgBuf;
                FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,\
                         NULL,GetLastError(), 0, (LPTSTR) &lpMsgBuf,	0,	NULL );
                LogErr((LPCTSTR)lpMsgBuf );
                LocalFree(lpMsgBuf);            
            }
        }
        
        //call the copyfile one by one.
        std::string::size_type subFileCnt= fileVec.size();
        for(std::string::size_type vj= 0; vj< subFileCnt; vj++)
        {
            string destTmp= destPathName+ fileVec[vj];
            string srcTmp= srcFilePath+ fileVec[vj];
            ChangeROAttrIfExist(destTmp);
            if( !CopyFile(srcTmp.c_str(), destTmp.c_str(), NULL))
            {
                LPVOID lpMsgBuf;
                FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,\
                         NULL,GetLastError(), 0, (LPTSTR) &lpMsgBuf,	0,	NULL );
                LogErr((LPCTSTR)lpMsgBuf );
                LocalFree(lpMsgBuf);            
                return ERROR_REMOTE_FILE_COPY;
            }
            m_copiedVec.push_back(destTmp);
        }
    }
    else
    {
        ChangeROAttrIfExist(destPathName.c_str());
        //if it's a file, just call the copyfile.
        string srcTmp= fileName;
        CheckSlash(srcTmp);
        if( !CopyFile(srcTmp.c_str(), destPathName.c_str(), NULL))
        {
            LPVOID lpMsgBuf;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,\
                     NULL,GetLastError(), 0, (LPTSTR) &lpMsgBuf,	0,	NULL );
            LogErr((LPCTSTR)lpMsgBuf );
            LocalFree(lpMsgBuf);        
            return ERROR_REMOTE_FILE_COPY;
        }
        m_copiedVec.push_back(destPathName);
    }
    return REMOTE_SUCCESS;
}

ERUXError RemoteFacade::doCopyFileBack(const std::string &fileName, const std::string &localPath, bool recurse)
{
    //get the pure name of the fileName need to cp back.
    LogInfo("Copy file: %s. from the remote machine.\n ", fileName.c_str() );

    string pureName, filePath;
    pureName= GetPureName(fileName, filePath); //fileName= filePath+ pureName;

    if(filePath.empty() && filePath==string(".\\"))
        filePath= lpszWorkingDir;
    else
    {
        filePath= MkRemotePathName(filePath); //change from c: to c$
    }
    CheckDir(filePath);
    string srcFileName= lpszMachine;
    CheckDir(srcFileName);
    srcFileName+= filePath+ pureName;
    CheckSlash(srcFileName);
    
    //make the loacal path+ name
    string destPathName= localPath;
    CheckDir(destPathName);
    destPathName+= pureName;
    CheckSlash(destPathName);


    //judge if it's a file ,dir or not exist.
    DWORD attr = GetFileAttributes(srcFileName.c_str());
    if(0xFFFFFFFF == attr)
    {
        LogErr("Remote file: %s does not exist. \n", fileName.c_str() );
        return ERROR_REMOTE_FILE_NOT_EXIST;
    }
    else if(FILE_ATTRIBUTE_DIRECTORY & attr)
    {
        CheckDir(destPathName);
        string srcFilePath= srcFileName;
        CheckDir(srcFilePath);
        CheckSlash(srcFilePath);
        //if is dir, get the sub file or dir, by the argument recurse .
        std::vector<std::string> dirVec, fileVec;
        GetSubDir(dirVec, fileVec,srcFileName,"", recurse);
        //creat the sub dir if need
        if( 0 == ::CreateDirectory(destPathName.c_str(),NULL))
            return ERROR_REMOTE_FILE_COPY;
        if(recurse)
        {
            if( !CreatSubDirs(dirVec, destPathName) )
            {
                LPVOID lpMsgBuf;
                FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,\
                         NULL,GetLastError(), 0, (LPTSTR) &lpMsgBuf,	0,	NULL );
                LogErr((LPCTSTR)lpMsgBuf );
                LocalFree(lpMsgBuf);            
            }
        }
        
        //call the copyfile one by one.
        std::string::size_type subFileCnt= fileVec.size();
        for(std::string::size_type vj= 0; vj< subFileCnt; vj++)
        {
            string destTmp= destPathName+ fileVec[vj];
            string srcTmp= srcFilePath+ fileVec[vj];
            ChangeROAttrIfExist(destTmp);
            //::CopyFileEx(srcTmp, destTmp, NULL, NULL, FALSE, NULL);

            if( !CopyFile(srcTmp.c_str(), destTmp.c_str(), NULL))
            {
                LPVOID lpMsgBuf;
                FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,\
                         NULL,GetLastError(), 0, (LPTSTR) &lpMsgBuf,	0,	NULL );
                LogErr((LPCTSTR)lpMsgBuf );
                LocalFree(lpMsgBuf);            
                return ERROR_REMOTE_FILE_COPY;
            }
        }
    }
    else
    {
        ChangeROAttrIfExist(destPathName.c_str());
        //if it's a file, just call the copyfile.
        if( !CopyFile(srcFileName.c_str(), destPathName.c_str(), NULL))
        {
            LPVOID lpMsgBuf;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,\
                     NULL,GetLastError(), 0, (LPTSTR) &lpMsgBuf,	0,	NULL );
            LogErr((LPCTSTR)lpMsgBuf );
            LocalFree(lpMsgBuf);        
            return ERROR_REMOTE_FILE_COPY;
        }
    }

    return REMOTE_SUCCESS;
}

//Get the sub dir and files under "path" recursively if "recurse" is true. The return value is like this dir/subdirA1/subdirB2/a.file
void GetSubDir(std::vector<std::string>& dirVec, std::vector<std::string>& fileVec, string path,string fatherPath, bool recurse)
{
    FInfo ffd;
    HANDLE hFind;
    CheckDir(path);
    if(!fatherPath.empty())
    {
        CheckDir(fatherPath);
    }

    string szDir = path + "*.*";
    
    hFind= FindFirstFile(szDir.c_str(),&ffd);
    if( hFind != INVALID_HANDLE_VALUE )
    {
        do{
            if( (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) && 
                strcmp(ffd.cFileName,".") && strcmp(ffd.cFileName,"..") )
            {
                string subPath= path+ ffd.cFileName;
                string subFatherPath= fatherPath+ffd.cFileName;
                dirVec.push_back(subFatherPath);
                if(recurse)
                {
                    GetSubDir(dirVec, fileVec, subPath, subFatherPath+"\\", recurse);
                }
            }
            else if( strcmp(ffd.cFileName,".") && strcmp(ffd.cFileName,"..") )
            {
                string subName= fatherPath+ ffd.cFileName;
                fileVec.push_back(subName);
            }
        }while(FindNextFile(hFind,&ffd));
    }
    FindClose(hFind);
}

/**
 * Analys the string Path name, split the name with the path. Return the pure filename,eg. c:\dir\subdirA\a.txt ->  a.txt & c:\dir\subdirA\
 */
string GetPureName(string inPathName, string& purePath)
{
    string pureName;
    std::string::size_type pos= inPathName.rfind('\\');
    if(pos == std::string::npos)
    { //can't find 
        purePath= ".\\";
        return inPathName;
    }
    else if(inPathName.size() == pos+1)
    { // the '\\' is the last charactor of the string.
        inPathName.erase(pos);
        pos= inPathName.rfind('\\');
        pureName= inPathName.substr(pos+1);
        purePath= inPathName.substr(0, pos+1);
    }
    else
    {
        pureName= inPathName.substr(pos+1);
        purePath= inPathName.substr(0, pos+1);
    } 
    return pureName;
}

//check the dirname, add slash to it if there is not slash at the end.
void CheckDir(string& dir)
{
    std::string::size_type pos= dir.rfind('\\');
    if( dir.size() == pos+1 )
    {
        return;
    }
    else
    {
        dir+= "\\";
    }
}

//create the the subdir by name like ".\dir\subdirA\subdirB" or "X:\dir\subdir"
bool CreatSubDirs(std::vector<std::string>& dirVec, string destPath)
{
    std::string::size_type subDirCnt= dirVec.size();
    for(std::string::size_type vi=0; vi< subDirCnt; vi++)
    {
        string tmp= destPath;
        tmp+= dirVec[vi];
        if( !::CreateDirectory(tmp.c_str(),NULL) )
        {
#ifdef _DEBUG
           printf("create dir: %s\n", tmp.c_str());
#endif        
           return false;
        }
    }
    return true;
}

//Change the file ReadOnly Attribute if the it exists. 
void ChangeROAttrIfExist(string file)
{
    FInfo fInfo;
    HANDLE hFind = ::FindFirstFile(file.c_str(), &fInfo);
    if(hFind!=INVALID_HANDLE_VALUE)
    {
        ::SetFileAttributes(file.c_str(),FILE_ATTRIBUTE_ARCHIVE);
    }
    FindClose(hFind);	
}

//Change the local made path to remote path, eg. \\10.0.0.1\x:\shared\ -> \\10.0.0.1\x$\shared\ ...
string MkRemotePathName(string name)
{
    string ret= name;

    basic_string <char>::size_type index;

    index = ret.find (":\\");
    
    if ( index != string::npos )
    {
        ret.replace(index, 2,"$\\");
    }
    return ret;
}

//del a directory recursively. 
bool DeleteTree(string sFileName)
{
    string pathName= sFileName;
    CheckDir(pathName);
    pathName+= "*.*";
    FInfo ffd;
    HANDLE hFind;	
    hFind= FindFirstFile(pathName.c_str(),&ffd);
    if( hFind != INVALID_HANDLE_VALUE )
    {
        do{
            if( (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) && 
                strcmp(ffd.cFileName,".") && strcmp(ffd.cFileName,"..") )
            {
                string tmp(sFileName);
                CheckDir(tmp);
                tmp+= ffd.cFileName;
                ::SetFileAttributes(tmp.c_str(), FILE_ATTRIBUTE_ARCHIVE);
                if( !DeleteTree(tmp) )
                {
                    return false;
                }
            }
            else if( strcmp(ffd.cFileName,".") && strcmp(ffd.cFileName,"..") )
            {
                string tmp(sFileName);
                CheckDir(tmp);
                tmp+= ffd.cFileName;
                ::SetFileAttributes(tmp.c_str(), FILE_ATTRIBUTE_ARCHIVE);
                if( !::DeleteFile(tmp.c_str()) )
                {
                    return false;
                }
            }
        }while(FindNextFile(hFind,&ffd));
    }
    FindClose(hFind);

    ::SetFileAttributes(sFileName.c_str(), FILE_ATTRIBUTE_ARCHIVE);
    if(!::RemoveDirectory(sFileName.c_str()))
    {
        if( !::DeleteFile(sFileName.c_str()) )
            return false;
    }
    return true;
}

void CheckSlash(string& path)
{
    basic_string<char>::iterator strIter = path.begin();
    for(;strIter!=path.end(); strIter++)
    {
        if( *strIter == '/' )
        {
            *strIter = '\\';
        }
    }
}

} //namespace ReCBB
