#include "DownloadTask.h"
 #include <functional>
 
namespace curl {
    DownloadTask::DownloadTask() {
    }
 
    DownloadTask::~DownloadTask() {
        if (task_thread_.joinable()) {
            task_thread_.join();
        }
    }
 
    bool DownloadTask::AddSeed(const std::string & url, const std::string & local_path) {
        if (is_runing_) {
            return false;
        }
        std::unique_ptr<CurlFile> curl(new CurlFile());
        curl->set_url(url);
        curl->set_local_path(local_path);
        seeds_.emplace_back(std::move(curl));
 
        return true;
    }
 
    double DownloadTask::GetProgressValue() {
        __int64 total = 0;
        __int64 now = 0;
        __int64 temp_total_size = 0;
        double progress = 0.0;
 
        for (auto iter = seeds_.begin(); iter != seeds_.end(); ++iter) {
            temp_total_size = (*iter)->total_file_size();
            if (temp_total_size <= 0) {
                return progress;
            }
            total += temp_total_size;
            now += (*iter)->Now();
        }
 
        if (total > 0) {
            progress = (now / static_cast<double long>(total)) * 100;
        }
 
        return progress;
    }
 
    void DownloadTask::BeginTask() {
        if (is_runing_) {
            return;
        }
        std::thread task_thread(std::bind(&DownloadTask::Run, this));
        is_runing_ = true;
        task_thread_ = std::move(task_thread);
    }
 
    void DownloadTask::PauseTask() {
        for (auto iter = seeds_.begin(); iter != seeds_.end(); ++iter) {
            (*iter)->Pause();
        }
    }
 
    void DownloadTask::ResumeTask() {
        for (auto iter = seeds_.begin(); iter != seeds_.end(); ++iter) {
            (*iter)->Resume();
        }
    }
 
    void DownloadTask::RemoveTask() {
        for (auto iter = seeds_.begin(); iter != seeds_.end(); ++iter) {
            (*iter)->Remove();
        }
    }
 
    __int64 DownloadTask::GetTaskDownloadSize() {
        __int64 now = 0;
        for (auto iter = seeds_.begin(); iter != seeds_.end(); ++iter) {
            now += (*iter)->Now();
        }
        return now;
    }
 
    __int64 DownloadTask::GetTaskTotalSize() {
        __int64 total = 0;
        __int64 temp_total_size = 0;
        for (auto iter = seeds_.begin(); iter != seeds_.end(); ++iter) {
            temp_total_size = (*iter)->total_file_size();
            total += temp_total_size;
        }
        return total;
    }
 
    void DownloadTask::Run() {
 
        for (auto iter = seeds_.begin(); iter != seeds_.end(); ++iter) {
            (*iter)->GetRemoteFileSize();
            (*iter)->ResumeLocalFileSize();
        }
 
        for (auto iter = seeds_.begin(); iter != seeds_.end(); ++iter) {
            if (!(*iter)->is_success()) {
                bool is_fair = (*iter)->Download();//下载是阻塞的
                if (!is_fair) {
                    break;
                }
            }
        }
 
    }
}
