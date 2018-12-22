#include <stdio.h>
#include <dirent.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define DEBUG 1 // 0 for production

bool isProcessDirectory(const struct dirent *dirent);

int main() {
  struct dirent *dirent;

  DIR *dir = opendir("/proc");

  if (dir == NULL) {
    printf("Could not open proc directory");
    return 1;
  }

  while ((dirent = readdir(dir)) != NULL) {
    bool isProcessDir = isProcessDirectory(dirent);
    if (isProcessDir) {
      #if DEBUG
      printf("found a process dir %s\n", dirent->d_name);
      #endif
    } else {
      #if DEBUG
      printf("not a process dir %s\n", dirent->d_name);
      #endif
    }
  }

  closedir(dir);
  return 0;
}

/**
 * Checks if the given dirent is a valid process directory.
 * Meaning it needs to be a directory and the name needs to be a number;
 *
 * Returns true if valid process directory
 */
bool isProcessDirectory(const struct dirent *dirent) {
  return dirent->d_type == DT_DIR && strtol(dirent->d_name, NULL, 0);
}
