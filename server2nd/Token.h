#include <string>
#include <openssl/md5.h>
#include "linuxheader.h"
class Token{
public:
    Token(std::string username, std::string salt):
        username_(username),
        salt_(salt){
            //使用MD5加密
            std::string tokenGen = username_ + salt_;
            unsigned char md[16];
            MD5((const unsigned char *)tokenGen.c_str(),tokenGen.size(),md);
            char frag[3] = {0};
            for(int i = 0;i < 16; ++i){
                sprintf(frag,"%02x",md[i]);
                token = token + frag;
            }

            char timeStamp[20];
            time_t now = time(NULL);
            struct tm *ptm = localtime(&now);
            sprintf(timeStamp,"%02d%02d%02d%02d",ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec);
            
            token = token + timeStamp;
        }
    std::string token;
private:
    std::string username_;
    std::string salt_;
};
