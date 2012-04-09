oems: oems.o login.o db.o
	g++ -o oems oems.o login.o db.o -g -lpq -lxml2 -L/usr/local/lib
oems.o: oems.cpp login.h
	g++ -c oems.cpp -g 
login.o: login.cpp db.h login.h xml.h
	g++ -c login.cpp -g -I/usr/include/libxml2
db.o: db.cpp db.h
	g++ -c db.cpp -g 

clean:
	rm oems.o login.o db.o
