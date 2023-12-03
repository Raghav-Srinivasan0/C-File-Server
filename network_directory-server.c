#include "network_directory.h"

int main(int argc, char **argv)
{
    int pk = atoi(argv[3]);
    int m = atoi(argv[4]);
    start_server(argv[1], argv[2], pk, m);
    return 0;
}