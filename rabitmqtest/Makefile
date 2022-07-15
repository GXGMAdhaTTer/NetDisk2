producer:producer.o
	g++ $^ -o $@ -lSimpleAmqpClient

producer.o:producer.cc
	g++ -c $^ -o $@ -std=c++11 -fno-rtti -g

consumer:consumer.o
	g++ $^ -o $@ -lalibabacloud-oss-cpp-sdk -lSimpleAmqpClient -lssl -lcrypt -lcrypto -lcurl -lpthread

consumer.o:consumer.cc
	g++ -c $^ -o $@ -std=c++11 -fno-rtti -g

clean:
	$(RM) producer consumer *.o
