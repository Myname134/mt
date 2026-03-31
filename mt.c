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

struct file_entry {
  char name[256];
  struct stat st;
};

int cmp_name(const void *a, const void *b){
  struct file_entry *f1 = (struct file_entry *)a;
  struct file_entry *f2 = (struct file_entry *)b;
  return strcmp(f1->name, f2->name);
}

int cmp_time(const void *a, const void *b) {
  struct file_entry *f1 = (struct file_entry *)a;
  struct file_entry *f2 = (struct file_entry *)b;
  return f2->st.st_mtime - f1->st.st_mtime;
}

int cmp_size(const void *a, const void *b) {
  struct file_entry *f1 = (struct file_entry *)a;
  struct file_entry *f2 = (struct file_entry *)b;
  return f2->st.st_size - f1->st.st_size;
}

int long_format = 0;
int show_hidden = 0;
int human_readable = 0;
int sort_time = 0;
int sort_size = 0;
int reverse_sort = 0;
int main(int argc, char *argv[]) {
  const char *path = ".";

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      for (int j = 1; argv[i][j] != '\0'; j++) {
        if (argv[i][j] == 'l') long_format = 1;
        else if (argv[i][j] == 'a') show_hidden = 1;
        else if (argv[i][j] == 'h') human_readable = 1;
        else if (argv[i][j] == 't') sort_time = 1;
        else if (argv[i][j] == 'S') sort_size = 1;
        else if (argv[i][j] == 'r') reverse_sort = 1;
      }
    } else {
      path = argv[i];
    }
  }

  DIR *dir = opendir(path);
  if (!dir) {
    perror("opendir");
    return 1;
  }

  struct dirent *entry;
  struct file_entry files[1024];
  int count = 0;
  while((entry = readdir(dir)) != NULL) {
    if (!show_hidden && entry->d_name[0] == '.')
      continue;

    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

    if(stat(fullpath, &files[count].st) == -1) {
      perror("stat");
      continue;
    }

    strcpy(files[count].name, entry->d_name);
    count++;
  }
  if (sort_time) 
    qsort(files, count, sizeof(struct file_entry), cmp_time);
  else if (sort_size) 
    qsort(files, count, sizeof(struct file_entry), cmp_size);
  else 
    qsort(files, count, sizeof(struct file_entry), cmp_name);

  int start = reverse_sort ? count - 1 : 0;
  int end = reverse_sort ? -1 : count;
  int step = reverse_sort ? -1 : 1;

  for (int i = start; i != end; i += step) {
    struct file_entry f = files[i];

    if(long_format) {
      print_permissions(f.st.st_mode);

      printf("%2lu ", f.st.st_nlink);

      struct passwd *pw = getpwuid(f.st.st_uid);
      printf("%-8s ", pw ? pw->pw_name : "unknown");

      struct group *gr = getgrgid(f.st.st_gid);
      printf("%-8s ", gr ? gr->gr_name : "unknown");

      if(human_readable)
        print_human_size(f.st.st_size);
      else 
        printf("%8lld ", (long long)f.st.st_size);

      char timebuf[64];
      struct tm *tm_info = localtime(&f.st.st_mtime);
      strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", tm_info);

      printf("%s ", timebuf);
      print_colored(f.name, f.st.st_mode);
      printf("\n");
    } else {
      print_colored(f.name, f.st.st_mode);
      printf("\n");
    }
  }

  closedir(dir);
  return 0;
}
