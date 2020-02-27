.PHONY: all

FLAGS=$(shell pkg-config --libs --cflags gio-2.0 gio-unix-2.0 glib-2.0)

all: rfm-srv client

rfm-srv: server.o rikfan-manager.o
	$(CC) -o $@ $^ $(FLAGS) $(LDFLAGS) 

client: client.o rikfan-manager.o
	$(CC) -o $@ $^ $(FLAGS) $(LDFLAGS)

server.o: server.c
	$(CC) -o $@ $^ -c $(FLAGS) $(CFLAGS)

client.o: client.c
	$(CC) -o $@ $^ -c $(FLAGS) $(CFLAGS)

# minminbus.o: minminbus.c
# 	gcc -o $@ $^ -c -g $(FLAGS)

rikfan-manager.o: rikfan-manager.c
	$(CC) -o $@ $^ -c $(FLAGS) $(CFLAGS)

clean:
	rm -f *.o rfm-srv client

# gen stub
gen:
	gdbus-codegen --generate-c-code rikfan-manager --interface-prefix com.rikor.ares. com.rikor.ares.rikfan.xml
## 	which pkg-config
# 	echo "$(FLAGS)"
# 	which gdbus-codegen
# 	echo $(CC)
# 	gdbus-codegen --generate-c-code rikfan-manager --c-namespace RikFan --interface-prefix com.rikor.ares. com.rikor.ares.rikfan.xml

# dbus
introspect:
	gdbus introspect -e -d com.rikor.ares.rikfan -o /com/rikor/ares/rikfan

call:
	gdbus call -e -d com.rikor.ares.rikfan -o /com/rikor/ares/rikfan -m /com/rikor/ares/rikfan.HelloWorld Rikor
