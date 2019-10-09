#include "global.h"

void exit_msg(char* msg) {
    printf("%s\n", msg);
    exit(-1);
}

void exit_msg_cmd() {
    printf("Add either g for get or p push to the command line\n");
    exit(-1);
}
