.PHONY: clean run profile test
CFLAGS=-Wall -g
TARGET=pgm3
ARGS = output

$(TARGET): $(TARGET).c

clean:
	rm -f *.o out/*

run:
	./$(TARGET) $(ARGS)

test:
	make
	valgrind ./$(TARGET) $(ARGS)
