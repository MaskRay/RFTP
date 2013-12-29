.SUFFIXES:
.PHONY: all dist clean release test test-%

DEPEND :=
CXXFLAGS += -g -Wall -Wextra -std=c++11 -D'PACKAGE="ftp"' -DVERSION=0.1
LDLIBS += -lreadline -lpthread
#CXX := clang++
TEX := xelatex -shell-escape -interaction=nonstopmode
P := ftp
BUILD := build
COMMON_SRC := src Makefile
RELEASE_SRC := $(COMMON_SRC) report/report.pdf
DIST_SRC := $(COMMON_SRC) report/*.tex
remove-space = $(subst $(SPACE),,$1)

CLIENT_SRC := $(shell find src/client -name '*.cc') $(wildcard src/*.cc)
SERVER_SRC := $(shell find src/server -name '*.cc') $(wildcard src/*.cc)
CLIENT_OBJ := $(addprefix $(BUILD)/,$(subst src/,,$(CLIENT_SRC:.cc=.o)))
SERVER_OBJ := $(addprefix $(BUILD)/,$(subst src/,,$(SERVER_SRC:.cc=.o)))

all: $(BUILD)/ftp $(BUILD)/ftpd report/report.pdf

test: $(addprefix test-,$(shell find tests -name Makefile -printf '%h\n' | cut -d/ -f2))

test-%: tests/%
	$(MAKE) -C $? test

$(BUILD)/ftp: $(CLIENT_OBJ) | $(BUILD)
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(BUILD)/ftpd: $(SERVER_OBJ) | $(BUILD)
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(BUILD): $(addprefix $(BUILD)/,client server)
	mkdir -p $@

$(BUILD)/client $(BUILD)/server:
	mkdir -p $@

sinclude $(CLIENT_OBJ:.o=.d)
sinclude $(SERVER_OBJ:.o=.d)

$(BUILD)/%.o: src/%.cc | $(BUILD)
	g++ -std=c++11 -Iinclude -MM -MP -MT $@ -MF $(@:.o=.d) $<
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

clean:
	-$(RM) -r $(BUILD)/* *.tar.gz

dist:
	tar zcf '/tmp/$(call remove-space,$(notdir $(CURDIR)))-dist.tar.gz' $(DIST_SRC) --exclude '.*'
