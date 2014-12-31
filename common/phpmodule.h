#ifndef __OMNI_PHPMODULE_H
#define __OMNI_PHPMODULE_H

#include "php.h"
#include "ext/standard/info.h"

#define PHP_MODULE_BEGIN() \
static int _tracing_enabled = 0; \

#define PHP_MODULE(name, functions) \
\
extern zend_module_entry name ## _module_entry; \
ZEND_GET_MODULE(name) \
\
PHP_FUNCTION(name ## _enable_tracing) { \
  _tracing_enabled = 1; \
  RETURN_NULL(); \
} \
\
static zend_function_entry name ## _functions[] = { \
  functions \
  PHP_FE(name ## _enable_tracing, NULL) \
  {NULL, NULL, NULL} \
}; \
\
zend_module_entry name ## _module_entry = { \
    STANDARD_MODULE_HEADER, \
    #name, \
    name ## _functions, \
    NULL, \
    NULL, \
    NULL, \
    NULL, \
    NULL, \
    NO_VERSION_YET, \
    STANDARD_MODULE_PROPERTIES \
};

#endif

#include <unistd.h>

//#define TRACE_FUNCTION_CALL() printf("%d: native call: %s\n", getpid(), __func__);
#define TRACE_CUSTOM(msg, ...) { if (_tracing_enabled) { fprintf(stderr, "%d: native call: " #msg "\n", getpid(), __VA_ARGS__); } }
#define TRACE_FUNCTION_CALL()