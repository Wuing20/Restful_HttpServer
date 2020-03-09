#include "HttpServer.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <jsoncpp/json/json.h>
#include <fstream>

//--------------CUri
CUri::CUri(const char* sPath) : m_SourcePath(sPath)
{

}

CUri::~CUri()
{
    
}

//获取指定目录下的文件大小
int CUri::GetFileSize(const char *sFile)
{
    if(nullptr == sFile)
    {
        return -1;
    }

    struct stat statbuf;

    if((stat(sFile,&statbuf)) < 0)
    {
        return -1;
    }

    return statbuf.st_size;
}

//将指定目录下的文件路径记录在m_UriPath中
bool CUri::InitSource()
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

    string sTemp("/sources/");
    int i = 1;
    char szStr[30] = "";

    for(auto it : vFile)
    {
        memset(szStr,0,sizeof(szStr));
        sprintf(szStr,"%d",i);                          //将Uri转换为sources/{1..n}的形式
        pair<string,string> Item(sTemp + szStr,m_SourcePath + it);
        m_UriPath.insert(Item);
        ++i;
    }

    return true;
}

void CUri::ShowSource()
{
    if(!m_UriPath.empty())
    {
        for(auto it : m_UriPath)
        {
            cout<<"uri: "<<it.first<<'\t'<<"source: "<<it.second<<endl;
        }
    }
}


//--------------CHttpServer
CHttpServer* CHttpServer::m_Instance(nullptr);

CHttpServer::CHttpServer() : m_pCUri(nullptr)
{
   
}

CHttpServer::~CHttpServer()
{
    if(!m_pCUri)
    {
        delete m_pCUri;
        m_pCUri = nullptr;
    }

   if(!m_Instance)
   {
       delete m_Instance;
       m_Instance = nullptr;
   }
}

//获取单例的实例句柄
CHttpServer* CHttpServer::GetInstance()
{
    if(!m_Instance)
    {
        m_Instance = new CHttpServer();
    }

    return m_Instance;
}

void CHttpServer::InitUri(const char* sPath)
{
    m_pCUri = new CUri(sPath);

    m_pCUri->InitSource();
    m_pCUri->ShowSource();

    stMethod_cbFunc stcbFunc[] = {
    {{served::method::GET},{{"/api/v1",cbServed}}},
    {{served::method::PUT},{{"/task",cbPutTask}}},
    {{served::method::GET},{{"/query",cbGetSourceInfo}}},
    {{},{}}
    };

    //将资源uri、http请求模式和对应的回调函数记录在CHttpServer::m_cbMethodFunc中
    for(stMethod_cbFunc *pTemp = stcbFunc; !(pTemp->vMethod.empty()); ++pTemp)
    {
        m_cbMethodFunc.push_back(*pTemp);
    }

    for(auto it : m_pCUri->m_UriPath)           //CHttpServer类是CUri的友元类
    {
        stMethod_cbFunc temp{{served::method::GET},{{it.first.c_str(),cbGetSource}}};

        m_cbMethodFunc.push_back(temp);
    }

}

void CHttpServer::Run(const char* sPath)
{
    InitUri(sPath);

    m_mux.use_before(ParseReqHead);
    for(auto i : m_cbMethodFunc)
    {
        auto it = i.path_cbfunc.begin();
        served::methods_handler& handler = m_mux.handle(it->first.c_str());

        cout<<it->first.c_str()<<endl;
        for(auto j : i.vMethod)
        {
            handler.method(j,it->second);    
        }
    }

    cout<<"curl http://localhost:8081/api/v1"<<endl;

    served::net::server server("127.0.0.1","8081",m_mux);
    server.run(10);
}

//为/api/v1 api接口返回的信息
void CHttpServer::cbServed(served::response &res,const served::request &req)
{
    int nCount = 0;
    bool bFlag = false;
    Json::Value root;
    Json::Value Item;
    Json::Value arrObj;
    Json::Value arrInfo;

    for(auto i : m_Instance->m_cbMethodFunc)
    {
        auto it = i.path_cbfunc.begin();
        bFlag = false;

        nCount = m_Instance->m_pCUri->m_UriPath.size();
        for(auto j : m_Instance->m_pCUri->m_UriPath)
        {
            --nCount;
            if(it->first == j.first)
            {
                Item[it->first] = j.second;
                
                bFlag = true;
                break;
            }
        }

        if(nCount == 0)
        {
            if(!bFlag)
            {
                arrObj["InstructUri"].append(it->first.c_str());
            }
        }
    }

    arrInfo["SourceUri"].append(Item);
    root["api"].append(arrObj);
    root["api"].append(arrInfo);

    string strInfo = root.toStyledString();

    res.set_header("content-type", "application/json");
    res.set_body(strInfo);
}

//每一个请求到来时先解析请求头部
void CHttpServer::ParseReqHead(served::response &res,const served::request &req)
{
    cout<<method_to_string(req.method())<<"\t"<<req.url().path()<<"\t"<<req.HTTP_version()<<endl;
	cout<<"Accept: "<<req.header("Accept")<<endl;
	cout<<"Referer: "<<req.header("Referer")<<endl;
	cout<<"Accept-Language: "<<req.header("Accept-Language")<<endl;
	cout<<"User-Agent: "<<req.header("User-Agent")<<endl;
	cout<<"Content-Type: "<<req.header("Content-Type")<<endl;
	cout<<"Host: "<<req.header("Host")<<endl;
    cout<<"Range: "<<req.header("Range")<<endl;
	cout<<"Content-Length: "<<req.header("Content-Length")<<endl;
	cout<<"Connection: "<<req.header("Connection")<<endl;
	cout<<"Cache-Control: "<<req.header("Cache-Control")<<endl;
	cout<<"Cookie: "<<req.header("Cookie")<<endl;
	cout<<"\r\n";

	if(!req.body().empty())
	{
		cout<<"body: "<<endl;
		cout<<req.body()<<endl;
	}
}

//查询某一资源的信息时，返回该资源的本地路径和文件大小
void CHttpServer::cbGetSourceInfo(served::response &res,const served::request &req)
{
    string sUri = req.url().path();
    string sTemp = req.query.get("source");

    if(!sTemp.empty())
    {
        sTemp = req.query["source"];
    }
    else
    {
        res<<"INVALID QUERY!";

        return ;
    }
    
    for(auto it : m_Instance->m_pCUri->m_UriPath)
    {
        if(string::npos != it.first.find(sTemp.c_str()))
        {
            int nSize = m_Instance->m_pCUri->GetFileSize(it.second.c_str());

            if(nSize < 0)
            {
                res<<"INVALID QUERY!";

                return ;
            }

            int nPos = it.second.find_last_of('/');
            string sFileName(it.second,nPos+1);
            Json::Value root;

            root["FileName"] = sFileName.c_str();
            root["FileSize"] = nSize;

            string strInfo = root.toStyledString();

            res.set_header("content-type", "application/json");
            res.set_body(strInfo);

            return ;
        }
    }

    res<<"NOT FOUND!";
}

//获取某一资源时，返回相关的资源
void CHttpServer::cbGetSource(served::response &res,const served::request &req)
{
    string sUri = req.url().path();

    for(auto it : m_Instance->m_pCUri->m_UriPath)
    {
        if(it.first == sUri)
        {
            ifstream ifs(it.second);

            res.set_body(string(
                (istreambuf_iterator<char>(ifs)),
                (istreambuf_iterator<char>())
            ));

            res.set_header("content-type","*/*");

            return ;
        }
    }

    res<<"NOT FOUND!";
}

void CHttpServer::cbPutTask(served::response &res,const served::request &req)
{

}
