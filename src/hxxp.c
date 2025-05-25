#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <getopt.h>
#include <curl/curl.h>

typedef struct {
    char* protocol;
    char* method;
    char* body;
    char* url;
    long body_length;
    struct curl_slist* headers;
} hxxp_request;

void usage(FILE *out, const char *name) {
    assert(out && name);

    char *base = strrchr(name, '/');
    fprintf(out, "usage: %s filename [options]\n", (base ? base + 1 : name));
    fputs("Options\n"
          " -h, --help            display this help and exit.\n"
          " -b, --body            output only body of the returned request.\n"
          " -s, --status          output only status code of the returned request.\n"
          " -v, --verbose         output verbose information about request and response.\n"
          "     --version         display version and exit.\n", out);
}

void version() {
    printf("0.0.1\n");
}

size_t read_hxxp_body(void* p, size_t size, size_t nmemb, FILE* fp) {
    size_t max = size * nmemb;
    size_t read_amount = fread(p, 1, max, fp);
    return read_amount;
}

long parse_hxxp_body_length(FILE* fp) {
    long current = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long end = ftell(fp);
    fseek(fp, current - end, SEEK_CUR);
    return end - current;
}

void parse_hxxp_headers(FILE* fp, hxxp_request* request) {
    char* header = malloc(sizeof(char) * getpagesize());

    if (fgets(header, getpagesize(), fp)) {
        if (!(header[0] == '\n')) {
            size_t length = strlen(header);
            header[length - 1] = '\0';
            request->headers = curl_slist_append(request->headers, header);
            parse_hxxp_headers(fp, request);
        }
    }
}

hxxp_request* parse_hxxp(FILE* fp) {
    hxxp_request* request = malloc(sizeof(*request));
    request->headers = NULL;

    // Shebang check
    char *buf = NULL;
    size_t len = 0;

    ssize_t nread = getline(&buf, &len, fp);

    if (strncmp(buf, "#!", 2) == 0) {
        if (nread == -1) {
            perror("getline");
        } else {
            len = 0;
            ssize_t nread = getline(&buf, &len, fp);
            if (nread == -1) {
                perror("getline");
            }
        }
    }

    // First line
    if (sscanf(buf, "%ms %ms %ms\n", &(request->method), &(request->url), &(request->protocol)) == 3) {
        free(buf);
        parse_hxxp_headers(fp, request);
        request->body_length = parse_hxxp_body_length(fp);
        return request;
    } else {
        free(buf);
        return NULL;
    }
}

size_t header_callback(void *ptr, size_t size, size_t nmemb, void *data) {
    size_t total_size = size * nmemb;
    fwrite(ptr, sizeof(char), total_size, stdout);

    return total_size;
}

FILE* interpolate(char* filename) {
    char* command;
    int res = asprintf(&command, "eval \"cat <<< \\\"$(cat \"%s\")\\\"\" 2> /dev/null", filename);
    if (res == -1) {
        perror("asprintf");
        return NULL;
    }

    FILE* fp = popen(command, "r");

    if (!fp) {
        perror("popen");
        return NULL;
    }

    return fp;
}

int main(int argc, char** argv) {
    FILE* fp = NULL;
    int opt;

    // Flags
    bool only_body = false;
    bool only_status = false;
    bool verbose = false;

    // Opts handling
    if (argc == 1) {
        usage(stderr, argv[0]);
        return 1;
    }

    static const struct option opts[] = {
        {"help",    no_argument,       0, 'h'},
        {"version", no_argument,       0, 0x100},
        {"verbose", no_argument,       0, 'v'},
        {"body",    no_argument,       0, 'b'},
        {"status",  no_argument,       0, 's'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "hvbs", opts, NULL)) != -1) {
        switch (opt) {
            case 0x100:
                version();
                return 0;

            case 'v':
                verbose = true;
                continue;

            case 'h':
                usage(stdout, argv[0]);
                return 0;

            case 'b':
                only_body = true;
                continue;

            case 's':
                only_status = true;
                continue;

            case '?':
                fprintf(stderr, "Invalid option\n");
                usage(stderr, argv[0]);
                return 1;

            default:
                break;
        }
    }

    if (only_body && only_status) {
        fprintf(stderr, "Can't mix -s and -b\n");
        return 1;
    }

    // Handle positional arguments
    for (int i = optind; i < argc; i++) {
        if (strcmp(argv[i], "-") == 0) {
            fp = interpolate("/dev/stdin");
            if (!fp) {
                perror("popen");
                return 1;
            }
            break;

        } else if (strcmp(argv[i], "--") == 0) {
            if (i + 1 < argc) {
                fp = interpolate(argv[i + 1]);
                break;
            } else {
                usage(stderr, argv[0]);
                return 1;
            }

        } else {
            fp = interpolate(argv[i]);
            break;
        }
    }

    if (fp == NULL) {
        fprintf(stderr, "Failed to open or interpolate the file\n");
        return 1;
    }

    // Parsing HXXP
    hxxp_request* request = parse_hxxp(fp);

    if (request == NULL) {
        fprintf(stderr, "Failed to parse hxxp file\n");
        return 1;
    }

    // Performing the request
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (curl) {
        // Setup curl
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request->method);
        curl_easy_setopt(curl, CURLOPT_URL, request->url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, request->headers);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_hxxp_body);
        curl_easy_setopt(curl, CURLOPT_READDATA, fp);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request->body_length);

        if (strcmp(request->protocol, "HTTP/1.1") == 0) {
            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
        } else if (strcmp(request->protocol, "HTTP/1.0") == 0) {
            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
        } else if (strcmp(request->protocol, "HTTP/2") == 0) {
            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
        } else if (strcmp(request->protocol, "HTTP/3") == 0) {
            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_3);
        } else if (strcmp(request->protocol, "HTTP") != 0) {
            curl_easy_cleanup(curl);
            fprintf(stderr, "Unsupported HTTP version");
            return 1;
        }

        if (verbose) {
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        }

        if (only_status) {
            curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        } else if (!only_body) {
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
        }

        res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            if (only_status) {
                long code = 0;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
                printf("%ld\n", code);
            }
        } else {
            fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "Failed to initialize curl\n");
        return 1;
    }

    curl_global_cleanup();
    return 0;
}
