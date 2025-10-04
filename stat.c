#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>
#include <sys/xattr.h>

void print_file_type(mode_t mode) {
    if (S_ISREG(mode)) printf("Regular file\n");
    else if (S_ISDIR(mode)) printf("Directory\n");
    else if (S_ISLNK(mode)) printf("Symbolic link\n");
    else if (S_ISCHR(mode)) printf("Character device\n");
    else if (S_ISBLK(mode)) printf("Block device\n");
    else if (S_ISFIFO(mode)) printf("FIFO (named pipe)\n");
    else if (S_ISSOCK(mode)) printf("Socket\n");
    else printf("Unknown\n");
}

void print_permissions(mode_t mode) {
    printf("Permissions: ");
    printf((mode & S_IRUSR) ? "r" : "-");
    printf((mode & S_IWUSR) ? "w" : "-");
    printf((mode & S_IXUSR) ? "x" : "-");
    printf((mode & S_IRGRP) ? "r" : "-");
    printf((mode & S_IWGRP) ? "w" : "-");
    printf((mode & S_IXGRP) ? "x" : "-");
    printf((mode & S_IROTH) ? "r" : "-");
    printf((mode & S_IWOTH) ? "w" : "-");
    printf((mode & S_IXOTH) ? "x" : "-");
    printf("\n");
}

void print_human_readable_size(off_t size) {
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    double size_in_units = (double)size;

    while (size_in_units >= 1024 && i < 4) {
        size_in_units /= 1024;
        i++;
    }

    printf("Size: %.2f %s\n", size_in_units, units[i]);
}

void print_formatted_time(time_t raw_time) {
    char time_str[100];
    struct tm *time_info = localtime(&raw_time);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);
    printf("%s\n", time_str);
}

void print_extended_attributes(const char *filename) {
    ssize_t attr_size;
    char *attr_name = NULL;
    char value[256];
    int i = 1;

    while ((attr_size = listxattr(filename, attr_name, 0)) > 0) {
        if (attr_size == -1) {
            perror("listxattr");
            return;
        }

        attr_name = malloc(attr_size + 1);
        listxattr(filename, attr_name, attr_size);

        ssize_t value_size = getxattr(filename, attr_name, value, sizeof(value));
        if (value_size != -1) {
            printf("Extended attribute: %s = %s\n", attr_name, value);
        }

        free(attr_name);
        i++;
    }
}

int main(void) {
    const char *filename = "text.txt";
    struct stat file_stat;

    if (stat(filename, &file_stat) == -1) {
        perror("stat");
        return 1;
    }

    printf("File: %s\n", filename);
    printf("--------------------------------------\n");
    printf("File Type: ");
    print_file_type(file_stat.st_mode);
    print_permissions(file_stat.st_mode);


    struct passwd *pw = getpwuid(file_stat.st_uid);
    struct group *gr = getgrgid(file_stat.st_gid);
    if (pw) printf("Owner: %s (UID: %u)\n", pw->pw_name, file_stat.st_uid);
    if (gr) printf("Group: %s (GID: %u)\n", gr->gr_name, file_stat.st_gid);


    printf("Number of links: %lu\n", (unsigned long)file_stat.st_nlink);
    printf("Device ID: %lu\n", (unsigned long)file_stat.st_dev);
    printf("Inode Number: %lu\n", (unsigned long)file_stat.st_ino);
    print_human_readable_size(file_stat.st_size);
    printf("Preferred block size for I/O: %ld bytes\n", (long)file_stat.st_blksize);
    printf("Blocks allocated: %ld\n", (long)file_stat.st_blocks);
    printf("Last accessed: ");
    print_formatted_time(file_stat.st_atime);
    printf("Last modified: ");
    print_formatted_time(file_stat.st_mtime);

    printf("Last status change: ");
    print_formatted_time(file_stat.st_ctime);
    print_extended_attributes(filename);

    return 0;
}