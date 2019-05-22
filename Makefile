.PHONY: 9cc, test, clean
9cc: 9cc.c
	docker container run -it --rm -v $(PWD):/app --workdir /app gcc:9 gcc -o 9cc 9cc.c

test: 9cc
	docker container run -it --rm -v $(PWD):/app --workdir /app gcc:9 ./test.sh

clean:
	rm -f 9cc *.o *~ tmp*
