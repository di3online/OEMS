#oems: oems.o login.o db.o
#	g++ -o oems oems.o login.o db.o -g -lpq -lxml2 -L/usr/local/lib
#oems.o: oems.cpp login.h
#	g++ -c oems.cpp -g 

oems: mainloop.o login.o db.o
	g++ -o oems mainloop.o login.o db.o -g -lpq -lxml2 -L/usr/local/lib
mainloop.o: mainloop.cpp login.h
	g++ -c mainloop.cpp -g -Wall
login.o: login.cpp db.h login.h xml.h
	g++ -c login.cpp -g -I/usr/include/libxml2 -Wall
db.o: db.cpp db.h
	g++ -c db.cpp -g -Wall


client: client.c
	gcc client.c -o client -g -Wall

clean:
	rm -f mainloop.o login.o db.o client

