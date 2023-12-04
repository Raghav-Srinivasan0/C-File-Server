#include "network_directory.h"

int main(int argc, char **argv)
{
    char *resultDir = "result_dir/";
    while (true)
    {
        char filename[30];
        printf("\033[0;34m");
        printf("> ");
        printf("\033[0m");
        scanf("%30s", filename);
        if (strcmp("exit_client", filename) == 0)
            break;
        data *res = client_request(argv[1], filename);
        if (strcmp(LIST, filename) == 0)
        {
            printf("\033[0;35m");
            printf("Available Files:");
            printf("\033[0m");
            printf("%s\n\n", (char *)res->data);
            continue;
        }
        if (strcmp(EXIT, filename) == 0)
            break;
        if (memcmp(res->data, UNABLE, res->size) == 0 || memcmp(res->data, ABLE, res->size) == 0)
            continue;
        char *fullfilename = calloc(30 + strlen(resultDir) + 1, sizeof(char));
        memcpy(fullfilename, resultDir, strlen(resultDir) + 1);
        strcat(fullfilename, filename);
        remove(fullfilename);
        printf("\033[0;31m");
        printf("Opening: ");
        printf("\033[0m");
        printf("%s\n", fullfilename);
        FILE *f = fopen(fullfilename, "ab");
        printf("\033[0;35m");
        printf("Amt written:");
        printf("\033[0m");
        printf(" %ld\n", fwrite(res->data, 1, res->size, f));
        fclose(f);
        free(fullfilename);
    }
    return 0;
}