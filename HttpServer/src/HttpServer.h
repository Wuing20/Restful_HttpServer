#ifndef __MY_HTTP_SERVER_H__
#define __MY_HTTP_SERVER_H__

#include "Source.h"
#include "threadpool.h"

struct stReqData
{
	string sUri;
	served::response *pRes; 
	string sEtag;
	long nLowPos;
	long nHighPos;
};

class CHttpServer
{
public:
	virtual ~CHttpServer();

	static CHttpServer* GetInstance();
	void Run(const char* sPath);

protected:
	void InitUri(const char* sPath);

	static void ParseReqHead(served::response &res,const served::request &req);
	static void cbServed(served::response &res,const served::request &req);
	static void cbGetSourceInfo(served::response &res,const served::request &req);
	static void cbGetSource(served::response &res,const served::request &req);
	
private:
	CHttpServer();

	CHttpServer(CHttpServer const &) = delete;
	CHttpServer& operator=(CHttpServer const &) = delete;

	enum eType{low=0,high};

	long GetRangNum(eType type,const string& str);
	static void* SendData(void *arg);

private:
	static CHttpServer* m_Instance;
	static threadpool_t* m_thp;

	CSource* m_pSource;
	served::multiplexer m_mux;
    vector<stMethod_cbFunc> m_cbMethodFunc;
};


#endif