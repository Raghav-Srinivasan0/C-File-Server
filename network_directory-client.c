#include "network_directory.h"

int main(int argc, char **argv)
{
    char *resultDir = "result_dir/";
    while (true)
    {
        char filename[30];
        scanf("%30s", filename);
        if (strcmp("exit_client", filename) == 0)
            break;
        data *res = client_request(argv[1], filename, atoi(argv[2]), atoi(argv[3]));
        if (strcmp("EXIT", filename) == 0)
            break;
        printf("Before memcmp\n");
        if (memcmp(res->data, UNABLE, res->size) == 0)
            continue;
        char *fullfilename = calloc(30 + strlen(resultDir) + 1, sizeof(char));
        memcpy(fullfilename, resultDir, strlen(resultDir) + 1);
        strcat(fullfilename, filename);
        remove(fullfilename);
        printf("Opening %s ...\n", fullfilename);
        FILE *f = fopen(fullfilename, "ab");
        printf("Amt written: %ld\n", fwrite(res->data, 1, res->size, f));
        fclose(f);
        free(fullfilename);
    }
    return 0;
}