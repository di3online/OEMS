#oems: oems.o login.o db.o
#	g++ -o oems oems.o login.o db.o -g -lpq -lxml2 -L/usr/local/lib
#oems.o: oems.cpp login.h
#	g++ -c oems.cpp -g 

oems: mainloop.o login.o db.o common.o getGIDByUID.o getUIDByCookie.o handle_lstet.o handle_einf.o
	g++ -o oems mainloop.o login.o db.o common.o getGIDByUID.o handle_lstet.o getUIDByCookie.o handle_einf.o -g -lpq -lxml2 -L/usr/local/lib
mainloop.o: mainloop.cpp login.h handlers.h
	g++ -c mainloop.cpp -g -Wall
login.o: login.cpp db.h login.h xml.h common.h getGIDByUID.h
	g++ -c login.cpp -g -I/usr/include/libxml2 -Wall
db.o: db.cpp db.h
	g++ -c db.cpp -g -Wall
common.o: common.cpp common.h db.h
	g++ -c common.cpp -g -Wall
getGIDByUID.o: getGIDByUID.cpp getGIDByUID.h common.h db.h
	g++ -c getGIDByUID.cpp -g -Wall
getUIDByCookie.o: getUIDByCookie.cpp getUIDByCookie.h db.h common.h
	g++ -c getUIDByCookie.cpp -g -Wall
handle_lstet.o: handle_lstet.cpp common.h db.h handlers.h getUIDByCookie.h getGIDByUID.h
	g++ -c handle_lstet.cpp -g -Wall -I/usr/include/libxml2
handle_einf.o: handle_einf.cpp common.h db.h handlers.h getGIDByUID.h getGIDByUID.h
	g++ -c handle_einf.cpp -g -Wall -I/usr/include/libxml2

client: client.c
	gcc client.c -o client -g -Wall

clean:
	rm -f mainloop.o login.o db.o common.o oems client

