/* Header file that contains ALL common defines and functions
   that all UpdateXpress tools will use. */
#ifndef UXCOMMON_H_INCLUDED
#define UXCOMMON_H_INCLUDED

#include <tchar.h>
#include "RemoteData.h"

// Return codes.           
#define SUCCESS                        0
#define INVALID_OPTION                 1
#define COULD_NOT_LOAD_RESOURCE_DLL    2
#define UX_RUNNING                     3
#define UNKNOWN_ERROR                  999


#define MAX_STRING              255

#define SERVICENAME             _T("ruxSvc")
#define LONGSERVICENAME         _T("RemoteUX Service")

#define RUXSVCEXE               _T("ruxSvc.exe")
#define RESOURCEDLL             _T("Resource.dll")

#define RUXCOMM                 _T("rux_comm")
#define RUXSTDOUT               _T("rux_stdout")
#define RUXSTDIN                _T("rux_stdin")
#define RUXSTDERR               _T("rux_stderr")

#define STOP_COMMAND            _T("***STOP_THREAD")
#define DLL_LOAD_ERROR			_T("Could not load necessary DLLs.  Exiting.\n")

//
// dwCommandType values
//

// Examine, get the target machine type
#define RUX_CMD_GETMACH         1
// Remotely run UXLite
#define RUX_CMD_RUNUX           3
// End remote service for UpdateXpress
#define RUX_CMD_END             4
// Get the log file
#define RUX_CMD_GETLOG          5
// Get the log file
#define RUX_CMD_CLEARLOG        6
//
// dwErrorCode values
//
#define RUX_PQUEST_RUNNING_ERROR -5000


#define BUFFERSIZE   0x8000
#define STR_SIZE     0x200

class rUxMessage
{
public:
   TCHAR szCommand[1000];
   TCHAR szWorkingDir[_MAX_PATH];
   TCHAR szLocalDir[_MAX_PATH];
   DWORD dwPriority;
   DWORD dwProcessId;
   DWORD dwCommandType;
   DWORD dwOutputBufSize;
   TCHAR szMachine[_MAX_PATH];
   BOOL  bNoWait;
   BOOL  logUxEvents;
};

class rUxResponse
{
public:
   DWORD dwErrorCode;
   DWORD dwReturnCode;
   TCHAR szMachineType[80];
   TCHAR szBiosVer[80];
   TCHAR szDiagsVer[80];
   TCHAR szRsaVer[80];
   TCHAR szIsmpVer[80];
   TCHAR szAsmpVer[80];
   TCHAR szAsmpPciVer[80];
   TCHAR szSrVer[80];
   int operatingSystem;
   int architectureType;
};



#endif                   
   


