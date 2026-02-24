#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>


void print_permissions(mode_t mode) {
  char perms[11];

  if(S_ISDIR(mode)) perms[0] = 'd';
  else if (S_ISLNK(mode)) perms[0] = 'l';
  else perms[0] = '-';

  perms[1] = (mode & S_IRUSR) ? 'r' : '-';
  perms[2] = (mode & S_IWUSR) ? 'w' : '-';
  perms[3] = (mode & S_IXUSR) ? 'x' : '-';

  perms[4] = (mode & S_IRGRP) ? 'r' : '-';
  perms[5] = (mode & S_IWGRP) ? 'w' : '-';
  perms[6] = (mode & S_IXGRP) ? 'x' : '-';

  perms[7] = (mode & S_IROTH) ? 'r' : '-';
  perms[8] = (mode & S_IWOTH) ? 'w' : '-';
  perms[9] = (mode & S_IXOTH) ? 'x' : '-';

  perms[10] = '\0';

  printf("%s ", perms);
}

void print_human_size(long long size) {
  const char *units[] = {"B", "K", "M", "G", "T"};
  int i = 0;
  double sz = (double)size;
  while (sz >= 1024 && i < 4) {
    sz /= 1024;
    i++;
  }
  printf("%6.1f%s ", sz, units[i]);
}

void print_colored(const char *name, mode_t mode) {
  if (S_ISDIR(mode)) {
    printf("\033[34m%s\033[0m", name);
  } else if (S_ISLNK(mode)) {
    printf("\033[36m%s\033[0m", name);
  } else if (mode & S_IXUSR) {
    printf("\033[32m%s\033[0m", name);
  }else {
    printf("%s", name);
  }
}

int long_format = 0;
int show_hidden = 0;
int human_readable = 0;
int main(int argc, char *argv[]) {
  const char *path = ".";

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-l") == 0) {
      long_format = 1;
    }else if(strcmp(argv[i], "-a") == 0) {
      show_hidden = 1;
    }else if(strcmp(argv[i], "-h") == 0){
      human_readable = 1;
    }else {
      path = argv[i];
    }
  }

  DIR *dir = opendir(path);
  if (!dir) {
    perror("opendir");
    return 1;
  }

  struct dirent *entry;
  while((entry = readdir(dir)) != NULL) {
    if (!show_hidden && entry->d_name[0] == '.') 
      continue;
    struct stat file_stat;
    if (long_format) {
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        if(stat(fullpath, &file_stat) == -1) {
          perror("stat");
          continue;
        }
      print_permissions(file_stat.st_mode);

      printf("%2lu ", file_stat.st_nlink);

      struct passwd *pw = getpwuid(file_stat.st_uid);
      printf("%-8s ", pw ? pw->pw_name : "unknwon");

      struct group *gr = getgrgid(file_stat.st_gid);
      printf("%-8s ", gr ? gr->gr_name : "unknown");

      if (human_readable) {
        print_human_size(file_stat.st_size);
      }else {
        printf("%8lld ", (long long) file_stat.st_size);
      }
      
      char timebuf[64];
      struct tm *tm_info = localtime(&file_stat.st_mtime);
      strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", tm_info);

      printf("%s ", timebuf);
      print_colored(entry->d_name, file_stat.st_mode);
      printf("\n");
    } else {
      struct stat file_stat;
      char fullpath[1024];
      snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

      if(stat(fullpath, &file_stat) == 0)
        print_colored(entry->d_name, file_stat.st_mode);
      else 
        printf("%s", entry->d_name);
      printf("\n");
    }
  }

  closedir(dir);
  return 0;
}
