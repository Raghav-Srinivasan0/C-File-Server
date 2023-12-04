#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define REQ "FILERQ"
#define UNABLE "UNABLE"
#define LIST "LIST"
#define REFRESH "REFRESH"
#define EXIT "EXIT"
#define ABLE "ABLE"
#define CD "CD"

typedef struct file_header file;

struct file_header
{
    char *name;
    void *data;
    size_t data_size;
};

typedef struct directory_header directory;

struct directory_header
{
    char* path;
    size_t content_len;
    file **content;
    size_t subdirs_len;
    directory **subdirs;
};

struct data_header
{
    void *data;
    size_t size;
};

typedef struct data_header data;

file *file_new(char *filename);
void file_free(file *f);
void directory_free(directory *D);
directory *directory_new(char *filename);
file *directory_search(directory *D, char *name);
void start_server(char *url, char *dirpath);
data *client_request(char *url, char *filename);