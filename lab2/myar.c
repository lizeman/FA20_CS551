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
#include <utime.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#define BUF_MAX_SIZE 8192
#define TIME_BUF_MAX_SIZE 80
#define SECOND_ONE_DAY 86400

#define ERR_USAGE "USAGE: myar [qxotvd:A:] archive-file [file1 .....]\n"
#define ERR_OPEN "myar: %s: No such file or directory\n"
#define ERR_INFORMAT "myar: %s: File format not recognized\n"
#define ERR_MALFORMED "myar: %s: Malformed archive\n"
#define ERR_UNVALID_OP "myar: two different operation options specified\n"
#define ERR_NON_REG "myar: %s: is not regular file\n"
#define ERR_UNVALID_A "myar: -A %s: argument not valid\n"
#define WARN_NO_ENTRY "no entry %s in archive\n"

struct meta {
    char name[16];   // room for null
    int mode;
    int size;
    time_t mtime;   // a time_t is a long
};

int check_ARMAG(int ar_fd, char * buf, const char * ar_fname) {
    if (read(ar_fd, buf, SARMAG) < 0) {
        //TODO: maybe change err msg
        fprintf(stderr, "read SARMAG failed\n");
        return 1;
    }
    if (strcmp(ARMAG, buf)) {
        fprintf(stderr, ERR_INFORMAT, ar_fname);
        return 1;
    }
    return 0;
}

int fill_meta(struct ar_hdr hdr, struct meta * meta) {
    size_t i;
    for (i=0; i<sizeof(hdr.ar_name); i++) {
        if ('/'== hdr.ar_name[i] || sizeof(hdr.ar_name) == i+1) {
            meta->name[i] = '\0';
            break;
        }
        meta->name[i] = hdr.ar_name[i];
    }

    // TODO: modify hard code; VERY IMPORTANT!!!
    meta->mode = 0;
    meta->mode += (hdr.ar_mode[3]-'0') * 64;
    meta->mode += (hdr.ar_mode[4]-'0') * 8;
    meta->mode += (hdr.ar_mode[5]-'0') * 1;

    meta->size = 0;
    for (i=0; i<sizeof(hdr.ar_size); i++) {
        if (hdr.ar_size[i] < '0' || hdr.ar_size[i] > '9') break;
        meta->size *= 10;
        meta->size += (hdr.ar_size[i] - '0');
    }
    
    meta->mtime = 0;
    for (i=0; i<sizeof(hdr.ar_date); i++) {
        if (hdr.ar_date[i] < '0' || hdr.ar_date[i] > '9') break;
        meta->mtime *= 10;
        meta->mtime += (hdr.ar_date[i] - '0');
    }
}

int fill_ar_hdr(char * filename, struct ar_hdr * hdr) {
    struct stat st;
    size_t i, t;
    if ((stat(filename, &st)) < 0) {
        return 1;
    }
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, ERR_NON_REG, filename);
        return 1;
    }
    t = strlen(filename);
    for (i=0; i<sizeof(hdr->ar_name); i++) {
        if (i < t) hdr->ar_name[i] = filename[i];
        else if (i == t) hdr->ar_name[i] = '/';
        else hdr->ar_name[i] = ' ';
    }
    snprintf(hdr->ar_date, sizeof(hdr->ar_date), "%-12ld", st.st_mtime);
    if (hdr->ar_date[sizeof(hdr->ar_date)-1] == '\0') hdr->ar_date[sizeof(hdr->ar_date)-1] = ' ';
    snprintf(hdr->ar_uid, sizeof(hdr->ar_uid), "%-6u", st.st_uid);
    if (hdr->ar_uid[sizeof(hdr->ar_uid)-1] == '\0') hdr->ar_uid[sizeof(hdr->ar_uid)-1] = ' ';
    snprintf(hdr->ar_gid, sizeof(hdr->ar_gid), "%-6u", st.st_gid);
    if (hdr->ar_gid[sizeof(hdr->ar_gid)-1] == '\0') hdr->ar_gid[sizeof(hdr->ar_gid)-1] = ' ';
    snprintf(hdr->ar_mode, sizeof(hdr->ar_mode), "%-8o", st.st_mode);
    if (hdr->ar_mode[sizeof(hdr->ar_mode)-1] == '\0') hdr->ar_mode[sizeof(hdr->ar_mode)-1] = ' ';
    snprintf(hdr->ar_size, sizeof(hdr->ar_size), "%-10ld", st.st_size);
    if (hdr->ar_size[sizeof(hdr->ar_size)-1] == '\0') hdr->ar_size[sizeof(hdr->ar_size)-1] = ' ';
    memcpy(hdr->ar_fmag, ARFMAG, sizeof(hdr->ar_fmag));
    return 0;
}

