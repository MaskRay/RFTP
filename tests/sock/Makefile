.PHONY: test ln

CXXFLAGS += -g -std=c++11
LIB := sock
SRC := main
OBJ := $(LIB:=.o) $(SRC:=.o)

test: main
	echo 5 | socat - tcp-l:9999,reuseaddr &
	sleep 0.05
	./$<

ln:
	-$(foreach i,$(LIB),ln -s ../../src/$i.* -t .;)

main: $(OBJ)
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.o: %.cc
	g++ -std=c++11 -Iinclude -MM -MP -MT $@ -MF $(@:.o=.d) $<
	$(COMPILE.cc) $(OUTPUT_OPTION) $<
