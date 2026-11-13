#define _POSIX_C_SOURCE 200809L   //нужно для dirent.h и dlfcn.h при -std=c11

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include "loader.h"
#include "calc.h"
#include "plugin_api.h"

#define MAX_PATH_LEN 4096

static void** handles = NULL;
static int handles_count = 0;
static int handles_capacity = 0;

static int add_handle(void* h) {
    if (handles_count == handles_capacity) {
        int new_cap = handles_capacity ? handles_capacity * 2 : 8;
        void** tmp = realloc(handles, new_cap * sizeof(void*));
        if (!tmp) return -1;
        handles = tmp;
        handles_capacity = new_cap;
    }
    handles[handles_count++] = h;
    return 0;
}

static int has_so_extension(const char* name) {
    size_t len = strlen(name);
    return len > 3 && strcmp(name + len - 3, ".so") == 0;
}

static int load_one(const char* path) {
    void* h = dlopen(path, RTLD_NOW);
    if (!h) {
        fprintf(stderr, "Ошибка dlopen: %s\n", dlerror());
        return -1;
    }
    if (add_handle(h) != 0) {
        dlclose(h);
        return -1;
    }

    dlerror();
    GetOperationFunc get_op = (GetOperationFunc)dlsym(h, PLUGIN_ENTRY_NAME);
    const char* err = dlerror();
    if (err || !get_op) {
        fprintf(stderr, "%s: нет функции '%s'\n", path, PLUGIN_ENTRY_NAME);
        return -1;
    }

    const OperationInfo* info = get_op();
    if (!info || register_operation(info) != 0) {
        fprintf(stderr, "%s: не удалось зарегистрировать операцию (неверные данные или повтор символа)\n", path);
        return -1;
    }

    printf("  + операция '%c' (приоритет %d) из %s\n", info->symbol, info->priority, path);
    return 0;
}

int load_plugins(const char* dir_path) {
    DIR* dir = opendir(dir_path);
    if (!dir) {
        perror(dir_path);
        return -1;
    }

    int loaded = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!has_so_extension(entry->d_name)) continue;

        char path[MAX_PATH_LEN];
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);
        if (load_one(path) == 0) loaded++;
    }
    closedir(dir);
    return loaded;
}

void unload_plugins(void) {
    clear_operations();
    for (int i = 0; i < handles_count; i++) {
        dlclose(handles[i]);
    }
    free(handles);
    handles = NULL;
    handles_count = 0;
    handles_capacity = 0;
}