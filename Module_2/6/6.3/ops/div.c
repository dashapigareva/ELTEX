#include <math.h>
#include "plugin_api.h"

static double op_div(double a, double b) { return (b == 0.0) ? NAN : (a / b); }

static const OperationInfo info = { '/', 2, op_div };

const OperationInfo* get_operation(void) { return &info; }