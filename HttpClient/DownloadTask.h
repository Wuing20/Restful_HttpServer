#ifndef DOWNLOAD_TASK_H_
#define DOWNLOAD_TASK_H_
#include <memory>
#include <vector>
#include <thread>
#include "CurlFile.h"
 
namespace curl {
    class CurlFile;
    class DownloadTask {
    public:
        DownloadTask();
        ~DownloadTask();
 
        bool AddSeed(const std::string& url, const std::string& local_path);
        double GetProgressValue();
        void BeginTask();
        void PauseTask();
        void ResumeTask();
        void RemoveTask();
        __int64 GetTaskDownloadSize();
        __int64 GetTaskTotalSize();
 
    private:
        DownloadTask(const DownloadTask&) = delete;
        DownloadTask& operator=(const DownloadTask&) = delete;
 
        void Run();
 
        std::vector<std::unique_ptr<CurlFile> > seeds_;
        bool is_runing_ = false;
        std::thread task_thread_;
    };
}
 
#endif
