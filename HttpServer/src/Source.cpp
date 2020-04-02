#include "Source.h"
#include "md5.h"
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

//--------------CSource
const string CSource::m_sUriPath = "/sources/";

CSource::CSource(const char* sPath) : m_SourcePath(sPath)
{

}

CSource::~CSource()
{
    
}

bool CSource::InitSource()
{
    DIR *dirptr = NULL;
    dirent *dirp = NULL;

    if(NULL == (dirptr = opendir(m_SourcePath.c_str())))
    {
        cerr<<"Open Source Path error!"<<endl;

        exit(-1);
    }

    vector<string> vFile;
    struct stat statbuf;

    while(NULL != (dirp = readdir(dirptr)))
    {
        lstat(dirp->d_name,&statbuf);
         if(0 == strcmp(dirp->d_name,".") || 0 == strcmp(dirp->d_name,".."))
         {
             continue;
         }
       vFile.push_back(dirp->d_name);
    }

    if(vFile.empty())
    {
        return false;
    }

    for(const auto& it : vFile)
    {
        RecordFileInfo(it.c_str());
    }

    return true;
}

bool CSource::RecordFileInfo(const char* pFileName)
{
    if(nullptr == pFileName)
    {
        return false;
    }

    struct stat statbuf;
    string sPath = m_SourcePath + pFileName;

    if((stat(sPath.c_str(),&statbuf)) < 0)
    {
        return false;
    }

    stFileInfo file_info;
    char szStr[50] = "";
    tm tmVal;

    file_info.sUri = m_sUriPath + pFileName;
    file_info.sFilePath = sPath;
    file_info.sFileName = pFileName;
    file_info.nFileSize = statbuf.st_size;
    file_info.tLastModeify = statbuf.st_mtime;
    localtime_r(&statbuf.st_mtime,&tmVal);
    sprintf(szStr,"%d%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d",
    statbuf.st_size,tmVal.tm_year+1900,tmVal.tm_mon+1,
    tmVal.tm_mday,tmVal.tm_hour,tmVal.tm_min,tmVal.tm_sec);

    string str = sPath + file_info.sFileName + szStr;
    MD5 mdObj(str);

    file_info.sMD5 = mdObj.toString();
    m_vFileInfo.push_back(file_info);

    return true;
}

void CSource::ShowSource()
{
    if(m_vFileInfo.empty())
    {
        cerr<<"Record file info error!"<<endl;

        return;
    }

    for(const auto& it : m_vFileInfo)
    {
        cout<<"FilePath: "<<it.sFilePath<<"\t"
                <<"Uri: "<<it.sUri<<"\t"
                <<"FileMD5: "<<it.sMD5<<endl;
    }
}