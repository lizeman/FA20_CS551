// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
//
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Jiaying Lu
//

#include <ar.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define ERR_USAGE "USAGE: myar [qxotvd:A:] archive-file [file1 .....]\n"
#define ERR_OPEN "myar: %s: No such file or directory\n"
#define ERR_INFORMAT "myar: %s: File format not recognized\n"

struct meta {
    char name[16];   // room for null
    int mode;
    int size;
    time_t mtime;   // a time_t is a long
};

int fill_meta(struct ar_hdr hdr, struct meta * meta) {
    char * fname = malloc(sizeof(hdr.ar_name));
    // truncate fname by '/'
    size_t i;
    for (i=0; i<sizeof(hdr.ar_name)-1; i++) {
        if ('/'== hdr.ar_name[i]) break;
        fname[i] = hdr.ar_name[i];
    }
    fname[i] = '\0';
    strcpy(meta->name, fname);
    printf("meta name=%s, strlen=%lu\n", meta->name, strlen(meta->name));

    printf("hdr ar_mode: %s\n", hdr.ar_mode);
    // TODO: modify hard code
    meta->mode = 0;
    meta->mode += (hdr.ar_mode[3]-'0') * 64;
    meta->mode += (hdr.ar_mode[4]-'0') * 8;
    meta->mode += (hdr.ar_mode[5]-'0') * 1;
    printf("meta (decimal)mode: %d\n", meta->mode);

    printf("hdr size:");
    meta->size = 0;
    for (i=0; i<sizeof(hdr.ar_size); i++) {
        if (hdr.ar_size[i] < '0' || hdr.ar_size[i] > '9') break;
        meta->size *= 10;
        meta->size += (hdr.ar_size[i] - '0');
        printf("_%c_", hdr.ar_size[i]);
    }
    printf("\n");
    printf("meta size: %d\n", meta->size);
    
    meta->mtime = 0;
    for (i=0; i<sizeof(hdr.ar_date); i++) {
        if (hdr.ar_date[i] < '0' || hdr.ar_date[i] > '9') break;
        meta->mtime *= 10;
        meta->mtime += (hdr.ar_date[i] - '0');
    }
    printf("mtime: %ld\n", meta->mtime);
    free(fname);
}


int main (int argc, char *argv[]) {
    extern char *optarg;
    extern int optind;
    int opt;
    char * ar_fname = NULL;
    int ar_fd;
    char * buf_ARMAG = malloc(SARMAG);
    struct ar_hdr * p_ar_hdr = malloc(sizeof(struct ar_hdr));
    struct meta * p_meta = malloc(sizeof(struct meta));

    while ((opt = getopt(argc, argv, "qxotvd:A:")) != -1) {
        switch (opt) {
            case 't':
                break;
            case '?':
                return 1;
                break;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, ERR_USAGE);
        fprintf(stderr, "missing archive-file\n");
        return 1;
    }

    ar_fname = argv[optind++];
    printf("archive_file=%s\n", ar_fname);

    ar_fd = open(ar_fname, O_RDONLY);
    if (ar_fd < 0) {
        fprintf(stderr, ERR_OPEN, ar_fname);
        return 1;
    }

    if (read(ar_fd, buf_ARMAG, SARMAG) < 0) {
        //TODO: maybe change err msg
        fprintf(stderr, "read SARMAG failed\n");
        return 1;
    }
    if (strcmp(ARMAG, buf_ARMAG)) {
        fprintf(stderr, ERR_INFORMAT, ar_fname);
        return 1;
    }
    
    // Start read AR_HDR, File
    if (read(ar_fd, p_ar_hdr, sizeof(struct ar_hdr)) < 0) {
        fprintf(stderr, "read_ar_hdr failed");
        return 1;
    }
    printf("%s\n", p_ar_hdr->ar_name);
    fill_meta(*p_ar_hdr, p_meta);
    //read_ar_hdr(ar_fd, p_ar_hdr);
    /*
    while (read(ar_fd, buf_ar_hdr, sizeof(struct ar_hdr)) > 0) {
    
    }
    */

    free(p_meta);
    free(p_ar_hdr);
    free(buf_ARMAG);
    return 0;
}
