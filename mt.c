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

  perms[4] = (mode & S_IRUSR) ? 'r' : '-';
  perms[5] = (mode & S_IWUSR) ? 'w' : '-';
  perms[6] = (mode & S_IXUSR) ? 'x' : '-';

  perms[7] = (mode & S_IRUSR) ? 'r' : '-';
  perms[8] = (mode & S_IWUSR) ? 'w' : '-';
  perms[9] = (mode & S_IXUSR) ? 'x' : '-';

  perms[10] = '\0';

  printf("%s ", perms);
}


int long_format = 0;
int main(int argc, char *argv[]) {
  const char *path = ".";

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-l") == 0) {
      long_format = 1;
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
    if (strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..")==0) 
      continue;
    if (long_format) {
        struct stat file_stat;
        if(stat(entry->d_name, &file_stat) == -1) {
          perror("stat");
          continue;
        }
      print_permissions(file_stat.st_mode);

      printf("%2lu ", file_stat.st_nlink);

      struct passwd *pw = getpwuid(file_stat.st_uid);
      printf("%-8s ", pw ? pw->pw_name : "unknwon");

      struct group *gr = getgrgid(file_stat.st_gid);
      printf("%-8s ", gr ? gr->gr_name : "unknown");

      printf("%8lld ", (long long) file_stat.st_size);

      char timebuf[64];
      struct tm *tm_info = localtime(&file_stat.st_mtime);
      strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", tm_info);

      printf("%s ", timebuf);
      printf("%s\n", entry->d_name);
    } else {
      printf("%s\n", entry->d_name);
    }
  }

  closedir(dir);
  return 0;
}
