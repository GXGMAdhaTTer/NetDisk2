#ifndef __FILEUTIL_H__
#define __FILEUTIL_H__

#include "gxgfunc.h"
#include <fcntl.h>
#include <openssl/sha.h>
#include <string>

class FileUtil {
public:
    static std::string sha1File(const char* path) {
        int fd = open(path, O_RDONLY);
        char buf[4096] = {0};
        SHA_CTX sha_ctx;
        SHA1_Init(&sha_ctx);
        // 循环update
        while (1) {
            bzero(buf, sizeof(buf));
            int ret = read(fd, buf, sizeof(buf));
            if (ret == 0) { //读到文件终止符
                break;
            }
            SHA1_Update(&sha_ctx, buf, ret);
        }
        // 最后再final
        unsigned char md[20]; //不是一个可打印字符 是40个16进制数组成
        SHA1_Final(md, &sha_ctx);
        std::string sha1Res;
        char frag[3];
        for (int i=0;i<20;++i) {
            sprintf(frag, "%02x", md[i]);
            sha1Res.append(frag);
        }
        return sha1Res;
    }
};

#endif

