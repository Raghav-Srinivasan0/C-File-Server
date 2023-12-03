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
#include <math.h>

#define REQ "FILERQ"
#define UNABLE "UNABLE"

typedef struct file_header file;

struct file_header
{
    char *name;
    char *extension;
    void *data;
    size_t data_size;
};

typedef struct directory_header directory;

struct directory_header
{
    size_t content_len;
    file **content;
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
file *directory_search(directory *D, char *name, char *extension);
void start_server(char *url, char *dirpath, int public_key, int mod);
data *client_request(char *url, char *filename, int private_key, int mod);