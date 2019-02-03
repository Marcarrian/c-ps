#include <stdio.h>
#include <dirent.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

bool isProcessDir(const struct dirent *dirent);
void handleProcessDir(const char *dirName);
void openStatusFile(const char *fullPath, const char *dirName);
char *trimWhitespace(char*);

static uid_t UID;
static char uidBuffer[32];

int main() {
  struct dirent *dirent;
  UID = getuid();

  // convert UID to string
  snprintf(uidBuffer, 32, "%d", UID);

  DIR *dir = opendir("/proc");

  if (dir == NULL) {
    printf("Could not open proc directory");
    return 1;
  }

  while ((dirent = readdir(dir)) != NULL) {
    bool isProcDir = isProcessDir(dirent);
    if (isProcDir) {
      handleProcessDir(dirent->d_name);
    }
  }

  closedir(dir);
  return 0;
}

/**
 * Checks if the given dirent is a valid process directory.
 * Meaning it needs to be a directory and the name needs to be a number.
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

  char *nameResult;
  char *vmrssResult;
  char *uidResult;
  vmrssResult = "0";
  bool isMyProcess = false;
  
  while(fgets(line, sizeof line, statusFile) != NULL) {    
    char uid[256];
    char name[256];
    char vmrss[256];
    
    if (strstr(line, "Uid")) {
      strcpy(uid, line);
      uidResult = strtok(uid, ":");
      uidResult = strtok(NULL, ":");
      uidResult = trimWhitespace(uidResult);

      for (int i = strcspn(uidResult, "\t "); i < strlen(uidResult); i++) {
	uidResult[i] = 0;
      }

      const int uidFromStatusFile = atoi(uidResult);

      if (uidFromStatusFile == UID) {
	isMyProcess = true;
      }
    }
    if (strstr(line, "Name")) {
      strcpy(name, line);

      const int i = strcspn(name, ":") + 1; // +1 to remove ":" aswell
      memmove(name, name + i, strlen(name) - i);
      // search for newline and set it to 0
      for (int j = strcspn(name, "\n"); j < strlen(name); j++) {
	name[j] = 0;
      }
      nameResult = trimWhitespace(name);
    }
    
    if (strstr(line, "VmRSS")) {
      strcpy(vmrss, line);
      vmrssResult = strtok(vmrss, ":");
      vmrssResult = strtok(NULL, ":");
      vmrssResult = trimWhitespace(vmrssResult);
      // remove "kB" and the trailing whitespace
      char* kbString;
      if (NULL != (kbString = strstr(vmrssResult, "kB"))) {
	kbString[-1] = 0;
	kbString[0] = 0;
	kbString[1] = 0;
      }
    }
    
  }
  if (isMyProcess) {
    printf("%5s %16s %6s\n", dirName, nameResult, vmrssResult);
  }
  fclose(statusFile);
}

/**
 * Trims the leading and trailing whitespace of a string
 *
 * https://stackoverflow.com/a/122721
 */
char *trimWhitespace(char *str)
{
  char *end;

  // Trim leading space
  // Increment the str pointer for each space character
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;
  
  // Trim trailing space
  end = str + strlen(str) - 1; // set the end pointer to the pointer of str + it's length
  // Decrement the pointer for each space character
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';
  
  return str;
}
