all:server client
client:client.pb_skeleton.o signup.pb.o
	g++ $^ -o $@ -lsrpc -lworkflow -lcrypt -lprotobuf

client.pb_skeleton.o:client.pb_skeleton.cc
	g++ -c $^ -o $@

signup.pb.o:signup.pb.cc
	g++ -c $^ -o $@

server:server.pb_skeleton.o signup.pb.o
	g++ $^ -o $@ -lsrpc -lworkflow -lcrypt -lprotobuf -lppconsul

server.pb_skeleton.o:server.pb_skeleton.cc
	g++ -c $^ -o $@
