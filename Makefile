CFLAGS = -g
objects = main.o userAction.o utility.o send_receive.o commonFormats.o encode.o
Target = LPFSupport
LPFSupport: $(objects)
	gcc $(CFLAGS) -o $(Target) $(objects) -lpthread

main.o: main.c commonFormats.h userAction.h ipmsg.h send_receive.h
	gcc $(CFLAGS) -c $^

userAction.o: userAction.c userAction.h commonFormats.h
	gcc $(CFLAGS) -c $^

utility.o: utility.c commonFormats.h
	gcc $(CFLAGS) -c $^

send_receive.o: send_receive.c send_receive.h commonFormats.h ipmsg.h userAction.h
	gcc $(CFLAGS) -c $^

commonFormats.o: commonFormats.c ipmsg.h commonFormats.h
	gcc $(CFLAGS) -c $^

encode.o: encode.c
	gcc $(CFLAGS) -c $^

.PHONY: clean
clean:
	rm -f $(Target) $(objects) *.gch
cgch:
	rm -f *.gch
