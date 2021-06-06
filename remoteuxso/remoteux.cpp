#include "remoteux.h"
//#include "remote.h"
#include "RemoteOutput.h"


namespace ReCBB{

//add slash to the linux path whether there is no slash at the end.
char* addSlashToPath(char* path)
{
    if(path == NULL) return path;
    char* p = path;
    int len = strlen(path);
    p+=len-1;
    if(*p == '/'){
        return path;
    }
    else{
        strcat(path, "/\0");
    }
    return path;
}

//Get a single pure name of a file, strip the slashs and the dir path, eg: /usr/bin/a.file -> a.file
string getPureName(string strIn)
{
    string strRet;
    unsigned int pos = strIn.rfind('/');
    if(pos ==  string::npos){ //can't find 
        return strIn;
    }
    else if(strIn.size() == pos+1){ // the '/' is the last charactor of the string.
        strIn.erase(pos);
        pos = strIn.rfind('/');
        strRet = strIn.substr(pos+1);
    }
    else{
        strRet = strIn.substr(pos+1);
    } 
    return strRet;
}

//Get local file info of (chFileName) , see whether it is a file or dir or even not exist.
int isFile(const char* chFileName)
{
    int ret;
    struct stat info;
    ret = stat(chFileName, &info);
    if(ret< 0){
        return -1; //file not exist;
    }
    else{
        if(S_ISREG(info.st_mode)){
            return 1; //it is a regular file.
        }
        else if(S_ISDIR(info.st_mode)){
            return 0; //it is a dir
        }
        else{
            return 2; //it's not a dir or a file, but some else. eg. a link ...
        }
    }
}

//Get remote file info of (chFileName), see whether it is a file or dir or even not exist.
int RemoteFacade::isRemoteDir(const char* filename)
{
    int ret = -1;
    char cmdLine[MAXLINE] = {0};
    char buff[MAXLINE] = {0};
    strcpy(cmdLine, "file ");
    strcat(cmdLine, filename);
    FILE* sshfp = NULL;
    if(m_isConnectFailed){
         return ret;
    }
    else{
        sshfp = sshopen(m_cmdopts, cmdLine);
    }

    if(sshfp == NULL){
        uxerror("sshopen error while checking the remote file %s.", filename);
        return ret;
    }
    
    char expectStr[MAXLINE] = {0};
    strcpy(expectStr, filename);
    strcat(expectStr, ": directory");
    while(fgets(buff, MAXLINE, sshfp) != NULL){
        if (strstr(buff, "__End__") != NULL) {
            break;
        }
        else if(strstr(buff, expectStr)){ //is a dir
            ret = 1;
            break;
        }
        else if(strstr(buff, "cannot open")!=NULL ){
            ret = -1;
            break;
        }
        else{ //it's a file or something else.
            ret=2;
        }
    }
    fclose(sshfp);
    
    return ret;
}

//##################################################################
RemoteFacade::RemoteFacade(RemoteOutput* log): m_pRemoteLog(log),m_isConnectFailed(false),m_eFailedReason(EConnectSuccess)
{
}

RemoteFacade::~RemoteFacade()
{
}

void RemoteFacade::doCleanup()
{
    //the filename in the vector m_listCopied is the src path file , not remote path name.
    //it's a little different with the Facade doCleanup() under windows. The windows will storage the 
    // file name as the remote pathName, such as \\10.0.0.5\c$\shared\a.file
    int fileCnt = m_listCopied.size();
    char remoteWorkDir[MAXLINE] = {0};
    strcpy(remoteWorkDir,m_cmdopts.remdir);
    addSlashToPath(remoteWorkDir);
    string destDir(remoteWorkDir);
    string remoteFileName;
    for(int i=0; i<fileCnt; i++){
        string tmp = getPureName(m_listCopied[i]);
        remoteFileName = destDir+tmp;
        deltreeOfRemote(remoteFileName); //both file or dir wil be removed by this function
    }
    
    string sysInfoScript = destDir+ string(BIOSINFO);
    deltreeOfRemote(sysInfoScript); 
      
    char echoBuf[] = "echo \"finish del the file in the remote machine.\"";
    FILE* sshfp = NULL;
    if(m_isConnectFailed){
        return;
    }
    else{
        sshfp = sshopen(m_cmdopts, echoBuf);
    }
    if(sshfp == NULL){
        return ;
    }
    fclose(sshfp);
}

ERUXError RemoteFacade::doInitialize(const string &rtSvr, const string &usrName, const string &pwd, const string &rtPath)
{
    //check the argument input is all right.
    if( rtSvr.length() > STR_SIZE - 1 ){
        return ERROR_REMOTE_ARGUMENT;
    }
    if( usrName.length() > STR_SIZE - 1 ){
        return ERROR_REMOTE_ARGUMENT;
    }
    if( pwd.length() > STR_SIZE - 1 ){
        return ERROR_REMOTE_ARGUMENT;
    }
    if( rtPath.length() > STR_SIZE - 1 ){
        return ERROR_REMOTE_CREATE_TMP_FILE;
    }
    //set the default
    m_cmdopts.updatetype = UXENUM;
    m_cmdopts.pkgname[0]='\0';
    m_cmdopts.nodos=0;
    strcpy(m_cmdopts.username, "root");
    strcpy(m_cmdopts.tmpdir,"/tmp");
    strcpy(m_cmdopts.remdir,"/tmp");
    strcpy(m_cmdopts.localimg,"./");
    strcpy(m_cmdopts.passwd,""); //in linux, can the password of root to be empty
    //set from input	
    strcpy(m_cmdopts.hostname, rtSvr.c_str()); //in the windows there is suffix of \\9.*.., which doesn't need in linux.
    strcpy(m_cmdopts.username, usrName.c_str());
    strcpy(m_cmdopts.passwd, pwd.c_str());
    if(!rtPath.empty()){
        strcpy(m_cmdopts.remdir, rtPath.c_str());
    }

    //mkdir in the remote machine
    int ret = mkDirToRemote(rtPath);
    if(m_eFailedReason == ENoPermission){
        return ERROR_REMOTE_AUTHENTICATE;
    }
    else if( ret == 1){
        return ERROR_REMOTE_CONNECT;
    }
    else if(ret == ERROR_REMOTE_CREATE_TMP_FILE){
        return ERROR_REMOTE_CREATE_TMP_FILE;
    }
        
#ifdef DEBUG
    m_cmdopts.verboseflag = 3;
#else
    m_cmdopts.verboseflag= 0;	
#endif
    return 	REMOTE_SUCCESS;
}

ERUXError RemoteFacade::doExecute(RemoteOutput *pOutput, const string cmdline,int &returnCode)
{
    char ch_cmdLine[MAXLINE] = {0};
    char buff[MAXLINE] = {0};
    char* pRet;
    strcpy(ch_cmdLine, cmdline.c_str());

    FILE* sshfp = NULL;
    if(m_isConnectFailed){
        return ERROR_REMOTE_CONNECT;
    }
    else{
        sshfp = sshopen(m_cmdopts, ch_cmdLine);
    }
    
    if(sshfp == NULL){ 
        return ERROR_REMOTE_CONNECT;
    }
    
    int sshfd= fileno(sshfp);
    fd_set rset;
    int selectRet = -1;
    struct timeval tv;
    tv.tv_sec= EXETIMEOUT;
    tv.tv_usec= 0;
    char oneCh[2] = {0};
    bool isFinished= false;
    do{
        FD_ZERO(&rset);
        FD_SET(sshfd, &rset);
        switch( select(sshfd+1, &rset, NULL, NULL, &tv) ){
        case -1:
            uxerror("Select error!\n");
            fclose(sshfp);
            return ERROR_REMOTE_CONNECT;	    	
            break;
        case 0:
            uxerror("Connect with remote machine time out!\n");
            fclose(sshfp);
            return ERROR_REMOTE_CONNECT;
            break;
        default:
            if(FD_ISSET(sshfd,&rset)){
                if( (fgets(oneCh, 2, sshfp)) != NULL){
                    int len = 0;
                    len = strlen(buff);
                    strncat(buff, oneCh, MAXLINE-len);
                    if(!isFinished){
                        if(oneCh[0] == '\n' || oneCh[0] == '.'){
                            if(pOutput!=NULL){
                                pOutput->printOut(buff);
                            }
                            memset(buff, 0, MAXLINE);
                        }
                        else if( (pRet = strstr(buff, "ReturnCode=")) != NULL ){
                            *pRet= '\0';
                            if(pOutput!=NULL){
                                pOutput->printOut(buff);
                            }
                            memset(buff, 0, MAXLINE);
                            isFinished= true;
                        }
                    }
                    else{
                        if(strstr(buff, "__End__") != NULL){
                            returnCode= atoi(buff);
                            fclose(sshfp);
                            return REMOTE_SUCCESS;
                        }
                    }
                }
           }
           else
               return ERROR_REMOTE_EXECUTION; 
        }
    }while(1);
}


ERUXError RemoteFacade::doInventory(string &machineType, OSTypeEnum &osType, ArchTypeEnum &archType)
{

    int i_archType, i_osType;
    i_archType = getArchType(m_cmdopts);
    i_osType = getOSType(m_cmdopts);
    if(i_archType < 0){
        return ERROR_REMOTE_INVENTARY;
    }
    if(i_osType < 0){
        return ERROR_REMOTE_INVENTARY;
    }
    
    osType = OSTypeEnum(i_osType) ;
    archType = ArchTypeEnum(i_archType);
    
    int machine = -1;
    char chrMachineType[16] = {0};
    machine = getBiosInfo(m_cmdopts, chrMachineType);

    if(machine > 0){
        machineType = std::string(chrMachineType);
    }
    else{
        machineType = std::string("unknown");
        return  ERROR_REMOTE_INVENTARY;        
    }
    
    return REMOTE_SUCCESS;	
}


//copy file to remote machine
ERUXError RemoteFacade::doCopyFileTo(string fileName, bool recurse)
{
    //first see if the file exist.
    char srcFile[SBUFF] = {0};
    char destDir[SBUFF] = {0};
    strcpy(destDir, m_cmdopts.remdir);	
    strcpy(srcFile, fileName.c_str());
        
    int fileStat = isFile(fileName.c_str());
    if( fileStat<0 ){
        uxerror("Can not copy file (%s), maybe not exist!\n", srcFile);
        return ERROR_REMOTE_FILE_NOT_EXIST; //file not exist 
    }
    else if( fileStat>1 ){
        uxerror("Can not copy file (%s), maybe it's a link or some else!\n", srcFile);
        return ERROR_REMOTE_FILE_COPY; //it's a link or some else.
    }
    
    if(recurse){
        scpfiles(m_cmdopts, srcFile, destDir);
    }
    else{
        if(fileStat == 1){ //it's a regular file. 
            scpfiles(m_cmdopts, srcFile, destDir);
        }
        else if(fileStat == 0) { //it's a dir, and we won't cp all the subdir recursively. 
            string pureName = getPureName(fileName.c_str());
            addSlashToPath(destDir);
            strcat(destDir, pureName.c_str());
            mkDirToRemote(destDir); //mkdir to the remote machine, so that file can be cpied to this dir. 
            
            DIR* dir = opendir(fileName.c_str());
            struct dirent* entry;
            while( (entry = readdir(dir))!= NULL ){
                strcpy(srcFile,fileName.c_str());
                addSlashToPath(destDir);
                strcat(srcFile, entry->d_name);
                if(isFile(srcFile) == 1)
                    scpfiles(m_cmdopts, srcFile, destDir); //we cp the file to the subdir of the remote work dir.
            }
        }
    }
    m_listCopied.push_back(fileName); //add the file copied to the m_listCopied.
    //if it is a dir, we just recode it's name. when clean up, we will use rm -rf the dir.	
    return REMOTE_SUCCESS;
}

//copy file back from remote machine
ERUXError RemoteFacade::doCopyFileBack(string fileName, string localPath, bool recurse)
{
    char remoteSrcFileName[MAXLINE] = {0};
    char localDir[MAXLINE] = {0};
    strcpy(localDir ,localPath.c_str());
    strcpy(remoteSrcFileName,fileName.c_str());
    int isDir = isRemoteDir(remoteSrcFileName);
    
    if(isDir< 0){
        uxerror("The file (%s) not exists in the remote machine.",fileName.c_str());
        return ERROR_REMOTE_FILE_NOT_EXIST;
    }
    else if(isDir == 1){
        addSlashToPath(localDir);
        char cmdMkdir[MAXLINE]= {"mkdir -p "};
        strcat(cmdMkdir, localDir);
        system(cmdMkdir);
        scpfilefromRem(m_cmdopts, remoteSrcFileName, localDir, recurse);
    }
    else{//the filename exists in the remote, maybe it's a file.
        scpfilefromRem(m_cmdopts, remoteSrcFileName, localDir, false);
    }
    return REMOTE_SUCCESS;
}

int RemoteFacade::mkDirToRemote(string dirName)
{
    char cmdLine[MAXLINE] = {0};
    sprintf(cmdLine, "mkdir -p %s", dirName.c_str());
    FILE* sshfp = NULL;
    if(m_isConnectFailed){
       return -1;
    }
    else{
        sshfp = sshopen(m_cmdopts, cmdLine);
    }
    if(sshfp == NULL){
        return 1;
    }

    char buff[MAXLINE] = {0};
    while(fgets(buff, MAXLINE, sshfp) != NULL){
        if( (strstr(buff, "exists but is not a directory")) != NULL ){
            uxerror("Can't create dir named (%s) in remote machine, the file exists but is not a directory.\n",dirName.c_str());
            return ERROR_REMOTE_CREATE_TMP_FILE;
        }
    }
    fclose(sshfp);
    return 0;	
}

//del a directory recursively in the remote machine
int RemoteFacade::deltreeOfRemote(string pathName)
{
    char cmdLine[MAXLINE] = {0};
    sprintf(cmdLine, "rm -rf %s", pathName.c_str());
    FILE* sshfp= NULL;
    if(m_isConnectFailed){
       return -1;
    }
    else{
        sshfp = sshopen(m_cmdopts, cmdLine);
    }

    if(sshfp == NULL){
        return -1;
    }
        
    fclose(sshfp);
    return 0;
}

//execute a cmd by ssh method.
FILE* RemoteFacade::sshopen( UXcmd cmdopt,  char *rcommand ) 
{
    int fd, count;
    pid_t pid;
    char buffer[MAXLINE] = {0};
    char target[MAXLINE]; 
    char option[3];
    char tmpchar[2];
    char remotecmd[MAXLINE];
    FILE *sshcmd;
    
    // Build the command options    

    // Use the -t switch to force a pseudo-tty terminal. This 
    // is necessary for the Broadcom package to work correctly.
    strcpy(option, "-t");

    strcpy(target, cmdopt.username);
    strcat(target, "@");
    strcat(target, cmdopt.hostname);
    
    strcpy(remotecmd, "echo __Start__; ");
    strcat(remotecmd, rcommand);
    strcat(remotecmd, "; echo ReturnCode=$?");
    strcat(remotecmd, "; echo __End__");
    
#ifdef DEBUG
    uxdebug("target:\"%s\" ; cmd:\"%s\" \n", target, rcommand);
#endif    
    
    pid = forkpty(&fd, 0, 0, 0);

    if (pid == 0) {
        execlp("ssh", "ssh", option, target, remotecmd, (void *)NULL);
        exit(1);
    } else if (pid == -1) {
        return (FILE *)NULL;
    } else {
        sshcmd=fdopen(fd, "a+");
        strcpy(buffer,"\0");
        // Read 1 char at a time and build up a string
        count=0;
        if (sshcmd == 0) {
            uxerror("Could not open ssh connection.");
            return (FILE *)NULL;
        }
        while((fgets(tmpchar, 2, sshcmd) != NULL ) && (count++ < MAXLINE)){
            strcat(buffer, tmpchar);
            if (strstr(buffer,"ssword:") != NULL) {
                fprintf(sshcmd, cmdopt.passwd);
                fprintf(sshcmd, "\n");
                fflush(sshcmd);
                // reset search string
                strcpy(buffer,"\0");
            }
            if (strstr(buffer,"connecting (yes/no)?") != NULL) {
                fprintf (sshcmd,"yes\n");
                fflush (sshcmd);
                // reset search string
                strcpy(buffer,"\0");
            }
            if (strstr(buffer,"Enter passphrase") != NULL) {
                fprintf(sshcmd, cmdopt.passwd);
                fprintf(sshcmd, "\n");
                fflush(sshcmd);
                // reset search string
                strcpy(buffer,"\0");
            }
            if (strstr(buffer,"No route to host") != NULL) {
                uxerror(buffer);
                fclose(sshcmd);
                m_isConnectFailed = true;
                m_eFailedReason = ENoRoutToHost;
                return (FILE *)NULL;
            }
            if (strstr(buffer,"Permission denied") != NULL) {
                uxerror(buffer);
                fclose(sshcmd);
                m_isConnectFailed = true;
                m_eFailedReason = ENoPermission;
                return (FILE *)NULL;
            }
            if (strstr(buffer,"Connection refused") != NULL) {
                uxerror(buffer);
                fclose(sshcmd);
                m_isConnectFailed = true;
                m_eFailedReason = EConnectRefused;
                return (FILE *)NULL;
            }
            if (strstr(buffer,"Host key verification failed") != NULL) {
                uxerror(buffer);
                fclose(sshcmd);
                m_isConnectFailed = true;
                m_eFailedReason = EHostKeyNoVerified;
                return (FILE *)NULL;
            }
            if (strstr(buffer,"Name or service not known") != NULL) {
                uxerror(buffer);
                fclose(sshcmd);
                m_isConnectFailed = true;
                m_eFailedReason = ENameSvcUnknown;
                return (FILE *)NULL;
            }
            if (strstr(buffer,"__Start__") != NULL) {
                return (FILE *)sshcmd;
            }
        }
    }
    return (FILE *)NULL;
}

//cp file to remote machine by scp command
bool RemoteFacade::scpfiles( UXcmd cmdopt, char *fname, char *destdir) 
{
    char cmdline[SBUFF];
    char buffer[MAXLINE];
    int fd;
    pid_t pid;
    char tmpchar[2];
    FILE *sshcmd;

    strcpy(cmdline,cmdopt.username);
    strcat(cmdline,"@");
    strcat(cmdline,cmdopt.hostname);
    strcat(cmdline,":");
    if( destdir == NULL ) {
        destdir=cmdopt.remdir;
    }
    strcat(cmdline,destdir);
#ifdef DEBUG
    printf("scp -rpC %s %s\n",fname,cmdline);
#endif
    pid = forkpty(&fd, 0, 0, 0);

    if (pid == 0) {
        execlp("scp", "scp", "-rpC", fname, cmdline, (void *)NULL);
        exit(1);
    } else if (pid == -1) {
        uxerror("Could not spawn scp process\n");
        return false;
    } else { 
// Read 1 char at a time and build up a string
// This will wait forever until "Last Login" is found... Adjust as necessary
//        count=0;
        sshcmd=fdopen(fd, "a+");
        strcpy(buffer,"\0");
        if (sshcmd == 0) {
            uxerror("Could not open ssh connection.");
            return false;
        }
        while((fgets(tmpchar, 2, sshcmd) != NULL ) ){
            if(strlen(buffer)>MAXLINE) {
                printf("endof buffer");
            }
#ifdef  DEBUG            
            printf(tmpchar);
#endif            
            if (strlen(buffer)<MAXLINE) {
                strcat(buffer, tmpchar);
            }
            if (strstr(buffer,"ssword:") != NULL) {
                fprintf(sshcmd, cmdopt.passwd);
                fprintf(sshcmd, "\n");
                fflush(sshcmd);
                // reset search string
                strcpy(buffer,"\0");
            }
            if (strstr(buffer,"connecting (yes/no)?") != NULL) {
                fprintf (sshcmd,"yes\n");
                fflush (sshcmd);
                // reset search string
                strcpy(buffer,"\0");
            }
            if (strstr(buffer,"Enter passphrase") != NULL) {
                fprintf(sshcmd, cmdopt.passwd);
                fprintf(sshcmd, "\n");
                fflush(sshcmd);
                // reset search string
                strcpy(buffer,"\0");
            }
            if (strstr(buffer,"No route to host") != NULL) {
                fclose(sshcmd);
                uxerror("\nscp failed, %s\n",buffer);
                m_isConnectFailed = true;
                m_eFailedReason = ENoRoutToHost;
                return false;
            }
            if (strstr(buffer,"Permission denied") != NULL) {
                fclose(sshcmd);
                uxerror("\nscp failed, %s\n",buffer);
                m_isConnectFailed = true;
                m_eFailedReason = ENoPermission;
                return false;
            }
            if (strstr(buffer,"Connection refused") != NULL) {
                fclose(sshcmd);
                uxerror("\nscp failed, %s\n",buffer);
                m_isConnectFailed = true;
                m_eFailedReason = EConnectRefused;
                return false;
            }
            if (strstr(buffer,"Host key verification failed") != NULL) {
                fclose(sshcmd);
                uxerror("\nscp failed, %s\n",buffer);
                m_isConnectFailed = true;
                m_eFailedReason = EHostKeyNoVerified;
                return false;
            }
            if (strstr(buffer,"Name or service not known") != NULL) {
                fclose(sshcmd);
                uxerror("\nscp failed, %s\n",buffer);
                m_isConnectFailed = true;
                m_eFailedReason = ENameSvcUnknown;
                return false;
            }
        }
    }
    return true;
}

//copy file from Remote machine by scp command
bool RemoteFacade::scpfilefromRem( UXcmd cmdopt, char *fname, char *localdir, bool recurse) 
{
    char remUsrHostFile[SBUFF];
    char buffer[MAXLINE];
    int fd;
    pid_t pid;
    char tmpchar[2];
    FILE *sshcmd;

    strcpy(remUsrHostFile,cmdopt.username);
    strcat(remUsrHostFile,"@");
    strcat(remUsrHostFile,cmdopt.hostname);
    strcat(remUsrHostFile,":");
    strcat(remUsrHostFile,fname);

    
    if (cmdopt.verboseflag > 1) {
        printf("scp command: scp -rpC %s %s\n",remUsrHostFile, localdir);
    }

    pid = forkpty(&fd, 0, 0, 0);

    if (pid == 0) {
        if(recurse){
            execlp("scp", "scp", "-rpC",  remUsrHostFile, localdir, (void *)NULL);
        }
        else{
            execlp("scp", "scp", "-pC",  remUsrHostFile, localdir, (void *)NULL);
        }
        exit(1);
    } else if (pid == -1) {
        printf("Could not spawn scp process\n");
        return false;
    } else { 
        sshcmd=fdopen(fd, "a+");
        strcpy(buffer,"\0");
// Read 1 char at a time and build up a string
// This will wait forever until "Last Login" is found... Adjust as necessary
//        count=0;
        if (sshcmd == 0) {
            uxerror("Could not open ssh connection.");
            return false; 
        }
        while((fgets(tmpchar, 2, sshcmd) != NULL ) ){
            if(strlen(buffer)>MAXLINE) {
                printf("endof buffer");
            }
#ifdef  DEBUG            
            printf(tmpchar);
#endif            
            if (strlen(buffer)<MAXLINE) {
                strcat(buffer, tmpchar);
            }
            if (strstr(buffer,"ssword:") != NULL) {
                fprintf(sshcmd, cmdopt.passwd);
                fprintf(sshcmd, "\n");
                fflush(sshcmd);
                // reset search string
                strcpy(buffer,"\0");
            }
            if (strstr(buffer,"connecting (yes/no)?") != NULL) {
                fprintf (sshcmd,"yes\n");
                fflush (sshcmd);
                // reset search string
                strcpy(buffer,"\0");
            }
            if (strstr(buffer,"Enter passphrase") != NULL) {
                fprintf(sshcmd, cmdopt.passwd);
                fprintf(sshcmd, "\n");
                fflush(sshcmd);
                // reset search string
                strcpy(buffer,"\0");
            }
            if (strstr(buffer,"No route to host") != NULL) {
                fclose(sshcmd);
                uxerror("\nscp failed, %s\n",buffer);
                m_isConnectFailed = true;
                m_eFailedReason = ENoRoutToHost;
                return false;
            }
            if (strstr(buffer,"Permission denied") != NULL) {
                fclose(sshcmd);
                uxerror("\nscp failed: %s\n",buffer);
                m_isConnectFailed = true;
                m_eFailedReason = ENoPermission;
                return false;
            }
            if (strstr(buffer,"Connection refused") != NULL) {
                fclose(sshcmd);
                printf("\nscp failed: %s\n",buffer);
                m_isConnectFailed = true;
                m_eFailedReason = EConnectRefused;
                return false;
            }
            if (strstr(buffer,"Host key verification failed") != NULL) {
                fclose(sshcmd);
                printf("\nscp failed: %s\n",buffer);
                m_isConnectFailed = true;
                m_eFailedReason = EHostKeyNoVerified;
                return false;
            }
            if (strstr(buffer,"Name or service not known") != NULL) {
                fclose(sshcmd);
                printf("\nscp failed: %s\n",buffer);
                m_isConnectFailed = true;
                m_eFailedReason = ENameSvcUnknown;
                return false;
            }
        }
    }
}

int RemoteFacade::getBiosInfo(UXcmd cmdopts, char* sType)
{
    FILE *sshfp = NULL;
    char tmpstr[MAXLINE];
    char cmdstr[SBUFF];
    char dmibuff[SBUFF];
    char buff[MAXLINE];
    char *tmpline;

    strcpy(dmibuff, "./bin/");
    strcat(dmibuff, BIOSINFO);
    scpfiles(cmdopts, dmibuff, NULL);

    strcpy(tmpstr, cmdopts.remdir);
    strcat(tmpstr, "/");
    strcat(tmpstr, BIOSINFO);
    
    //Make sure it's executable
    strcpy(cmdstr,"chmod 755 ");
    strcat(cmdstr,tmpstr);
    strcat(cmdstr,";");
    strcat(cmdstr,tmpstr);
    
    char chMachineType[16] = {0};
    // Exec BIOSINFO on remote system
    if(m_isConnectFailed){
         return -1;
    }
    else{
        sshfp = sshopen(cmdopts, cmdstr);
    }
    if(sshfp == NULL){
        return -1;
    }
    while (fgets(buff, MAXLINE, sshfp) != NULL) {
#ifdef DEBUG
        printf(buff);
#endif        
        if (strstr(buff,"__End__") != NULL) {
            break;
        }
        else if(strstr(buff, "SystemProduct=" ) != NULL ){
            char *str1 = strstr(buff, "-[");
            char *str2 = strstr(buff, "]-");
            if(str1!=NULL && str2!=NULL && str1<str2){
                strncpy(chMachineType, (str1+2), 16);
            }
        }
    }
    fclose(sshfp);

    if(strlen(chMachineType)!=0){
        strncpy(sType, chMachineType, 4);
        return 1;
    }
    else{
        return -1;
    }

    return -1;
}

int RemoteFacade::getArchType(UXcmd cmdopts)
{
    int arch = -1;
    FILE *sshfp = NULL;
    char cmdstr[SBUFF];
    char buff[MAXLINE];
    //make the cmd line to be execute
    memset(cmdstr,0,sizeof(cmdstr));
    strcpy(cmdstr, "uname -m"); 
    
    //execute the cmd
    if(m_isConnectFailed){
         return -1;
    }
    else{
        sshfp = sshopen(cmdopts, cmdstr);
    }
    //show the result of execution
    if (sshfp == NULL) {
        return -1;
    }
    while(fgets(buff, MAXLINE, sshfp) != NULL){

        if (strstr(buff, "__End__") != NULL) {
            break;
        }
        else if(strstr(buff, "i386")!=NULL || strstr(buff, "i486")!=NULL 
            || strstr(buff, "i586")!=NULL || strstr(buff, "i686")!=NULL){
            arch = e32bit;
            break;
        }
        else if(strstr(buff, "x86_64")!=NULL || strstr(buff, "X86_64")!=NULL){
            arch = e64bit;
            break;
        }
        else{
            arch = eNoArch;
        }
#ifdef DEBUG		
        printf(buff);
#endif		
    }
    fclose(sshfp);
    return arch;
}

int RemoteFacade::getOSType(UXcmd cmdopts)
{
    bool isVmware = false;
    bool isRedhat = false;
    bool isSUSE = false;
    int version = 0;
    int ret = -1;
    
    FILE *sshfp = NULL;
    char cmdstr[SBUFF];
    char buff[MAXLINE];

   //make the cmd line to be execute
    memset(cmdstr,0,sizeof(cmdstr));
    memset(buff,0,sizeof(buff));
    strcpy(cmdstr, "vmware -v");

    //execute the cmd
    if(m_isConnectFailed){
         return -1;
    }
    else{
        sshfp = sshopen(cmdopts, cmdstr);
    }


    //show the result of execution
    if (sshfp == NULL) {
        return -1;
    }
    while(fgets(buff, MAXLINE, sshfp) != NULL){
#ifdef DEBUG
        printf("%s", buff);
#endif
        if(strstr(buff, "command not found")!=NULL){
            isVmware = false;
            break;
        }
        else{
            if( strstr(buff, "VMware ESX")!=NULL ){
                isVmware = true;
            }
            if( strstr(buff, " 2.")!=NULL && isVmware==true ){
                ret = eVMWareESX2;
            }
            else if( strstr(buff, " 3.0") != NULL && isVmware==true ){
                ret = eVMWareESX30;
            }
            else if( strstr(buff, " 3.5") != NULL && isVmware==true ){
                ret = eVMWareESX35; 
            }
        }

    }
    fclose(sshfp);

    if(ret > 0){
        return ret;
    }

    //make the cmd line to be execute
    memset(cmdstr,0,sizeof(cmdstr));
    memset(buff,0,sizeof(buff));
    strcpy(cmdstr, "cat /etc/redhat-release");

    //execute the cmd
    if(m_isConnectFailed){
         return -1;
    }
    else{
        sshfp = sshopen(cmdopts, cmdstr);
    }
    

    //show the result of execution
    if (sshfp == NULL) {
        return -1;
    }
    while(fgets(buff, MAXLINE, sshfp) != NULL){
#ifdef DEBUG
        printf(buff);
#endif			
        if(strstr(buff, "No such file or directory")!=NULL){
            isRedhat = false;
            break;
        }
        else{
            if(strstr(buff, "Red") != NULL){
                isRedhat = true;
            }
            char *p = buff;
            for(;*p!='\0';p++){
                if(*p>='0' && *p<='9'){
                    version = atoi(p);
                    break;
                }
            }
        }
        if(version == 3 && isRedhat){
            ret = eRHEL3;
            break;
        }
        else if(version == 4 && isRedhat){
            ret = eRHEL4;
            break;
        }
        else if(version == 5 && isRedhat){
            ret = eRHEL5;
            break;
        }
        else if(version == 6 && isRedhat){
            ret = eRHEL6;
            break;
        }
    }
    fclose(sshfp);
    
    if(ret > 0){
        return ret;
    }
    
    //make the cmd line to be execute
    memset(cmdstr,0,sizeof(cmdstr));
    memset(buff,0,sizeof(buff));
    strcpy(cmdstr, "cat /etc/SuSE-release");

    //execute the cmd
    if(m_isConnectFailed){
         return -1;
    }
    else{
        sshfp = sshopen(cmdopts, cmdstr);
    }
    
    //show the result of execution
    if (sshfp == NULL) {
        return -1;
    }
    while(fgets(buff, MAXLINE, sshfp) != NULL){
        if(strstr(buff, "No such file or directory")!=NULL){
            isSUSE = false;
            break;
        }
        else{
            if(strstr(buff, "SUSE") != NULL){
                isSUSE = true;
            }
            char *p = buff;
            for(;*p!='\0';p++){
                if(*p>='0' && *p<='9'){
                    version = atoi(p);
                    break;
                }
            }
        }
        if(version == 8 && isSUSE){
            ret = eSLES8;
            break;
        }
        else if(version == 9 && isSUSE){
            ret = eSLES9;
            break;
        }
        else if(version == 10 && isSUSE){
            ret = eSLES10;
            break;
        }
        else if(version == 11 && isSUSE){
            ret = eSLES11;
            break;
        }
        else if(isSUSE){
            ret = eSLES9;
            break;
        }
        else{
            ret = eNone;
        }
#ifdef DEBUG
        printf("contain of /etc/SuSE-release : %s\n", buff);
#endif		
    }
    fclose(sshfp);
    return ret;	
}

void RemoteFacade::uxerror (const char* format, ...) 
{
    char buffer[1024];
    memset(buffer,0,1024);
    va_list ap;
    va_start(ap, format);
    vsprintf(buffer, format, ap);
    va_end(ap);
    if(m_pRemoteLog!= NULL)
        m_pRemoteLog->printError(buffer);
}

void RemoteFacade::uxdebug (const char* format, ...)
{
    char buffer[1024];
    memset(buffer,0,1024);
    va_list ap;
    va_start(ap, format);
    vsprintf(buffer, format, ap);
    va_end(ap);
    if(m_pRemoteLog!= NULL)
        m_pRemoteLog->printOut(buffer);
}

DNode* RemoteFacade::split ( char *src, char *sep )
{
    DNode *top, *curr, *newItem;
    char tmpstr[MAXLINE];
    char *tok, *tmpline;

    strcpy(tmpstr, src);
    tmpline=tmpstr;

    top = curr = newItem = NULL;

    if ((src == NULL) || (src[0] == '\0')) {
        return ((DNode *)NULL);
    }

    for (tok=strtok(tmpline,sep); tok!=NULL;tok=strtok(NULL,"=")) {
        
        newItem=(DNode *) malloc( sizeof( DNode ) );
        strcpy(newItem->dname,tok);
        newItem->next=NULL;

        if( curr == NULL ) {
            top = newItem;
        } else {
            curr->next=newItem;
        }
        curr = newItem;
    }
    return (DNode *)top;	
}

char* RemoteFacade::chomp( char *src ) 
{
    char tmpstr[SBUFF];
    char *tmpline = NULL;
    int i;
    
    //First, remove leading spaces
    while( ( (isspace(src[0])) && (src[0])) != '\0') {
        src++;
    }
    strcpy(tmpstr,src);
    for(i=strlen(tmpstr)-1; i>=0; i--) {
        // Remove carriage returns
        if(tmpstr[i] == '\r') {
            tmpstr[i] = '\0';
            continue;
        }
        // Remove line feeds
        if(tmpstr[i] == '\n') {
            tmpstr[i] = '\0';
            continue;
        }
        // Finally, remove trailing spaces
        if(isspace(tmpstr[i])) {
            tmpstr[i] = '\0';
        } else {
            break;
        }
    }
    if(tmpstr[0] == '\0') {
        tmpline=NULL;
    } else {
        strcpy(src, tmpstr);
        tmpline=src;
    }
    return (tmpline);
}

int RemoteFacade::getPPCMTM(UXcmd cmdopts, char* sType)
{
#define PROC_DEV_MODEL "/proc/device-tree/model"
    int mtm = -1;
    FILE *sshfp = NULL;
    char cmdstr[SBUFF] = {0};
    char buff[MAXLINE] = {0};
    strcpy(cmdstr, "cat ");
    strcat(cmdstr, PROC_DEV_MODEL);

    //execute the cmd
    if(m_isConnectFailed){
         return -1;
    }
    else{
        sshfp = sshopen(cmdopts, cmdstr);
    }

    //parse the result of execution
    if (sshfp == NULL) {
        return -1;
    }
    while(fgets(buff, MAXLINE, sshfp) != NULL){
        char *tmp = buff;
        if(strstr(buff, "No such file or directory") != NULL){
            break;
        }
        char chMTM[8] = {0};
        while( *tmp != '\0' ){
            //if there are 4 consecutive digit, we conside it as the MTM
            if( isdigit(*tmp) && isdigit(*(tmp+1)) && isdigit(*(tmp+2)) && isdigit(*(tmp+3)) ){
                strncpy(chMTM, tmp, sizeof(chMTM)-1);
                strncpy(sType, chMTM, 4);
                mtm = atoi(chMTM);
            }
            tmp++;
        }

        if (strstr(buff, "__End__") != NULL) {
            break;
        }
    }
    fclose(sshfp);

    return mtm;
}

}//namespace ReCBB
