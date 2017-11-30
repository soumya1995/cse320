#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include "server.h"

void server_init(void) {

    char *argv[] = {"bin/cream", "100", "8080", "40"};
    int argc = 4;
   main(argc, argv);
}


Test(server_suite, main, .timeout = 2, .init = server_init) {

    printf("xxxxxs\n");

}
