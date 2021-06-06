#include "RemoteInterface.h"
#include "remoteux.h"
#include "RemoteData.h"
#include "RemoteOutput.h"

namespace ReCBB{

RemoteInterface *RemoteInterface::instance = NULL;
    
RemoteInterface::RemoteInterface(RemoteOutput* log)
{
    pRemoteFacade = new RemoteFacade(log);
}

RemoteInterface::~RemoteInterface(void)
{
    pRemoteFacade->doCleanup();
}

RemoteInterface * RemoteInterface::getInstance(RemoteOutput* log) 
{
    if( instance == NULL )
        instance = new  RemoteInterface(log); 
    return instance;
}

void RemoteInterface::RemoveInstance()
{
    if (instance != NULL )
        delete instance;	
}

ERUXError RemoteInterface::initialize(const std::string &remoteServer, const std::string &userName, const std::string &userPassword, const std::string &remotePath)
{
    return pRemoteFacade->doInitialize(remoteServer,userName,userPassword,remotePath);
}	

ERUXError RemoteInterface::getSystemInfo(std::string &machineType, OSTypeEnum &osType, ArchTypeEnum &archType)
{
    return pRemoteFacade->doInventory(machineType, osType, archType) ;
}


ERUXError RemoteInterface::execute(RemoteOutput* output, const std::string &commandLine, int& returnvalue)
{
    return pRemoteFacade->doExecute(output,commandLine,returnvalue);
}

ERUXError RemoteInterface::copyFilesToRemote(std::string fileName, bool recurse)
{
    return pRemoteFacade->doCopyFileTo(fileName, recurse);
}

ERUXError RemoteInterface::copyFilesToLocal(std::string fileName, std::string localPath, bool recurse)
{
    return pRemoteFacade->doCopyFileBack(fileName, localPath, recurse);
}

ERUXError RemoteInterface::copyFilesToRemote(std::vector<std::string>& filelist, bool recurse)
{
    ERUXError result;
    bool beNoneFileExists = true;

    //sj: it returns ERROR_REMOTE_FILE_NOT_EXIST, if only none file exists,
    // else if there is one file copied successfully, it will return REMOTE_SUCCESS;
    for(std::string::size_type i=0; i<filelist.size(); i++)
    {
        result = pRemoteFacade->doCopyFileTo(filelist[i], recurse);
        if ( result == REMOTE_SUCCESS ){
            beNoneFileExists = false;
            result = REMOTE_SUCCESS;
        }
        else if ( result == ERROR_REMOTE_FILE_NOT_EXIST ){
            beNoneFileExists = beNoneFileExists && true;
        }
        else{
            return result; //some other reason cause copying file failed, it will return immediately.
        }
    }
    
    return beNoneFileExists ? ERROR_REMOTE_FILE_NOT_EXIST : result;
}

ERUXError RemoteInterface::copyFilesToLocal(std::vector<std::string>& filelist, std::string localPath, bool recurse)
{
    ERUXError result;

    for(std::string::size_type i=0; i<filelist.size(); i++)
    {
        result = pRemoteFacade->doCopyFileBack(filelist[i], localPath, recurse);
        if ( result != REMOTE_SUCCESS )
            return result;
    }
    return REMOTE_SUCCESS;
}


}//namespace ReCBB
