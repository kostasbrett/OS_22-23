TARGETS = child gates

# Targets to build when running `make`
all: $(TARGETS)


.PHONY: all clean


child: child.c
	gcc child.c -o child


gates: parent.c
	gcc parent.c -o gates

# Run `make clean` to delete binaries and object files.
clean:
	rm -rf $(TARGETS) *.o

