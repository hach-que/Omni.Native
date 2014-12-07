#include "php.h"
#include "ext/standard/info.h"
#include <unistd.h>
#include <termios.h>
#include <errno.h>

extern zend_module_entry omni_module_entry;
#define phpext_omni_ptr &omni_module_entry

#ifdef PHP_WIN32
#define PHP_OMNI_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#define PHP_OMNI_API __attribute__((visibility("default")))
#else
#define PHP_OMNI_API
#endif

ZEND_GET_MODULE(omni)

PHP_FUNCTION(omni_tcgetpgrp) {
  long fd;
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &fd) == FAILURE) {
    RETURN_FALSE;
  }
  
  RETURN_LONG(tcgetpgrp(fd));
}

PHP_FUNCTION(omni_tcsetpgrp) {
  long fd;
  long pgid;
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &fd, &pgid) == FAILURE) {
    RETURN_FALSE;
  }
  
  tcsetpgrp(fd, pgid);
  
  RETURN_TRUE;
}

PHP_FUNCTION(omni_tcgetattr) {
  long fd;
  struct termios tmodes;
  zval *arr_cc;
  int i;
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &fd) == FAILURE) {
    RETURN_FALSE;
  }
  
  tcgetattr(fd, &tmodes);
  
  array_init(return_value);
  add_assoc_long(return_value, "iflag", tmodes.c_iflag);
  add_assoc_long(return_value, "oflag", tmodes.c_oflag);
  add_assoc_long(return_value, "cflag", tmodes.c_cflag);
  add_assoc_long(return_value, "lflag", tmodes.c_lflag);
  add_assoc_long(return_value, "ispeed", tmodes.c_ispeed);
  add_assoc_long(return_value, "ospeed", tmodes.c_ospeed);
  
  ALLOC_INIT_ZVAL(arr_cc);
  array_init(arr_cc);
  for (i = 0; i < NCCS; i++) {
    add_index_long(arr_cc, i, tmodes.c_cc[i]);
  }
  add_assoc_zval(return_value, "cc", arr_cc);
}

PHP_FUNCTION(omni_tcsetattr_tcsadrain) {
  long fd;
  zval *array;
  struct termios tmodes;
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "la", &fd, &array) == FAILURE) {
    RETURN_FALSE;
  }
  
  // TODO Read settings from array.
  //tcsetattr(fd, TCSADRAIN, &tmodes);
  
  RETURN_TRUE;
}

PHP_FUNCTION(omni_dup2) {
  long oldFD;
  long newFD;
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &oldFD, &newFD) == FAILURE) {
    RETURN_FALSE;
  }
  
  RETURN_LONG(dup2(oldFD, newFD));
}

PHP_FUNCTION(omni_close) {
  long fd;
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &fd) == FAILURE) {
    RETURN_FALSE;
  }
  
  RETURN_LONG(close(fd));
}

PHP_FUNCTION(omni_execvp) {
  zval *array;
  HashTable *arr_hash;
  HashPointer pointer;
  int argv_count;
  char** argv;
  int i, a, vlen;
  zval** zv_dest;
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &array) == FAILURE) {
    RETURN_FALSE;
  }
  
  arr_hash = Z_ARRVAL_P(array);
  argv_count = zend_hash_num_elements(arr_hash);
  
  argv = malloc(argv_count + 1);
  for (i = 0; i < argv_count; i++) {
    if (zend_hash_index_find(arr_hash, i, (void**)&zv_dest) == SUCCESS) {
      convert_to_string_ex(zv_dest);
      vlen = Z_STRLEN_PP(zv_dest);
      argv[i] = malloc(vlen + 1);
      for (a = 0; a < vlen; a++) {
        argv[i][a] = Z_STRVAL_PP(zv_dest)[a];
      }
      argv[i][vlen] = 0;
    }
  }
  argv[argv_count] = NULL;
  
  if (execvp(argv[0], argv) == -1) {
    RETURN_LONG(errno);
  }
  
  // Will never reach here.
  RETURN_NULL();
}

PHP_FUNCTION(omni_pipe) {
  int endpoint[2];
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    RETURN_FALSE;
  }
  
  if (pipe(endpoint) < 0) {
    RETURN_FALSE;
  }
  
  array_init(return_value);
  add_assoc_long(return_value, "read", endpoint[0]);
  add_assoc_long(return_value, "write", endpoint[1]);
}

static zend_function_entry omni_functions[] = {
    PHP_FE(omni_tcgetpgrp, NULL)
    PHP_FE(omni_tcsetpgrp, NULL)
    PHP_FE(omni_tcgetattr, NULL)
    PHP_FE(omni_tcsetattr_tcsadrain, NULL)
    PHP_FE(omni_dup2, NULL)
    PHP_FE(omni_close, NULL)
    PHP_FE(omni_execvp, NULL)
    PHP_FE(omni_pipe, NULL)
    {NULL, NULL, NULL}
};

zend_module_entry omni_module_entry = {
    STANDARD_MODULE_HEADER,
    "omni",
    omni_functions,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NO_VERSION_YET,
    STANDARD_MODULE_PROPERTIES
};

