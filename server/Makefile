server:main.o
	g++ $^ -o $@ -lwfrest -lworkflow -lssl -lcrypt -lcrypto -g
main.o:main.cc
	g++ -c $^ -o $@
