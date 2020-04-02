#include "DownloadTask.h"
#include <tuple>

using namespace std;


int main(int argc,char* argv[])
{
    using namespace curl;

    if(3 != argc)
    {
        std::cerr<<"Please Usage <url> <local_path>"<<std::endl;

        return -1;
    }

    DownloadTask rObj;

    //rObj.AddSeed("http://localhost:8081/api/v1","./info.txt");
    rObj.AddSeed(argv[1],argv[2]);

    rObj.GetProgressValue();
    rObj.GetTaskDownloadSize();
    rObj.BeginTask();
    

    return 0;
}