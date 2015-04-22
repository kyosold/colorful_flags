CC = gcc -g
INCS = -I./confparser/ -I/usr/local/include/ -I/usr/local/include/ImageMagick-6/ -I/usr/local/webserver/mysql/include/mysql/ -I/usr/local/json-c/include/
LDS = -L/usr/local/lib64/ -L/usr/local/lib/ -L/usr/local/webserver/mysql/lib/mysql/ -L/usr/local/json-c/lib/ -lMagickWand-6.Q16 -lmysqlclient -lcurl -ljson-c #-lmemcached

OBJS = liao_server

all: $(OBJS)

$(OBJS):	confparser/libconfparser.a \
			liao_server.o	\
			liao_log.o	\
			liao_utils.o	\
			liao_commands.o \
			base64.o
	$(CC) -o $(OBJS) liao_server.o \
			liao_log.o liao_utils.o \
			liao_commands.o \
			base64.o \
			confparser/libconfparser.a $(INCS) $(LDS)

liao_server.o:	liao_server.c
	$(CC) -c -o liao_server.o liao_server.c $(INCS)

liao_log.o:	liao_log.c
	$(CC) -c -o liao_log.o liao_log.c $(INCS)

liao_utils.o: liao_utils.c
	$(CC) -c -o liao_utils.o liao_utils.c $(INCS) -DMAGICKCORE_QUANTUM_DEPTH=16 -DMAGICKCORE_HDRI_ENABLE=1

liao_commands.o: liao_commands.c
	$(CC) -c -o liao_commands.o liao_commands.c $(INCS)

base64.o: base64.c
	$(CC) -c -o base64.o base64.c $(INCS)

confparser/libconfparser.a:
	cd confparser; make -f linux.mk; cd -






clean:
	cd confparser; make -f linux.mk clean; cd -
	rm -rf *.o
	rm -rf $(OBJS)
	
