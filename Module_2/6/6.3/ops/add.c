#include "plugin_api.h"

static double op_add(double a, double b) { return a + b; }

static const OperationInfo info = { '+', 1, op_add };

const OperationInfo* get_operation(void) { return &info; }