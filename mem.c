#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parseLine(char* line)
{
    const char* p = line;
    while (*p < '0' || *p > '9') p++;   // skip characters until first digit
    line[strlen(line)-3] = '\0';   // trim " kB" from end of string
    return atoi(p);
}

// these functions return megabytes

double virtual_memory()
{
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmSize:", 7) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result / 1024.0;
}

double physical_memory(void)
{
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];
    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmRSS:", 6) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result / 1024.0;
}

double total_memory_usage(void)
{
    FILE* file = fopen("/proc/meminfo", "r");
    int total = -1, free = -1;
    char line[128];
    if (fgets(line, 128, file) != NULL) {
        if (strncmp(line, "MemTotal:", 9) == 0){
            total = parseLine(line);
        }
    }
    if (fgets(line, 128, file) != NULL) {
        if (strncmp(line, "MemFree:", 8) == 0){
            free = parseLine(line);
        }
    }
    fclose(file);
    return (total - free) / 1024.0;
}
