
apigateway:main.o server.pb_skeleton.o signup.pb.o
	g++ $^ -o $@ -lsrpc -lworkflow -lcrypt -lprotobuf -lppconsul

server.pb_skeleton.o:server.pb_skeleton.cc
	g++ -c $^ -o $@