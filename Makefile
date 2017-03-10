all: wc tr spltac

wc: wc/*
	cd wc && gcc -o mywc *.c

tr: tr/*
	cd tr && gcc -o mytr *.c

spltac: spltac/*
	cd spltac && gcc -o spltac *.c

clean:
	rm -f tr/mytr wc/mywc spltac/spltac