void print_verbose_file_info(struct ar_hdr my_ar_hdr, struct meta my_meta) {
    struct tm * tm_info;
    char buf[TIME_BUF_MAX_SIZE];
    size_t i;
    printf( (my_meta.mode & S_IRUSR) ? "r" : "-");
    printf( (my_meta.mode & S_IWUSR) ? "w" : "-");
    printf( (my_meta.mode & S_IXUSR) ? "x" : "-");
    printf( (my_meta.mode & S_IRGRP) ? "r" : "-");
    printf( (my_meta.mode & S_IWGRP) ? "w" : "-");
    printf( (my_meta.mode & S_IXGRP) ? "x" : "-");
    printf( (my_meta.mode & S_IROTH) ? "r" : "-");
    printf( (my_meta.mode & S_IWOTH) ? "w" : "-");
    printf( (my_meta.mode & S_IXOTH) ? "x" : "-");
    printf(" ");
    for (i=0; i<sizeof(my_ar_hdr.ar_uid); i++) {
        if (my_ar_hdr.ar_uid[i] < '0' || my_ar_hdr.ar_uid[i] > '9') break;
        printf("%c", my_ar_hdr.ar_uid[i]);
    }
    printf("/");
    for (i=0; i<sizeof(my_ar_hdr.ar_gid); i++) {
        if (my_ar_hdr.ar_gid[i] < '0' || my_ar_hdr.ar_gid[i] > '9') break;
        printf("%c", my_ar_hdr.ar_gid[i]);
    }
    printf(" %6d", my_meta.size);
    tm_info = localtime(&my_meta.mtime);
    strftime(buf, TIME_BUF_MAX_SIZE, "%b %d %H:%M %Y", tm_info);
    printf(" %s", buf);
    printf(" %s\n", my_meta.name);
}

int operation_t(int ar_fd, const char * ar_fname, int verbose) {
    struct ar_hdr my_ar_hdr;
    struct meta my_meta;
    char buf[3];
    int ret;
    while ((ret = read(ar_fd, &my_ar_hdr, sizeof(struct ar_hdr))) > 0) {
        // check ar_fmag
        strncpy(buf, my_ar_hdr.ar_fmag, 2);
        buf[2] = '\0';
        if (strcmp(ARFMAG, buf)) {
            fprintf(stderr, ERR_MALFORMED, ar_fname);
            return 1;
        } // end check
        fill_meta(my_ar_hdr, &my_meta);
        lseek(ar_fd, my_meta.size, SEEK_CUR);
        if (my_meta.size % 2) lseek(ar_fd, 1, SEEK_CUR);
        if (verbose) {
            print_verbose_file_info(my_ar_hdr, my_meta);
        } else {
            printf("%s\n", my_meta.name);
        }
    }   // end while
    if (ret < 0) {
        fprintf(stderr, "read_ar_hdr failed");
        return 1;
    }
    return ret;
}

int read_write_buffer(int in_fd, int out_fd, char * buf, int size) {
    int read_size = (size < BUF_MAX_SIZE) ? size : BUF_MAX_SIZE;
    int io_size = 0;

    while (read_size > 0 && (io_size = read(in_fd, buf, read_size)) > 0) {
        if (io_size != read_size) return 1;   //TODO: whether need err_msg
        if (write(out_fd, buf, read_size) != read_size) return 1;   //TODO: whether need err_msg
        size -= read_size;
        read_size = (size < BUF_MAX_SIZE) ? size : BUF_MAX_SIZE;
    }
    if (io_size == -1) return 1;   //TODO: whether need err_msg
    return 0;
}

int search_name_in_array(const char * name, char ** array, int start, int end) {
    int hit_flag = 0;
    for (int i=start; i<end; i++) {
        if (!strcmp(name, array[i])) {
            hit_flag = 1;
            array[i][0] = '\0';
        }
    }
    return hit_flag;
}

int check_permission(const char * fname) {
    //TODO: if have time
    struct stat s;
    return 0;
}

