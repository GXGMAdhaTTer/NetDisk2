#ifndef __FILECALLBACKLIB_H__
#define __FILECALLBACKLIB_H__

#include <wfrest/HttpServer.h>
#include "OSS.h"

class FileCallbackLib {
public:
    FileCallbackLib(Config& config);
    void fileUploadGet(const wfrest::HttpReq* req, wfrest::HttpResp* resp);
    void fileUploadPost(const wfrest::HttpReq* req, wfrest::HttpResp* resp);
    void fileUploadSuccess(const wfrest::HttpReq* req, wfrest::HttpResp* resp);
    void fileDownloadGet(const wfrest::HttpReq* req, wfrest::HttpResp* resp);
    void fileQueryPost(const wfrest::HttpReq* req, wfrest::HttpResp* resp, SeriesWork* series);
    void fileDownloadUrlPost(const wfrest::HttpReq* req, wfrest::HttpResp* resp, SeriesWork* series);

private:
    Config& _config;
};

#endif
