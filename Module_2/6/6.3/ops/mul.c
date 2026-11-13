#include "plugin_api.h"

static double op_mul(double a, double b) { return a * b; }

static const OperationInfo info = { '*', 2, op_mul };

const OperationInfo* get_operation(void) { return &info; }