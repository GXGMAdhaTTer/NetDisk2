server:main.o
	g++ $^ -o $@ -lwfrest -lworkflow -lssl -lcrypt -lcrypto -lalibabacloud-oss-cpp-sdk -lcurl -lpthread
main.o:main.cc 
	g++ -c $^ -o $@  -std=c++11 -fno-rtti -g

clean:
	$(RM) server main.o
