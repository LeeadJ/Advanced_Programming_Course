#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>

void print_tree(const char *path, int indent, int *dir_count, int *file_count);


int main(int argc, char *argv[]) {
    const char *path = ".";

    if (argc > 1) {
        path = argv[1];
    }

    int dir_count = 0;
    int file_count = 0;

    print_tree(path, 0, &dir_count, &file_count);

    printf("\n%d directories, %d files\n", dir_count, file_count);

    return 0;
}

void print_permissions(mode_t mode) {
    char permissions[10];

    permissions[0] = (S_ISDIR(mode)) ? 'd' : '-';
    permissions[1] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (mode & S_IXUSR) ? 'x' : '-';
    permissions[4] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (mode & S_IXGRP) ? 'x' : '-';
    permissions[7] = (mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (mode & S_IXOTH) ? 'x' : '-';

    printf("[");
    printf("%s", permissions);
    // printf("]");
}

void print_tree(const char *path, int indent, int *dir_count, int *file_count) {
    struct dirent *entry;
    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);

        struct stat filestat;
        if (stat(filepath, &filestat) < 0) {
            perror("stat");
            continue;
        }

        struct passwd *user = getpwuid(filestat.st_uid);
        struct group *grp = getgrgid(filestat.st_gid);
        const char *username = (user != NULL) ? user->pw_name : "Unknown";
        const char *groupname = (grp != NULL) ? grp->gr_name : "Unknown";

        for (int i = 0; i < indent; i++) {
            printf("│   ");
        }

        printf("├── ");
        print_permissions(filestat.st_mode);
        printf(" %s %s %10lld ] %s\n",
               username, groupname, (long long)filestat.st_size, entry->d_name);

        if (S_ISDIR(filestat.st_mode)) {
            (*dir_count)++;
            print_tree(filepath, indent + 1, dir_count, file_count);
        } else {
            (*file_count)++;
        }
    }

    closedir(dir);
}
