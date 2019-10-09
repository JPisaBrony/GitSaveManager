#include "global.h"

int main(int argc, char *argv[]) {
    interface_init();
    curl_init();
    main_interface();

    /*
    #ifdef _3DS

    printf("Press A to upload files\n");
    printf("Press B to download files\n");

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();

        if (kDown & KEY_START) break;

        if(kDown & KEY_A) {
            FileList *cur = file_list;
            while(cur != NULL) {
                char* url = getUrlFromGistByFilename(cur->name);
                if(url == NULL)
                    createGist(cur->path, cur->name);
                else
                    updateGist(cur->path, url);

                cur = cur->next;
            }
            printf("upload successful\n");
        } else if(kDown & KEY_B) {
            FileList *cur = file_list;
            while(cur != NULL) {
                char* url = getUrlFromGistByFilename(cur->name);
                if(url != NULL)
                    getAndSaveGist(cur->path, url);

                cur = cur->next;
            }

            printf("download successful\n");
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    freeSavefileList(file_list);
    curl_global_cleanup();
    freeCreds();
    socExit();
    gfxExit();

    #else

    if(argc == 1) {
        exit_msg_cmd();
    } else if(argv[1][0] == 'p') {
        curl_global_init(CURL_GLOBAL_ALL);
        getLocalCreds();
        FileList *file_list = getSavefileList();

        FileList *cur = file_list;
        while(cur != NULL) {
            char* url = getUrlFromGistByFilename(cur->name);
            if(url == NULL)
                createGist(cur->path, cur->name);
            else
                updateGist(cur->path, url);

            cur = cur->next;
        }

        freeSavefileList(file_list);
        curl_global_cleanup();
        freeCreds();
    } else if(argv[1][0] == 'g') {
        curl_global_init(CURL_GLOBAL_ALL);
        getLocalCreds();
        FileList *file_list = getSavefileList();

        FileList *cur = file_list;
        while(cur != NULL) {
            char* url = getUrlFromGistByFilename(cur->name);
            if(url != NULL)
                getAndSaveGist(cur->path, url);

            cur = cur->next;
        }

        freeSavefileList(file_list);
        curl_global_cleanup();
        freeCreds();
    } else {
        exit_msg_cmd();
    }

    #endif
    */

    return 0;
}
