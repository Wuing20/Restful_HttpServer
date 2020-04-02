#ifndef CURLFILE_H_
#define CURLFILE_H_
 
#include "curl/curl.h"
#include <iostream>
#include <string>
#include <atomic>
#include <unistd.h>
#include <stdlib.h>
//#include <sys/types.h>
//#include <sys/stat.h>


#define __int64 long long
 
namespace curl {
 
    class CurlFile {
    public:
        CurlFile();
        ~CurlFile();
 
        double GetRemoteFileSize();
        bool Download();//下载前，需要设置url和下载位置
        void SetNow(__int64 now); //设置下载大小
        __int64 ResumeLocalFileSize();//本地已经下载的文件大小，用于断点续传
        bool Pause();
        bool Resume();
        bool Remove();
 
        //property
        void set_url(const std::string& url);
        std::string url() const;
        CURLcode url_code() const;
        long response_code() const;
        __int64 Now() const; //返回当前下载大小
        __int64 total_file_size() const;
        std::string local_path() const;
        void set_local_path(const std::string& local_path);
        bool is_success() const;
        bool cancel_download() const;
        bool being_download() const;
 
    private:
        CurlFile(const CurlFile&) = delete;
        CurlFile& operator=(const CurlFile&) = delete;
 
        static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
        static int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
        
        std::string url_ = {""};
        std::string local_path_ = {""};
        CURLcode url_code_ = { CURLE_OK };
        long response_code_ = { 200 };
        __int64 local_file_size_ = { 0 };
        std::atomic_llong current_downloaded_size_ = { 0 };
        std::atomic_llong total_file_size_ = { 0 };
        std::atomic_bool cancel_download_ = { false };
        std::atomic_bool being_download_ = { false };
        std::atomic_bool is_success_ = { false };
        CURL *curl_handle_ = { nullptr };
 
    };
}
 
 
 
 
#endif
