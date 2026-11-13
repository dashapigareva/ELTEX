#include "plugin_api.h"

static double op_sub(double a, double b) { return a - b; }

static const OperationInfo info = { '-', 1, op_sub };

const OperationInfo* get_operation(void) { return &info; }