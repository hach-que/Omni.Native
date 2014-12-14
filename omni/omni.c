#include "php.h"
#include "ext/standard/info.h"
#include <unistd.h>
#include <errno.h>
#include <phpmodule.h>

PHP_MODULE_BEGIN()

PHP_FUNCTION(omni_execvp) {
  zval *array;
  HashTable *arr_hash;
  int argv_count;
  char** argv;
  int i, a, vlen;
  zval** zv_dest;
  
  TRACE_FUNCTION_CALL();
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &array) == FAILURE) {
    RETURN_FALSE;
  }
  
  arr_hash = Z_ARRVAL_P(array);
  argv_count = zend_hash_num_elements(arr_hash);
  
  argv = emalloc(argv_count + 1);
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

PHP_MODULE(omni,
  PHP_FE(omni_execvp, NULL)
  PHP_FE(omni_pipe, NULL)
)

