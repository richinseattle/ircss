clang-8 -fsanitize=fuzzer  -g -O2 -lpthread  -o ircssd ss.c irc.c misc.c sock.c  -lpthread
