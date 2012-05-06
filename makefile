#oems: oems.o login.o db.o
#	g++ -o oems oems.o login.o db.o -g -lpq -lxml2 -L/usr/local/lib
#oems.o: oems.cpp login.h
#	g++ -c oems.cpp -g 

oems: mainloop.o \
	login.o \
	db.o \
	common.o \
	getGIDByUID.o \
	getUIDByCookie.o \
	exam.o \
	paper.o \
	question.o \
	choice.o \
	users.o \
	handle_lste.o \
	handle_einf.o \
	handle_upans.o \
	handle_adde.o \
	handle_meinf.o \
	handle_mpsta.o \
	handle_mqinf.o \
	handle_addq.o \
	handle_users.o \
	handle_dele.o \
	handle_delq.o \
	handle_lstq.o
	g++ -o oems \
		mainloop.o\
	   	login.o \
		db.o \
		common.o \
		getGIDByUID.o \
		getUIDByCookie.o \
		exam.o \
		paper.o \
		question.o \
		choice.o \
		users.o \
		handle_einf.o \
		handle_upans.o \
		handle_lste.o \
		handle_adde.o \
		handle_meinf.o \
		handle_mpsta.o \
		handle_mqinf.o \
		handle_addq.o \
		handle_users.o \
		handle_dele.o \
		handle_delq.o \
		handle_lstq.o \
		-g -lpq -lxml2 -L/usr/local/lib -lpthread
mainloop.o: mainloop.cpp login.h handlers.h
	g++ -c mainloop.cpp -g -Wall -I/usr/include/libxml2
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
exam.o: exam.cpp common.h db.h getUIDByCookie.h getGIDByUID.h
	g++ -c exam.cpp -g -Wall
paper.o: paper.cpp common.h db.h getUIDByCookie.h getGIDByUID.h
	g++ -c paper.cpp -g -Wall
question.o: question.cpp common.h db.h getUIDByCookie.h getGIDByUID.h
	g++ -c question.cpp -g -Wall
choice.o: choice.cpp common.h db.h getUIDByCookie.h getGIDByUID.h
	g++ -c choice.cpp -g -Wall
users.o: users.cpp
	g++ -c users.cpp -g -Wall -I/usr/include/libxml2

handle_lste.o: handle_lste.cpp common.h db.h handlers.h getUIDByCookie.h getGIDByUID.h
	g++ -c handle_lste.cpp -g -Wall -I/usr/include/libxml2
handle_einf.o: handle_einf.cpp common.h db.h handlers.h getGIDByUID.h getUIDByCookie.h
	g++ -c handle_einf.cpp -g -Wall -I/usr/include/libxml2
handle_upans.o: handle_upans.cpp common.h db.h handlers.h getGIDByUID.h getUIDByCookie.h
	g++ -c handle_upans.cpp -g -Wall -I/usr/include/libxml2
handle_adde.o: handle_adde.cpp common.h db.h getUIDByCookie.h getGIDByUID.h
	g++ -c handle_adde.cpp -g -Wall -I/usr/include/libxml2
handle_meinf.o: handle_meinf.cpp common.h db.h getGIDByUID.h getUIDByCookie.h
	g++ -c handle_meinf.cpp -g -Wall -I/usr/include/libxml2 
handle_mpsta.o: handle_mpsta.cpp
	g++ -c handle_mpsta.cpp -g -Wall -I/usr/include/libxml2 
handle_mqinf.o: handle_mqinf.cpp
	g++ -c handle_mqinf.cpp -g -Wall -I/usr/include/libxml2
handle_addq.o: handle_addq.cpp
	g++ -c handle_addq.cpp -g -Wall  -I/usr/include/libxml2
handle_users.o: handle_users.cpp
	g++ -c handle_users.cpp -g -Wall -I/usr/include/libxml2
handle_dele.o: handle_dele.cpp
	g++ -c handle_dele.cpp -g -Wall
handle_delq.o: handle_delq.cpp
	g++ -c handle_delq.cpp -g -Wall
handle_lstq.o: handle_lstq.cpp
	g++ -c handle_lstq.cpp -g -Wall -I/usr/include/libxml2



client: client.c
	gcc client.c -o client -g -Wall

clean:
	rm -f mainloop.o login.o db.o common.o oems client

