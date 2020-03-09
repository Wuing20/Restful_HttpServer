#include "HttpServer.h"


int main(int argc,char *argv[])
{
    
    CHttpServer *pObj = CHttpServer::GetInstance();

    pObj->Run("../www/"); 


    return 0;
}