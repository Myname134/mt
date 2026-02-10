#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

int main(int argc, char *argv[]) {
  const char *path = ".";

  if (argc > 1) {
    path = argv[1];
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
    printf("%s\n",entry->d_name);
  }
  closedir(dir);
  return 0;
}
