#include <math.h>
#include "plugin_api.h"

static double op_pow(double a, double b) { return pow(a, b); }

static const OperationInfo info = { '^', 3, op_pow };

const OperationInfo* get_operation(void) { return &info; }