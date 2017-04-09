.PHONY: app clean
OBJ = main.o NetSocket.o NetListenSocket.o BaseSocketManager.o ClientSocketManager.o GameServerListenSocket.o RemoteEventSocket.o NetworkEventForwarder.o

CPPFLAGS = -Wall -g

%.o: %.cpp
	$(CC) -c -o $@ $< $(CPPFLAGS)

app: $(OBJ)
	g++ -o $@ $^ $(CPPFLAGS)

clean:
	rm *.o
