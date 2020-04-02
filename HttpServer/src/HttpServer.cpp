#include "HttpServer.h"
#include "threadpool.h"
#include <jsoncpp/json/json.h>
#include <fstream>
#include <memory>

//--------------CHttpServer
CHttpServer* CHttpServer::m_Instance(nullptr);
threadpool_t* CHttpServer::m_thp(nullptr);
const int nMinThread = 5;
const int nMaxThread = 100;
const int nQueSize = 100;


CHttpServer::CHttpServer() : m_pSource(nullptr)
{
    m_thp = threadpool_create(nMinThread,nMaxThread,nQueSize);
}

CHttpServer::~CHttpServer()
{
    threadpool_destroy(m_thp);
    if(!m_pSource)
    {
        delete m_pSource;
        m_pSource = nullptr;
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
    m_pSource = new CSource(sPath);

    m_pSource->InitSource();
    m_pSource->ShowSource();

    stMethod_cbFunc stcbFunc[] = {
    {{served::method::GET},{{"/api/v1",cbServed}}},
    {{served::method::GET},{{"/query",cbGetSourceInfo}}},
    {{},{}}
    };

    //将资源uri、http请求模式和对应的回调函数记录在CHttpServer::m_cbMethodFunc中
    for(stMethod_cbFunc *pTemp = stcbFunc; !(pTemp->vMethod.empty()); ++pTemp)
    {
        m_cbMethodFunc.push_back(*pTemp);
    }

    for(const auto& it : m_pSource->m_vFileInfo)           //CHttpServer类是CUri的友元类
    {
        stMethod_cbFunc temp{{served::method::GET},{{it.sUri,cbGetSource}}};

        m_cbMethodFunc.push_back(temp);
    }

}

void CHttpServer::Run(const char* sPath)
{
    static const int nRun = 10;

    InitUri(sPath);

    m_mux.use_before(ParseReqHead);
    for(const auto& i : m_cbMethodFunc)
    {
        auto it = i.path_cbfunc.begin();
        served::methods_handler& handler = m_mux.handle(it->first.c_str());

        for(const auto& j : i.vMethod)
        {
            handler.method(j,it->second);
            ++it;
        }
    }

    cout<<"curl http://localhost:8081/api/v1"<<endl;

    served::net::server server("127.0.0.1","8081",m_mux);
    server.run(nRun);
}

//为/api/v1 api接口返回的信息
void CHttpServer::cbServed(served::response &res,const served::request &req)
{
    long nCount = 0;
    bool bFlag = false;
    Json::Value root;
    Json::Value Item;
    Json::Value arrObj;
    Json::Value arrInfo;

    for(const auto& i : m_Instance->m_cbMethodFunc)
    {
        auto it = i.path_cbfunc.begin();
        bFlag = false;

        nCount = m_Instance->m_pSource->m_vFileInfo.size();
        for(const auto& j : m_Instance->m_pSource->m_vFileInfo)
        {
            --nCount;
            if(it->first == j.sUri)
            {
                Item[it->first] = j.sFileName;
                
                bFlag = true;
                break;
            }
        }

        if(0 == nCount)
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
    cout<<"Etag: "<<req.header("Etag")<<endl;
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
    
    for(const auto& it : m_Instance->m_pSource->m_vFileInfo)
    {
        if(string::npos != it.sFileName.find(sTemp.c_str()))
        {
            int nSize = it.nFileSize; 
            Json::Value root;

            root["FileName"] = it.sFileName.c_str();
            root["FileSize"] = nSize;
            root["Etag"] = it.sMD5;

            string strInfo = root.toStyledString();

            res.set_header("content-type", "application/json");
            res.set_header("Content-Length",convert<string>(it.nFileSize));
            res.set_header("Etag",it.sMD5);
            res.set_body(strInfo);

            return ;
        }
    }

    res<<"NOT FOUND!";
}

//获取某一资源时，返回相关的资源
void CHttpServer::cbGetSource(served::response &res,const served::request &req)
{
    shared_ptr<stReqData> pReqData(new stReqData);

    pReqData->sUri = req.url().path();
    pReqData->pRes = &res;
    pReqData->sEtag = req.header("Etag");
    pReqData->nLowPos = 0;
    pReqData->nHighPos = 0;

    string sRang = req.header("Range");

    if(!sRang.empty())
    {
        eType type = low;

        cout<<"Rang: "<<sRang<<endl;

        pReqData->nLowPos = m_Instance->GetRangNum(type,sRang);
        type = high;
        pReqData->nHighPos = m_Instance->GetRangNum(type,sRang);

        cout<<"nLow: "<<pReqData->nLowPos<<endl;
        cout<<"nHigh: "<<pReqData->nHighPos<<endl;
    }
    threadpool_add(m_thp,SendData,(void *)pReqData.get());
}

long CHttpServer::GetRangNum(eType type,const string& str)
{
    const string sNum("0123456789");
    int nBeg = 0;
    int nEnd = 0;
    long nNum = 0;

    if(low == type)
    {
        nEnd = str.find("-");
    }
    else
    {
        nBeg = str.find("-");
        nEnd = str.size();
    }
    
    for(int i = nBeg;i < nEnd;++i)
    {
        if(sNum.npos != sNum.find(str[i]))
        {
            nNum = nNum*10 + (int)(str[i] - '0');
        }
    }

    return nNum;
}

void* CHttpServer::SendData(void *arg)
{
    stReqData* pData = (stReqData *)arg;
    stFileInfo* pFileInfo = nullptr;

    for(auto& it : m_Instance->m_pSource->m_vFileInfo)
    {
        if(pData->sUri == it.sUri)
        {
            pFileInfo = &it;
            break;
        }
    }

    if(nullptr == pFileInfo)
    {
        *(pData->pRes)<<"NOT FOUND!";

        return nullptr;
    }

    (*(pData)->pRes).set_status(206);
    (*(pData->pRes)).set_header("content-type","*/*");
    (*(pData->pRes)).set_header("Content-Length",convert<string>(pFileInfo->nFileSize));
    (*(pData->pRes)).set_header("Etag",pFileInfo->sMD5);

    if((0 != pData->nHighPos && pData->nLowPos > pData->nHighPos) || pData->nHighPos > pFileInfo->nFileSize)
    {
        (*(pData->pRes)).set_status(416);
        *(pData->pRes)<<"REQUEST RANGE ERROR!";

        return nullptr;
    }

    if((0 == pData->nLowPos && 0 == pData->nHighPos))
    {
        ifstream ifs(pFileInfo->sFilePath);

        (*(pData->pRes)).set_body(string(
                (istreambuf_iterator<char>(ifs)),
                (istreambuf_iterator<char>())
            ));

        ifs.close();
        return nullptr;
    }

    string str = "";
    ifstream ifs(pFileInfo->sFilePath,ios::in | ios::binary);
    int nLen = 0;

    if(!ifs.is_open())
    {
        cerr<<"Open source file error!"<<endl;

        return nullptr;
    }
    ifs.seekg(pData->nLowPos,ios::beg);
    if(0 != pData->nHighPos)
    {
        nLen = pData->nHighPos - pData->nLowPos;
    }
    else
    {
        nLen = pFileInfo->nFileSize - pData->nLowPos;
    }
    shared_ptr<char> pStr(new char(nLen));
    //char *pStr = new char(nLen);

    cout<<"nLen: "<<nLen<<endl;

    ifs.read(pStr.get(),nLen);
    //ifs.getline(pStr,nLen);
    //cout<<"pStr size: "<<strlen(pStr)<<endl;
    str.assign(pStr.get(),nLen);
    cout<<"Data size: "<<str.size()<<endl;
    (*(pData->pRes)).set_body(str);

    sleep(3);
    //delete pStr,
    //pStr = nullptr;
    ifs.close();
    return nullptr;
}

