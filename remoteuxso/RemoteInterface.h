#ifndef REMOTEINTERFACE_H_
#define REMOTEINTERFACE_H_


#ifdef FDR_OS_WINDOWS
    #ifdef REMOTE_EXPORTS
    #define EXPORT_CLASS __declspec(dllexport)
    #else
    #define EXPORT_CLASS __declspec(dllimport)
    #endif
#else
    #define EXPORT_CLASS
#endif

#include <stdio.h>
#include <string>
#include <vector>
#include "RemoteData.h"
#include "RemoteOutput.h"

namespace ReCBB
{

class RemoteFacade;

class EXPORT_CLASS RemoteInterface
{
public:
    /**
	 * Destructor.
	 * This function should be defined as virtual to ensure the instance can be deleted within the dll module.
	 * It calls RemoteFacade::doCleanup()
	 */
    virtual ~RemoteInterface(void);
    /**
	 * The only way for consumers to get an instance. If log is specified, it will be used for output messages and errors.
	 */
    static RemoteInterface *getInstance(RemoteOutput* log = NULL);

    static void RemoveInstance();
		
	/**
	 * This function connects remote machine and sets working enviroment on the remote machine.
	 * It calls RemoteFacade::doInitialize(const std::string &rtSvr, const std::string &usrName, 
	 * const std::string &pwd, const std::string &rtPath).
	 */
    ERUXError initialize(const std::string &remoteServer, const std::string &userName, const std::string &userPassword, const std::string &remotePath);
    /**
	 * This function gets system information of the remote machine, such as machine type, OS type, and architeture type.
	 * It calls RemoteFacade::doInventary(std::string &machineType, OSTypeEnum &osType, ArchTypeEnum &archType).
	 */
    ERUXError getSystemInfo(std::string &machineType, OSTypeEnum &osType, ArchTypeEnum &archType);
	/**
	 * This function executes commands on the remote machine.
	 * It calls RemoteFacade::doExecute(RemoteOutput *pOutput, const std::string &cmdline,int &returnCode).
	 */
    ERUXError execute(RemoteOutput* output, const std::string &commandLine, int& returnvalue);

    /**
	 * This function will copy a list of files from local machine to remote machine.
	 * It calls RemoteFacade::doCopyFileTo(const std::string &fileName, bool recurse= true).
	 */
    ERUXError copyFilesToRemote(std::vector<std::string>& filelist, bool recurse= true);
	/**
	 * This function will copy a list of files from remote machine to local machine.
	 * It calls RemoteFacade::doCopyFileBack(const std::string &fileName, const std::string &localPath, bool recurse= true).
	 */
    ERUXError copyFilesToLocal(std::vector<std::string>& filelist, std::string localPath, bool recurse= true);

	/**
	 * This function will copy one file from local machine to remote machine.
	 * It calls RemoteFacade::doCopyFileTo(const std::string &fileName, bool recurse= true).
	 */
    ERUXError copyFilesToRemote(std::string fileName, bool recurse= true);
	/**
	 * This function will copy one file from remote machine to local machine.
	 * It calls RemoteFacade::doCopyFileBack(const std::string &fileName, const std::string &localPath, bool recurse= true).
	 */
    ERUXError copyFilesToLocal(std::string fileName, std::string localPath, bool recurse= true);

private:
	/**
	 * Constructor.
	 */
    RemoteInterface(RemoteOutput* log = NULL);
    static RemoteInterface *instance;
    //pRemoteFacade implements concrete functions, while RemoteInterface only focus on interfaces.
    RemoteFacade* pRemoteFacade;
};

}//namespace ReCBB


#endif



