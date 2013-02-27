ifeq ($(shell uname -s),Darwin)
# OS X ld64 is b0rked, so let's use clang
	LD = clang
endif

ipvec: .obj/ipvec.o .obj/main.o
	$(LD) $(LDFLAGS) -lc -o $@ $^

.obj/%.o: %.c
	@mkdir -p .obj
	$(CC) $(CFLAGS) -Wall -g -std=c99 -c -o $@ $<

clean:
	rm -rf .obj

.PHONY: clean
