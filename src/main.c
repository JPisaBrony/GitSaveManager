#include "global.h"

int main(int argc, char *argv[]) {
    interface_init();
    curl_init();
    file_init();
    main_interface();
    return 0;
}
