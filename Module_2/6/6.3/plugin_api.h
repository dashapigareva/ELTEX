#ifndef PLUGIN_API_H
#define PLUGIN_API_H

typedef double (*BinaryOpFunc)(double, double);
typedef struct {
    char symbol;
    int priority;       
    BinaryOpFunc func;
} OperationInfo;
#define PLUGIN_ENTRY_NAME "get_operation"
typedef const OperationInfo* (*GetOperationFunc)(void);

#endif