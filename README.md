
Using a Debian or Ubuntu Linux system, install the required packages:

	sudo apt-get install git libc-dev gcc make

Clone and build the source code:

	git clone https://github.com/hexluthor/unlif.git
	cd unlif
	make

Run unlif on a LIF file:

	./unlif filename.lif
