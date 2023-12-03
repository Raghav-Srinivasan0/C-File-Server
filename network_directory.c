#include "network_directory.h"

void
fatal(const char *func, int rv)
{
        fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
        exit(1);
}

file* file_new(char* filename)
{
    FILE* f = fopen(filename,"r");
    if (!f)
    {
        printf("Unable to open file!");
        return NULL;
    }
    file* temp_file = malloc(sizeof(file));
    int n = strlen(filename);
    int i;
    for (i = n-1; filename[i] != '.' && i >= 0; i--);
    temp_file->extension = malloc(sizeof(char)*(n-i+1));
    memcpy(temp_file->extension,&filename[i+1],n-i);
    temp_file->extension[n-i] = '\0';
    i--;
    n = i;
    for (; filename[i] != '/' && i >= 0; i--);
    temp_file->name = malloc(sizeof(char)*(n-i+1));
    memcpy(temp_file->name,&filename[i+1],n-i);
    temp_file->name[n-i] = '\0';
    int c = fgetc(f);
    i = 0;
    while (c != EOF)
    {
        i++;
        c = fgetc(f);
    }
    fclose(f);
    f = fopen(filename,"r");
    temp_file->data = calloc(i+1,sizeof(char));
    c = fgetc(f);
    i = 0;
    while (c != EOF)
    {
        temp_file->data[i] = (char)c;
        i++;
        c = fgetc(f);
    }
    temp_file->data[i] = '\0';
    fclose(f);
    return temp_file;
}

void file_free(file* f)
{
    free(f->data);
    free(f->extension);
    free(f->name);
    free(f);
}

directory* directory_new(char* filename)
{
    DIR* d = opendir(filename);
    if (!d)
    {
        printf("Unable to open directory!");
        return NULL;
    }
    directory* temp_dir = malloc(sizeof(directory));
    struct dirent *dir;
    size_t filename_len = strlen(filename);
    size_t num_files = 0;
    while ((dir = readdir(d)) != NULL)
    {
        char* file_name = dir->d_name;
        size_t file_name_len = strlen(file_name);
        char* truepath = calloc(filename_len+file_name_len+2,sizeof(char));
        memcpy(truepath,filename,filename_len+1);
        strcat(truepath,"/");
        strcat(truepath,file_name);
        struct stat *buf = calloc(1,sizeof(struct stat));
        stat(truepath, buf);
        if(S_ISREG(buf->st_mode)) num_files++;
        free(buf);
        free(truepath);
    }
    closedir(d);
    temp_dir->content = calloc(num_files,sizeof(file*));
    temp_dir->content_len = num_files;
    d = opendir(filename);
    dir = NULL;
    num_files = 0;
    while ((dir = readdir(d)) != NULL)
    {
        char* file_name = dir->d_name;
        size_t file_name_len = strlen(file_name);
        char* truepath = calloc(filename_len+file_name_len+2,sizeof(char));
        memcpy(truepath,filename,filename_len+1);
        strcat(truepath,"/");
        strcat(truepath,file_name);
        struct stat *buf = calloc(1,sizeof(struct stat));
        stat(truepath, buf);
        if(S_ISREG(buf->st_mode))
        {
            temp_dir->content[num_files] = file_new(truepath);
            num_files++;
        }
        free(buf);
        free(truepath);
    }
    closedir(d);
    return temp_dir;
}

int min(int a, int b) {return (a < b) ? a : b;}

bool streq(char* a, char* b, size_t len)
{
    for (size_t i = 0; i<len; i++)
    {
        if (a[i] != b[i]) return false;
    }
    return true;
}

file* directory_search(directory* D,char* name, char* extension) // NEEDS MORE TESTING ON NONEXISTENT FILES
{
    printf("content_len=%ld\n",D->content_len);
    for (size_t i = 0; i<D->content_len; i++)
    {
        printf("content at %ld: %s\n",i,D->content[i]->data);
        if (strlen(name) == strlen(D->content[i]->name) && strlen(extension) == strlen(D->content[i]->extension) && streq(name,D->content[i]->name,strlen(name)) && streq(extension,D->content[i]->extension,strlen(extension))) return D->content[i];
    }
    return NULL;
}

