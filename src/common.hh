#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>

#include <fnmatch.h>
#include <getopt.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include <readline/history.h>
#include <readline/readline.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <functional>
#include <string>
#include <vector>
using namespace std;

typedef unsigned long long ull;

void exit_all();
