makeall: screen-worms-server screen-worms-client

screen-worms-server: build
	cp build/screen-worms-server .

screen-worms-client: build
	cp build/screen-worms-client .

build:
	mkdir build
	cmake -S . -B build
	cd build && make

clean:
	rm -f screen-worms-client
	rm -f screen-worms-server
	rm -r -f build
