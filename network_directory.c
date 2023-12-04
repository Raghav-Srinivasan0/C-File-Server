#include "network_directory.h"

void fatal(const char *func, int rv)
{
    fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
    exit(1);
}

long int findSize(char file_name[])
{
    // opening the file in read mode
    FILE *fp = fopen(file_name, "rb");

    // checking if the file exist or not
    if (fp == NULL)
    {
        printf("File Not Found!\n");
        return -1;
    }

    fseek(fp, 0L, SEEK_END);

    // calculating the size of the file
    long int res = ftell(fp);

    // closing the file
    fclose(fp);

    return res;
}

file *file_new(char *filename)
{
    file *temp_file = malloc(sizeof(file));
    int n = strlen(filename);
    int i;
    for (i = n - 1; filename[i] != '/' && i >= 0; i--)
        ;
    temp_file->name = malloc(sizeof(char) * (n - i + 1));
    memcpy(temp_file->name, &filename[i + 1], n - i);
    temp_file->name[n - i] = '\0';
    FILE *f = fopen(filename, "rb");
    if (!f)
    {
        printf("Unable to open file!");
        return NULL;
    }
    temp_file->data = malloc(findSize(filename));
    temp_file->data_size = findSize(filename);
    printf("New File: %s\nLength: %ld\n", temp_file->name, temp_file->data_size);
    printf("Amount read: %ld\n\n", fread(temp_file->data, 1, temp_file->data_size, f));
    fclose(f);
    return temp_file;
}

void file_free(file *f)
{
    free(f->data);
    free(f->name);
    free(f);
}

directory *directory_new(char *filename, directory *parent)
{
    DIR *d = opendir(filename);
    if (!d)
    {
        printf("Unable to open directory!");
        return NULL;
    }
    printf("Creating Directory at %s\n", filename);
    int i = strlen(filename);
    for (; i >= 0 && filename[i] != '/'; i--)
        ;
    if (i > 0)
    {
        char *raw_dir_name = calloc(strlen(filename) - i + 1, sizeof(char));
        memcpy(raw_dir_name, &(filename[i + 1]), strlen(filename) - i);
        raw_dir_name[strlen(filename) - i + 1] = '\0';
        printf("raw_dir_name: %s\n", raw_dir_name);
        if (strcmp(raw_dir_name, "..") == 0 || strcmp(raw_dir_name, ".") == 0)
        {
            printf("Unable\n");
            return NULL;
        }
    }
    directory *temp_dir = malloc(sizeof(directory));
    temp_dir->path = calloc(strlen(filename) + 1, sizeof(char));
    temp_dir->parent = parent;
    memcpy(temp_dir->path, filename, strlen(filename) + 1);
    struct dirent *dir;
    size_t filename_len = strlen(filename);
    size_t num_files = 0;
    size_t num_subdirs = 0;
    while ((dir = readdir(d)) != NULL)
    {
        char *file_name = dir->d_name;
        if (filename[0] == '.')
        {
            printf("Detected invalid\n");
            continue;
        }
        size_t file_name_len = strlen(file_name);
        char *truepath = calloc(filename_len + file_name_len + 2, sizeof(char));
        memcpy(truepath, filename, filename_len + 1);
        strcat(truepath, "/");
        strcat(truepath, file_name);
        struct stat *buf = calloc(1, sizeof(struct stat));
        stat(truepath, buf);
        if (S_ISREG(buf->st_mode))
            num_files++;
        if (S_ISDIR(buf->st_mode))
            num_subdirs++;
        free(buf);
        free(truepath);
    }
    closedir(d);
    temp_dir->content = calloc(num_files, sizeof(file *));
    temp_dir->subdirs = calloc(num_subdirs, sizeof(directory *));
    temp_dir->content_len = num_files;
    temp_dir->subdirs_len = num_subdirs;
    printf("subdirs_len: %ld\n", num_subdirs);
    d = opendir(filename);
    dir = NULL;
    num_files = 0;
    num_subdirs = 0;
    while ((dir = readdir(d)) != NULL)
    {
        char *file_name = dir->d_name;
        size_t file_name_len = strlen(file_name);
        char *truepath = calloc(filename_len + file_name_len + 2, sizeof(char));
        memcpy(truepath, filename, filename_len + 1);
        strcat(truepath, "/");
        strcat(truepath, file_name);
        struct stat *buf = calloc(1, sizeof(struct stat));
        stat(truepath, buf);
        if (S_ISREG(buf->st_mode))
        {
            temp_dir->content[num_files] = file_new(truepath);
            num_files++;
        }
        else if (S_ISDIR(buf->st_mode))
        {
            printf("Before directory_new\n");
            directory *temp_subdir = directory_new(truepath, temp_dir);
            printf("After directory_new\n");
            if (temp_subdir != NULL)
            {
                printf("Valid directory_new\n");
                printf("num_subdirs: %ld\n", num_subdirs);
                temp_dir->subdirs[num_subdirs] = temp_subdir; // BROKEN
                printf("Line 149\n");
                num_subdirs++;
            }
        }
        free(buf);
        free(truepath);
    }
    closedir(d);
    return temp_dir;
}