int operation_x(int ar_fd, const char * ar_fname, char ** files_to_extract, int start, int end,\
        char * buf, int x_restore) {
    struct ar_hdr my_ar_hdr;
    struct meta my_meta;
    int ret = 0;
    int w_fd;
    while ((ret = read(ar_fd, &my_ar_hdr, sizeof(struct ar_hdr))) > 0) {
        // check ar_fmag
        strncpy(buf, my_ar_hdr.ar_fmag, 2);
        buf[2] = '\0';
        if (strcmp(ARFMAG, buf)) {
            fprintf(stderr, ERR_MALFORMED, ar_fname);
            return 1;
        }  // end check
        fill_meta(my_ar_hdr, &my_meta);
        if (search_name_in_array(my_meta.name, files_to_extract, start, end)) {
            // TODO: check permission
            unlink(my_meta.name);
            if (x_restore) {
                //printf("original mode=%o, ~mode=%o\n", my_meta.mode, (~my_meta.mode) & 0777);
                int mask_to_apply = (~my_meta.mode) & 0777;
                mode_t mask = umask(mask_to_apply);
                w_fd = open(my_meta.name, O_WRONLY | O_CREAT | O_TRUNC, 0777);
                umask(mask);
            } else {
                w_fd = open(my_meta.name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            }
            if (w_fd < 0) {
                fprintf(stderr, "%s: could not write\n", my_meta.name);
                return 1;    
            }
            ret = read_write_buffer(ar_fd, w_fd, buf, my_meta.size);
            close(w_fd);
            if (x_restore) {
                struct utimbuf oldt;
                oldt.actime = my_meta.mtime;
                oldt.modtime = my_meta.mtime;
                if (utime(my_meta.name, &oldt) != 0) return 1;
            }
        } else {
            lseek(ar_fd, my_meta.size, SEEK_CUR);
        }
        if (my_meta.size % 2) lseek(ar_fd, 1, SEEK_CUR);
    }
    
    for (int i=start; i<end; i++) {
        if (strlen(files_to_extract[i]) > 0) fprintf(stderr, WARN_NO_ENTRY, files_to_extract[i]);
    }
    return ret;
}

int operation_x_all(int ar_fd, const char * ar_fname, char * buf, int x_restore) {
    struct ar_hdr my_ar_hdr;
    struct meta my_meta;
    int ret = 0;
    int w_fd;
    while ((ret = read(ar_fd, &my_ar_hdr, sizeof(struct ar_hdr))) > 0) {
        // check ar_fmag
        strncpy(buf, my_ar_hdr.ar_fmag, 2);
        buf[2] = '\0';
        if (strcmp(ARFMAG, buf)) {
            fprintf(stderr, ERR_MALFORMED, ar_fname);
            return 1;
        }  // end check
        fill_meta(my_ar_hdr, &my_meta);
        // TODO: check permission
        unlink(my_meta.name);
        if (x_restore) {
            //printf("original mode=%o, ~mode=%o\n", my_meta.mode, (~my_meta.mode) & 0777);
            int mask_to_apply = (~my_meta.mode) & 0777;
            mode_t mask = umask(mask_to_apply);
            w_fd = open(my_meta.name, O_WRONLY | O_CREAT | O_TRUNC, 0777);
            umask(mask);
        } else {
            w_fd = open(my_meta.name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        }
        if (w_fd < 0) {
            fprintf(stderr, "%s: could not write\n", my_meta.name);
            return 1;    
        }
        ret |= read_write_buffer(ar_fd, w_fd, buf, my_meta.size);
        close(w_fd);
        if (x_restore) {
            struct utimbuf oldt;
            oldt.actime = my_meta.mtime;
            oldt.modtime = my_meta.mtime;
            if (utime(my_meta.name, &oldt) != 0) return 1;
        }
        if (my_meta.size % 2) lseek(ar_fd, 1, SEEK_CUR);
    }
    return ret;
}

int operation_d(int ar_fd, const char * ar_fname, char ** files_to_extract, int start, int end,\
        char * buf, int x_restore) {
    struct ar_hdr my_ar_hdr;
    struct meta my_meta;
    int ret = 0;
    int w_fd;

    unlink(ar_fname);
    // write ARMAG
    if ((w_fd = open(ar_fname, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) return 1;
    if (write(w_fd, ARMAG, SARMAG) != SARMAG) return 1;

    while ((ret = read(ar_fd, &my_ar_hdr, sizeof(struct ar_hdr))) > 0) {
        // check ar_fmag
        strncpy(buf, my_ar_hdr.ar_fmag, 2);
        buf[2] = '\0';
        if (strcmp(ARFMAG, buf)) {
            fprintf(stderr, ERR_MALFORMED, ar_fname);
            return 1;
        }  // end check
        fill_meta(my_ar_hdr, &my_meta);
        if (search_name_in_array(my_meta.name, files_to_extract, start, end)) {
            lseek(ar_fd, my_meta.size, SEEK_CUR);
        } else {
            ret |= write(w_fd, &my_ar_hdr, sizeof(struct ar_hdr));
            ret |= read_write_buffer(ar_fd, w_fd, buf, my_meta.size);
            if (my_meta.size % 2) ret |= write(w_fd, "\n", 1);
        }
        if (my_meta.size % 2) lseek(ar_fd, 1, SEEK_CUR);
    }

    close(ar_fd);
    close(w_fd);
    return ret;
}

int operation_q(int ar_fd, const char * ar_fname, char ** files_to_extract, int start, int end,\
        char * buf) {
    struct ar_hdr my_ar_hdr;
    struct meta my_meta;
    int ret = 0;
    int fd;

    while ((ret = read(ar_fd, &my_ar_hdr, sizeof(struct ar_hdr))) > 0) {
        // check ar_fmag
        strncpy(buf, my_ar_hdr.ar_fmag, 2);
        buf[2] = '\0';
        if (strcmp(ARFMAG, buf)) {
            fprintf(stderr, ERR_MALFORMED, ar_fname);
            return 1;
        }  // end check
        fill_meta(my_ar_hdr, &my_meta);
        lseek(ar_fd, my_meta.size, SEEK_CUR);
        if (my_meta.size % 2) lseek(ar_fd, 1, SEEK_CUR);
    }

    // append files
    if (ret != 0) {
        fprintf(stderr, ERR_MALFORMED, ar_fname);
        return ret;
    }
    for (int i=start; i<end; i++) {
        if ((fd = open(files_to_extract[i], O_RDONLY)) < 0) {
            fprintf(stderr, ERR_OPEN, files_to_extract[i]);
            return 1;
        }
        ret |= fill_ar_hdr(files_to_extract[i], &my_ar_hdr);
        if (ret != 0) return 1;
        if (write(ar_fd, &my_ar_hdr, sizeof(struct ar_hdr)) != sizeof(struct ar_hdr)) return 1;
        fill_meta(my_ar_hdr, &my_meta);
        if ( (ret |= read_write_buffer(fd, ar_fd, buf, my_meta.size)) != 0) return 1;
        if (my_meta.size % 2) write(ar_fd, "\n", 1);
        close(fd);
    }
    return ret;
}

int operation_A(int ar_fd, const char * ar_fname, long N_day, char * buf) {
    struct dirent *de;
    DIR *dr = opendir(".");
    struct ar_hdr my_ar_hdr;
    struct meta my_meta;
    int ret = 0;
    int fd;
    time_t now = time(0);

    while ((ret = read(ar_fd, &my_ar_hdr, sizeof(struct ar_hdr))) > 0) {
        // check ar_fmag
        strncpy(buf, my_ar_hdr.ar_fmag, 2);
        buf[2] = '\0';
        if (strcmp(ARFMAG, buf)) {
            fprintf(stderr, ERR_MALFORMED, ar_fname);
            return 1;
        }  // end check
        fill_meta(my_ar_hdr, &my_meta);
        lseek(ar_fd, my_meta.size, SEEK_CUR);
        if (my_meta.size % 2) lseek(ar_fd, 1, SEEK_CUR);
    }

    // append files
    if (dr == NULL) {
        fprintf(stderr, "myar: Could not open current directory\n");
        return 1;
    }
    while ((de = readdir(dr)) != NULL) {
        if (de->d_type != DT_REG) continue;
        if (!strcmp(de->d_name, ar_fname)) continue;
        if (strlen(de->d_name) > 15) continue;

        if ((fd = open(de->d_name, O_RDONLY)) < 0) {
            fprintf(stderr, ERR_OPEN, de->d_name);
            return 1;
        }
        ret |= fill_ar_hdr(de->d_name, &my_ar_hdr);
        if (ret != 0) return 1;
        fill_meta(my_ar_hdr, &my_meta);
        if ((now - my_meta.mtime) < N_day * SECOND_ONE_DAY) continue;
        if (write(ar_fd, &my_ar_hdr, sizeof(struct ar_hdr)) != sizeof(struct ar_hdr)) return 1;
        if ( (ret |= read_write_buffer(fd, ar_fd, buf, my_meta.size)) != 0) return 1;
        if (my_meta.size % 2) write(ar_fd, "\n", 1);
        close(fd);
    }
}

int main (int argc, char *argv[]) {
    extern char *optarg;
    extern int optind;
    int opt;
    char copt = '\0';
    char * ar_fname = NULL;
    int verbose = 0;
    int x_restore = 0;
    int ar_fd;
    int ret = 0;
    char * buf = malloc(BUF_MAX_SIZE);    // 8KB buffer
    long N_day = -1;
    char * cptr;

    while ((opt = getopt(argc, argv, "qxotvdA:")) != -1) {
        switch (opt) {
            case 't':
                if (copt != '\0' && copt != 'v' && copt != 'o') {
                    fprintf(stderr, ERR_UNVALID_OP);
                    return 1;
                }
                copt = 't';
                break;
            case 'v':
                verbose = 1;
                break;
            case 'x':
                if (copt != '\0' && copt != 'v' && copt != 'o') {
                    fprintf(stderr, ERR_UNVALID_OP);
                    return 1;
                }
                copt = 'x';
                break;
            case 'o':
                x_restore = 1;
                break;
            case 'd':
                if (copt != '\0' && copt != 'v' && copt != 'o') {
                    fprintf(stderr, ERR_UNVALID_OP);
                    return 1;
                }
                copt = 'd';
                break;
            case 'q':
                if (copt != '\0' && copt != 'v' && copt != 'o') {
                    fprintf(stderr, ERR_UNVALID_OP);
                    return 1;
                }
                copt = 'q';
                break;
            case 'A':
                if (copt != '\0' && copt != 'v' && copt != 'o') {
                    fprintf(stderr, ERR_UNVALID_OP);
                    return 1;
                }
                copt = 'A';
                N_day = strtol(optarg, &cptr, 10);
                if (N_day < 0) {
                    fprintf(stderr, ERR_UNVALID_A, optarg);
                    return 1;
                }
                break;
            case '?':
                return 1;
                break;
        }
    }

    if (optind >= argc) {
        // TODO: ERR MSG check
        fprintf(stderr, ERR_USAGE);
        fprintf(stderr, "missing archive-file\n");
        return 1;
    }

    ar_fname = argv[optind++];

    if ((ar_fd = open(ar_fname, O_RDWR | O_CREAT, 0666)) < 0) {
        fprintf(stderr, ERR_OPEN, ar_fname);
        return 1;
    }

    // Check ARMAG magic string
    if ( copt == 'x' || copt == 't' || copt == 'd' ) {
        if ((ret = check_ARMAG(ar_fd, buf, ar_fname)) != 0) return ret;
    } else if ( copt == 'q' || copt == 'A') {
        if (lseek(ar_fd, 0, SEEK_END) == 0) {
            if (write(ar_fd, ARMAG, SARMAG) != SARMAG) return 1;  //TODO: whether need err_msg
        } else {
            lseek(ar_fd, 0, SEEK_SET);
            if ((ret = check_ARMAG(ar_fd, buf, ar_fname)) != 0) return ret;
        }
    } else {
        fprintf(stderr, "myar: no operation specified\n");
        return 1;
    }

    // add logic for -x
    if (copt == 'x') {
        // no specified files
        if (optind == argc) {
            ret = operation_x_all(ar_fd, ar_fname, buf, x_restore);
        } else {
            ret = operation_x(ar_fd, ar_fname, argv, optind, argc, buf, x_restore);
        }
    }

    // add logic for -t
    if (copt == 't') {
        ret = operation_t(ar_fd, ar_fname, verbose);
    }

    // add logic for -d
    if (copt == 'd') {
        if (optind == argc) {
            ;   //DO Nothing
        } else {
            ret = operation_d(ar_fd, ar_fname, argv, optind, argc, buf, x_restore);
        }
    }

    // add logic for -q
    if (copt == 'q') {
        ret = operation_q(ar_fd, ar_fname, argv, optind, argc, buf);
    }

    // add logic for -A
    if (copt == 'A') {
        ret = operation_A(ar_fd, ar_fname, N_day, buf);
    }

    free(buf);
    return ret;
}
