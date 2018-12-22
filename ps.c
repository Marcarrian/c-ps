#include <stdio.h>
#include <dirent.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define DEBUG 1 // 0 for production

bool isProcessDir(const struct dirent *dirent);
void handleProcessDir(const char *dirName);
void openStatusFile(const char *fullPath, const char *dirName);

int main() {
  struct dirent *dirent;

  DIR *dir = opendir("/proc");

  if (dir == NULL) {
    printf("Could not open proc directory");
    return 1;
  }

  while ((dirent = readdir(dir)) != NULL) {
    bool isProcDir = isProcessDir(dirent);
    if (isProcDir) {
      #if DEBUG
      // printf("found a process dir %s\n", dirent->d_name);
      #endif
      handleProcessDir(dirent->d_name);
    } else {
      #if DEBUG
      // printf("not a process dir %s\n", dirent->d_name);
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
bool isProcessDir(const struct dirent *dirent) {
  return dirent->d_type == DT_DIR && strtol(dirent->d_name, NULL, 0);
}

/**
 * Puts together the full path of the status file
 */
void handleProcessDir(const char *dirName) {
  char fullPath[20];
  strcpy(fullPath, "/proc/");
  strcat(fullPath, dirName);
  strcat(fullPath, "/status");

  // printf("fullPath %s \n", fullPath);

  openStatusFile(fullPath, dirName);
}

/**
 * Opens the status file at the given path.
 */
void openStatusFile(const char *fullPath, const char *dirName) {
  FILE *statusFile;
  char line[128];

  if ((statusFile = fopen(fullPath, "r")) == NULL) {
    printf("unable to open status file %s", fullPath);
    exit(1);
  }

  char names[1024];

  unsigned int i = 0;

  char *nameResult;
  char *vmrssResult;
  
  while(fgets(line, sizeof line, statusFile) != NULL) {
    char name[256];
    char vmrss[256];
      
    if (strstr(line, "Name")) {
      strcpy(name, line);
      nameResult = strtok(name, ":");
      nameResult = strtok(NULL, ":");
    }
    
    if (strstr(line, "VmRSS")) {
      strcpy(vmrss, line);
      vmrssResult = strtok(vmrss, ":");
      vmrssResult = strtok(NULL, ":");
    }
  }
  printf("----------------------------------\n");
  printf("pid: %s\n", dirName);
  printf("name: %s\n", nameResult);
  printf("vmrss: %s\n", vmrssResult);
  printf("----------------------------------\n");
    
  fclose(statusFile);
}