int min(int a, int b) { return (a < b) ? a : b; }

bool streq(char *a, char *b, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        if (a[i] != b[i])
            return false;
    }
    return true;
}

file *directory_search(directory *D, char *name) // NEEDS MORE TESTING ON NONEXISTENT FILES
{
    printf("content_len=%ld\n", D->content_len);
    for (size_t i = 0; i < D->content_len; i++)
    {
        printf("content at %ld: %s\n", i, (unsigned char *)(D->content[i]->data));
        if (strlen(name) == strlen(D->content[i]->name) && streq(name, D->content[i]->name, strlen(name)))
            return D->content[i];
    }
    return NULL;
}

void directory_free(directory *D)
{
    for (size_t i = 0; i < D->content_len; i++)
    {
        file_free(D->content[i]);
    }
    for (size_t i = 0; i < D->subdirs_len; i++)
    {
        if (D->subdirs[i] != NULL)
            directory_free(D->subdirs[i]);
    }
    free(D->subdirs);
    free(D->content);
    free(D->path);
    free(D);
}

void start_server(char *url, char *dirpath)
{

    nng_socket sock;
    int rv;

    directory *dir = directory_new(dirpath, NULL);

    if ((rv = nng_rep0_open(&sock)) != 0)
    {
        fatal("nng_rep0_open", rv);
    }
    if ((rv = nng_listen(sock, url, NULL, 0)) != 0)
    {
        fatal("nng_listen", rv);
    }
    printf("Server Activated!\n\n");
    for (;;)
    {
        char *buf = NULL;
        size_t sz;
        if ((rv = nng_recv(sock, &buf, &sz, NNG_FLAG_ALLOC)) != 0)
        {
            fatal("nng_recv", rv);
        }
        if (streq(REQ, buf, strlen(REQ)))
        {
            char *filenamefull = calloc(strlen(buf) - strlen(REQ) + 1, sizeof(char));
            memcpy(filenamefull, &buf[strlen(REQ)], strlen(buf) - strlen(REQ));
            printf("filename: %s\n", filenamefull);
            if (strcmp(filenamefull, EXIT) == 0)
            {
                free(filenamefull);
                if ((rv = nng_send(sock, UNABLE, strlen(UNABLE) + 1, 0)) != 0)
                {
                    fatal("nng_send", rv);
                }
                break;
            }
            else if (strcmp(filenamefull, REFRESH) == 0)
            {
                directory *parent = dir->parent;
                directory_free(dir);
                dir = directory_new(dirpath, parent);
                if ((rv = nng_send(sock, ABLE, strlen(ABLE) + 1, 0)) != 0)
                {
                    fatal("nng_send", rv);
                }
            }
            else if (strcmp(filenamefull, LIST) == 0)
            {
                printf("In LIST\n");
                size_t size = 0;
                for (size_t i = 0; i < dir->content_len; i++) // SEGFAULT dir->content_len
                {
                    size += strlen(((dir->content)[i])->name) + 5;
                }
                for (size_t i = 0; i < dir->subdirs_len; i++)
                {
                    printf("Before line 258\n");
                    if ((dir->subdirs)[i] != NULL)
                        size += strlen(((dir->subdirs)[i])->path) + 5;
                    printf("After line 258\n");
                }
                printf("Size: %ld\n", size);
                char *all_names = calloc(size + 2, sizeof(char));
                for (size_t i = 0; i < dir->content_len; i++)
                {
                    strcat(all_names, "\nf - ");
                    strcat(all_names, ((dir->content)[i])->name);
                }
                for (size_t i = 0; i < dir->subdirs_len; i++)
                {
                    if ((dir->subdirs)[i] != NULL)
                    {
                        strcat(all_names, "\nd - ");
                        strcat(all_names, ((dir->subdirs)[i])->path);
                    }
                }
                all_names[size + 2] = '\0';
                if ((rv = nng_send(sock, all_names, size + 1, 0)) != 0)
                {
                    fatal("nng_send", rv);
                }
                free(all_names);
            }
            else if (strcmp(filenamefull, OUT) == 0)
            {
                if (dir->parent != NULL)
                    dir = dir->parent;
                if ((rv = nng_send(sock, ABLE, strlen(ABLE) + 1, 0)) != 0)
                {
                    fatal("nng_send", rv);
                }
            }
            else if (streq(filenamefull, CD, strlen(CD)))
            {
                printf("In CD\n");
                char *newdir = calloc(strlen(filenamefull) - strlen(CD) + 1, sizeof(char));
                memcpy(newdir, &filenamefull[strlen(CD)], strlen(filenamefull) - strlen(CD) + 1);
                printf("newdir: %s\n", newdir);
                directory *old_dir = dir;
                printf("Past line 292\n");
                dir = directory_new(newdir, old_dir);
                if (dir == NULL)
                {
                    printf("REVERTING to old Directory\n");
                    dir = old_dir;
                }
                printf("Past line 294\n");
                if ((rv = nng_send(sock, ABLE, strlen(ABLE) + 1, 0)) != 0)
                {
                    fatal("nng_send", rv);
                }
            }
            else
            {
                file *res = directory_search(dir, filenamefull);
                free(filenamefull);
                if (res == NULL)
                {
                    printf("Unable to find file!\n");
                    if ((rv = nng_send(sock, UNABLE, strlen(UNABLE) + 1, 0)) != 0)
                    {
                        fatal("nng_send", rv);
                    }
                }
                else
                {
                    printf("File Found!\n");
                    printf("Length: %ld\n", res->data_size);
                    if ((rv = nng_send(sock, res->data, res->data_size, 0)) != 0)
                    {
                        fatal("nng_send", rv);
                    }
                }
            }
        }
        if (streq(LISTEN, buf, strlen(LISTEN)))
        {
            char *filenamefull = calloc(strlen(buf) - strlen(LISTEN) + 1, sizeof(char));
            memcpy(filenamefull, &buf[strlen(LISTEN)], strlen(buf) - strlen(REQ));
            printf("filename: %s\n", filenamefull);
            if ((rv = nng_recv(sock, &buf, &sz, NNG_FLAG_ALLOC)) != 0)
            {
                fatal("nng_recv", rv);
            }
            FILE *f = fopen(filenamefull, "ab");
            fwrite(buf, 1, sz, f);
            fclose(f);
        }
        else
        {
            buf = NULL;
            break;
        }
        nng_free(buf, sz);
    }
    directory_free(dir);
}

