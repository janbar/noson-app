#pragma once

#include <cstddef>

#define SAFE_DELETE(p)        if ((p) != NULL) { delete (p);    (p) = NULL; }
#define SAFE_DELETE_ARRAY(p)  if ((p) != NULL) { delete[] (p);  (p) = NULL; }