void directory_free(directory* D)
{
    for (size_t i = 0; i<D->content_len; i++)
    {
        file_free(D->content[i]);
    }
    free(D->content);
    free(D);
}

void start_server(char* url, char* dirpath)
{

    nng_socket sock;
    int rv;

    directory* dir = directory_new(dirpath);

    if ((rv = nng_rep0_open(&sock)) != 0) {
        fatal("nng_rep0_open", rv);
    }
    if ((rv = nng_listen(sock, url, NULL, 0)) != 0) {
        fatal("nng_listen", rv);
    }
    for (;;) 
    {
        char *buf = NULL;
        size_t sz;
        if ((rv = nng_recv(sock, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
            fatal("nng_recv", rv);
        }
        printf("buf recieved: %s\n",buf);
        if (streq(REQ, buf,strlen(REQ))) {
            printf("Valid Request\n");
            char* filenamefull = calloc(strlen(buf)-strlen(REQ)+1,sizeof(char));
            memcpy(filenamefull,&buf[strlen(REQ)],strlen(buf)-strlen(REQ));
            printf("Past filenamefull copy\n");
            int i;
            for (i = 0; i<strlen(filenamefull) && filenamefull[i] != '.'; i++);
            printf("Got i = %d\n",i);
            char* filename = calloc(i+1,sizeof(char));
            printf("Allocated filename\n");
            memcpy(filename,filenamefull,i);
            printf("Copied data\n");
            filename[i] = '\0';
            printf("Past filename\n");
            char* ext = calloc(strlen(filenamefull)-i+1,sizeof(char));
            memcpy(ext,&filenamefull[i+1],strlen(filenamefull)-i);
            ext[strlen(filenamefull)-i] = '\0';
            printf("Past ext\n");
            free(filenamefull);
            printf("filename: %s\next: %s\n",filename,ext);
            if (strcmp(filename,"EXIT") == 0)
            {
                free(filename);
                free(ext);
                if ((rv = nng_send(sock, UNABLE, strlen(UNABLE) + 1, 0)) != 0) {
                    fatal("nng_send", rv);
                }
                break;
            } else {
                file* res = directory_search(dir,filename,ext);
                free(filename);
                free(ext);
                if (res==NULL)
                {
                    printf("Unable to find file!\n");
                    if ((rv = nng_send(sock, UNABLE, strlen(UNABLE) + 1, 0)) != 0) {
                        fatal("nng_send", rv);
                    }
                } else {
                    printf("Data: %s\n",res->data);
                    if ((rv = nng_send(sock, res->data, strlen(res->data) + 1, 0)) != 0) {
                        fatal("nng_send", rv);
                    }
                }
            }
        } else {
            buf = NULL;
            break;
        }
        nng_free(buf, sz);
    }
    directory_free(dir);
}

char* client_request(char* url, char* filename)
{
    nng_socket sock;
    int rv;
    size_t sz;
    char *buf = NULL;
    char* full_request = calloc(strlen(filename)+strlen(REQ)+1,sizeof(char));
    strcpy(full_request,REQ);
    strcat(full_request,filename);
    printf("Request: %s\n",full_request);

    if ((rv = nng_req0_open(&sock)) != 0) {
            fatal("nng_socket", rv);
    }
    if ((rv = nng_dial(sock, url, NULL, 0)) != 0) {
            fatal("nng_dial", rv);
    }
    if ((rv = nng_send(sock, full_request, strlen(full_request)+1, 0)) != 0) {
            fatal("nng_send", rv);
    }
    if ((rv = nng_recv(sock, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
            fatal("nng_recv", rv);
    }
    printf("NODE1: RECEIVED DATA %s\n", buf);
    char* buf_alloc = calloc(strlen(buf)+1,sizeof(char));
    memcpy(buf_alloc,buf,strlen(buf)+1);
    nng_free(buf, sz);
    nng_close(sock);
    return buf_alloc;
}