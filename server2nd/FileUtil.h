#ifndef __FILEUTIL__
#define __FILEUTIL__
#include "linuxheader.h"
#include <openssl/sha.h> // sudo apt install libssl-dev
#include <string>
class FileUtil{
public:
    static std::string sha1File(const char *path){
        int fd = open(path,O_RDONLY);
        char buf[4096] = {0};
        SHA_CTX sha_ctx;
        SHA1_Init(&sha_ctx);
        // 循环update
        while(1){
            bzero(buf,sizeof(buf));
            int ret = read(fd,buf,sizeof(buf));
            if(ret == 0){
                break;
            }
            SHA1_Update(&sha_ctx,buf,ret);
        }
        // 最后再final
        unsigned char md[20];//这不是一个可打印字符 40个16进制数组成
        SHA1_Final(md,&sha_ctx);
        std::string sha1Res;
        char frag[3]; //{'1' 'a' '\0'}
        for(int i = 0; i < 20; ++i){
            sprintf(frag,"%02x", md[i]);
            sha1Res.append(frag);
        }
        return sha1Res;
    }
};
#endif