data *client_request(char *url, char *filename)
{
    nng_socket sock;
    int rv;
    size_t sz;
    char *buf = NULL;
    char *full_request = calloc(strlen(filename) + strlen(REQ) + 1, sizeof(char));
    strcpy(full_request, REQ);
    strcat(full_request, filename);
    printf("\033[0;33m");
    printf("Request:");
    printf("\033[0m");
    printf(" %s\n", full_request);
    if ((rv = nng_req0_open(&sock)) != 0)
    {
        fatal("nng_socket", rv);
    }
    if ((rv = nng_dial(sock, url, NULL, 0)) != 0)
    {
        fatal("nng_dial", rv);
    }
    if ((rv = nng_send(sock, full_request, strlen(full_request) + 1, 0)) != 0)
    {
        fatal("nng_send", rv);
    }
    if ((rv = nng_recv(sock, &buf, &sz, NNG_FLAG_ALLOC)) != 0)
    {
        fatal("nng_recv", rv);
    }
    data *buf_alloc = malloc(sizeof(data));
    buf_alloc->size = sz;
    printf("\033[0;32m");
    printf("Amount Recieved:");
    printf("\033[0m");
    printf(" %ld\n\n", sz);
    buf_alloc->data = malloc(sz);
    memcpy(buf_alloc->data, buf, sz);
    nng_free(buf, sz);
    nng_close(sock);
    return buf_alloc;
}

void client_send(char *url, char *filename)
{
    nng_socket sock;
    int rv;
    size_t sz;
    char *buf = NULL;
    char *full_request = calloc(strlen(filename) + strlen(LISTEN) + 1, sizeof(char));
    strcpy(full_request, LISTEN);
    strcat(full_request, filename);
    printf("\033[0;33m");
    printf("Request:");
    printf("\033[0m");
    printf(" %s\n", full_request);
    if ((rv = nng_req0_open(&sock)) != 0)
    {
        fatal("nng_socket", rv);
    }
    if ((rv = nng_dial(sock, url, NULL, 0)) != 0)
    {
        fatal("nng_dial", rv);
    }
    if ((rv = nng_send(sock, full_request, strlen(full_request) + 1, 0)) != 0)
    {
        fatal("nng_send", rv);
    }
    FILE *f = fopen(filename, "rb");
    char *data = calloc(findSize(filename), sizeof(char));
    fread(data, 1, findSize(filename), f);
    if ((rv = nng_send(sock, data, findSize(filename), 0)) != 0)
    {
        fatal("nng_send", rv);
    }
}