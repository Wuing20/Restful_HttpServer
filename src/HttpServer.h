#ifndef __MY_HTTP_SERVER_H__
#define __MY_HTTP_SERVER_H__

#include <served/served.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <exception>

using namespace std;

//此结构体用于j记录Uri和相应的回调函数
struct stMethod_cbFunc
{
	vector<served::method> vMethod;		//记录http请求方法
	multimap<string,served::served_req_handler> path_cbfunc;	//记录一个Uri和它所对应的多种请求方法的回调函数
};

class CUri		//获取指定目录下的资源
{
	friend class CHttpServer;
public:
	CUri(const char* sPath);

	virtual ~CUri();

	bool InitSource();
	void ShowSource();
	int GetFileSize(const char *sFile);

private:
	CUri() = delete;
	CUri(CUri &) = delete;

	CUri& operator=(const CUri& ) = delete;

private:
	const string m_SourcePath;
	map<string,string> m_UriPath;	//<uri,source_path>
};

class CHttpServer
{
public:
	virtual ~CHttpServer();

	static CHttpServer* GetInstance();
	void Run(const char* sPath);

protected:
	void InitUri(const char* sPath);
	//void GetReqFileName(const served::request &req);

	static void ParseReqHead(served::response &res,const served::request &req);
	static void cbServed(served::response &res,const served::request &req);
	static void cbGetSourceInfo(served::response &res,const served::request &req);
	static void cbGetSource(served::response &res,const served::request &req);
	static void cbPutTask(served::response &res,const served::request &req);

private:
	CHttpServer();

	CHttpServer(CHttpServer const &) = delete;
	CHttpServer& operator=(CHttpServer const &) = delete;

private:
	static CHttpServer* m_Instance;

	CUri* m_pCUri;
	served::multiplexer m_mux;
    vector<stMethod_cbFunc> m_cbMethodFunc;
};


#endif