all: memload flash catchall

clean:
	rm -f common.o memload* flash* catchall*

catchall: catchall.o common.o
	gcc -o $@ $^

flash: flash.o common.o
	gcc -o $@ $^

memload: memload.o common.o
	gcc -o $@ $^
