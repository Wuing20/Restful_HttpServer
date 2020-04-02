#ifndef __SOURCE_H__
#define __SOURCE_H__

#include <served/served.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>

using namespace std;

class CHttpServer;

//记录资源文件的信息
struct stFileInfo
{
    string sUri;
    string sFilePath;
    string sFileName;
    long nFileSize;
    time_t tLastModeify;
    string sMD5;
};

//此结构体用于j记录Uri和相应的回调函数
struct stMethod_cbFunc
{
	vector<served::method> vMethod;		//记录http请求方法
	multimap<string,served::served_req_handler> path_cbfunc;	//记录一个Uri和它所对应的多种请求方法的回调函数
};

//获取指定目录下的资源信息的类
class CSource
{
    friend class CHttpServer;
public:
    CSource(const char* sPath);

    virtual ~CSource();

    bool InitSource();
    void ShowSource();

private:
    CSource() = delete;
    CSource(CSource &) = delete;

    CSource& operator=(const CSource &) = delete;

    bool RecordFileInfo(const char* pFileName);

private:
    const static string m_sUriPath;
    const string m_SourcePath;
    vector<stFileInfo> m_vFileInfo;
};


template<typename out_type,typename in_value>
out_type convert(const in_value & t)
{
    stringstream stream;
    out_type result;//这里存储转换结果

    stream<<t;//向流中传值
    stream>>result;//向result中写入值

    return result;
}

#endif