#include <curl/curl.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

void usage(FILE *out, const char *name) {
    assert(out && name);

    char *base = strrchr(name, '/');
    fprintf(out, "usage: %s string [options]\n", (base ? base + 1 : name));
    fputs("Options\n"
          " -h, --help            display this help and exit.\n"
          "     --version         display version.\n", out);
}

int version() {
    printf("0.0.1\n");
    return 0;
}

int main(int argc, char** argv) {
    CURL *curl = curl_easy_init();
    char* string = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--version") == 0) {
            version();
            return 0;

        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage(stdout, argv[0]);
            return 0;

        } else if (strcmp(argv[i], "-") == 0) {
            if (i < argc - 1) {
                string = argv[i + 1];
            }
            break;

        } else {
            string = argv[i];
        }
    }

    if (string == NULL) {
        usage(stderr, argv[0]);
        return 1;
    } else {
        char* encoded = curl_easy_escape(curl, string, strlen(string));
        printf("%s\n", encoded);
        return 0;
    }
}
