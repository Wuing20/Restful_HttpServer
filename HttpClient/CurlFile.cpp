#include "CurlFile.h"
#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
 
namespace curl {
    CurlFile::CurlFile() {
    }
 
    CurlFile::~CurlFile() {
    }
 
    double CurlFile::GetRemoteFileSize() {
        double file_size = 0.0;
        if (url_.empty()) {
            return file_size;
        }
        CURL *curl_handle = curl_easy_init();
        curl_easy_setopt(curl_handle, CURLOPT_URL, url_.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(curl_handle, CURLOPT_NOBODY, 1);    //不需要body  
        if (curl_easy_perform(curl_handle) == CURLE_OK) {
            curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &file_size);
        }
        curl_easy_reset(curl_handle);
        curl_easy_cleanup(curl_handle);
        total_file_size_ = static_cast<__int64>(file_size);
 
        return file_size;
    }
 
    bool CurlFile::Download() {
        if (url_.empty() || local_path_.empty() || total_file_size_ <= 0) {
            return false;
        }
        __int64 offset_pos = 0;
        std::shared_ptr<FILE> file_obj(fopen(local_path_.c_str(), "ab"), [&](void *ptr) { fclose((FILE *)ptr); });
 
        offset_pos = local_file_size_;
        if (offset_pos > 0) {
            if (offset_pos == total_file_size_) {
                is_success_ = true;
                return true;//已经下载完毕
            }
        }
        curl_handle_ = curl_easy_init();
        being_download_ = true;
 
        CURL *curl_handle = curl_handle_;
        curl_easy_reset(curl_handle);
        /* set URL to get here */
        curl_easy_setopt(curl_handle, CURLOPT_URL, url_.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
        /* enable TCP keep-alive for this transfer */
        curl_easy_setopt(curl_handle, CURLOPT_TCP_KEEPALIVE, 1L);
 
        /* keep-alive idle time to 120 seconds */
        curl_easy_setopt(curl_handle, CURLOPT_TCP_KEEPIDLE, 120L);
 
        /* interval time between keep-alive probes: 60 seconds */
        curl_easy_setopt(curl_handle, CURLOPT_TCP_KEEPINTVL, 60L);
        curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1L);
 
        //当HTTP返回值大于等于400的时候，请求失败, 404错误
        curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1L);
        //curl_easy_setopt(curl_handle, CURLOPT_RESUME_FROM, offset_pos);
        curl_easy_setopt(curl_handle, CURLOPT_RESUME_FROM_LARGE, offset_pos);
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 1800);
        curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 100);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.96 Safari/537.36");
 
        /* send all data to this function  */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
        /* write the page body to this file handle */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, file_obj.get());
        curl_easy_setopt(curl_handle, CURLOPT_XFERINFOFUNCTION, progress_callback);
        curl_easy_setopt(curl_handle, CURLOPT_PROGRESSDATA, this);
        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
        CURLcode code = curl_easy_perform(curl_handle);
        if (code == CURLE_OK) {
            code = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code_);
        }
        being_download_ = true;
        curl_easy_cleanup(curl_handle_);
        curl_handle_ = nullptr;
 
        url_code_ = code;
        if (code == CURLE_OK) {
            is_success_ = true;
        }
 
        return is_success_;
    }
 
    void CurlFile::SetNow(__int64 now) {
        current_downloaded_size_ = now + local_file_size_;
    }
 
    __int64 CurlFile::ResumeLocalFileSize() {
        __int64 local_size = 0;
        if (local_path_.empty()) {
            return local_size;
        }
 
        struct stat file_info;
        int state = stat(local_path_.c_str(), &file_info);
        if (0 == state) {
            local_size = file_info.st_size;
            local_file_size_ = local_size;
            current_downloaded_size_ = local_file_size_;
        }
        return local_size;
    }
 
    bool CurlFile::Pause() {
        if (!being_download()) {
            return false;
        }
 
        if (nullptr == curl_handle_) {
            return false;
        }
        CURLcode code = curl_easy_pause(curl_handle_, CURLPAUSE_RECV);
        if (code != CURLE_OK) {
            return false;
        }
        return true;
    }
 
    bool CurlFile::Resume() {
        if (!being_download()) {
            return false;
        }
        if (nullptr == curl_handle_) {
            return false;
        }
        CURLcode code = curl_easy_pause(curl_handle_, CURLPAUSE_RECV_CONT);
        if (code != CURLE_OK) {
            return false;
        }
        return true;
    }
 
    bool CurlFile::Remove() {
        if (!being_download()) {
            return false;
        }
        cancel_download_ = true;
 
        return true;
    }
 
    size_t CurlFile::write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
        size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
        return written;
    }
 
    int CurlFile::progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
        CurlFile *curl_file = static_cast<CurlFile*>(clientp);
        if (nullptr != curl_file) {
            if (curl_file->cancel_download()) {
                return 1;//取消下载
            }
 
            if (dlnow > 0) {
                curl_file->SetNow(dlnow);
            }
        }
        return 0;
    }
 
    void CurlFile::set_url(const std::string& url) {
        url_ = url;
    }
 
    std::string CurlFile::url() const {
        return url_;
    }
 
    CURLcode CurlFile::url_code() const {
        return url_code_;
    }
 
    long CurlFile::response_code() const {
        return response_code_;
    }
 
    __int64 CurlFile::Now() const {
        return current_downloaded_size_;
    }
 
    __int64 CurlFile::total_file_size() const {
        return total_file_size_;
    }
 
    std::string CurlFile::local_path() const {
        return local_path_;
    }
 
    void CurlFile::set_local_path(const std::string & local_path) {
        local_path_ = local_path;
    }
 
    bool CurlFile::is_success() const {
        return is_success_;
    }
 
    bool CurlFile::cancel_download() const {
        return cancel_download_;
    }
 
    bool CurlFile::being_download() const {
        return being_download_;
    }
 
 
}
