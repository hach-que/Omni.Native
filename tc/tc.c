#include <termios.h>
#include <phpmodule.h>

PHP_MODULE_BEGIN()

PHP_FUNCTION(tc_tcgetpgrp) {
  long fd;
  
  TRACE_FUNCTION_CALL();
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &fd) == FAILURE) {
    RETURN_FALSE;
  }
  
  RETURN_LONG(tcgetpgrp(fd));
}

PHP_FUNCTION(tc_tcsetpgrp) {
  long fd;
  long pgid;
  
  TRACE_FUNCTION_CALL();
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &fd, &pgid) == FAILURE) {
    RETURN_FALSE;
  }
  
  tcsetpgrp(fd, pgid);
  
  RETURN_TRUE;
}

PHP_FUNCTION(tc_tcgetattr) {
  long fd;
  struct termios tmodes;
  zval *arr_cc;
  int i;
  
  TRACE_FUNCTION_CALL();
  
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

#define LOAD_FROM_HASH_TABLE_OR_ERROR(hash, name) \
  err = zend_hash_exists(hash, name, strlen(name)); \
  if (err != SUCCESS) { \
    RETURN_FALSE; \
  } \
  err = zend_hash_find(hash, name, strlen(name) + 1, (void**)&temp); \
  if (err != SUCCESS) { \
    RETURN_FALSE; \
  }
  
#define GET_LONG_FROM_HASH_TABLE_OR_ERROR(hash, name, target) \
  LOAD_FROM_HASH_TABLE_OR_ERROR(hash, name); \
  convert_to_long_ex(temp); \
  target = Z_LVAL_PP(temp);

PHP_FUNCTION(tc_tcsetattr) {
  long fd;
  long mode;
  zval *array;
  struct termios tmodes;
  HashTable* arr_hash;
  int arr_count; 
  zval* cc_array;
  HashTable* cc_hash;
  int err;
  zval** temp;
  int i;
  
  TRACE_FUNCTION_CALL();
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lla", &fd, &mode, &array) == FAILURE) {
    RETURN_FALSE;
  }
  
  arr_hash = Z_ARRVAL_P(array);
  arr_count = zend_hash_num_elements(arr_hash);
  
  GET_LONG_FROM_HASH_TABLE_OR_ERROR(arr_hash, "iflag", tmodes.c_iflag);
  GET_LONG_FROM_HASH_TABLE_OR_ERROR(arr_hash, "oflag", tmodes.c_oflag);
  GET_LONG_FROM_HASH_TABLE_OR_ERROR(arr_hash, "cflag", tmodes.c_cflag);
  GET_LONG_FROM_HASH_TABLE_OR_ERROR(arr_hash, "lflag", tmodes.c_lflag);
  GET_LONG_FROM_HASH_TABLE_OR_ERROR(arr_hash, "ispeed", tmodes.c_ispeed);
  GET_LONG_FROM_HASH_TABLE_OR_ERROR(arr_hash, "ospeed", tmodes.c_ospeed);
  
  LOAD_FROM_HASH_TABLE_OR_ERROR(arr_hash, "cc");
  
  cc_hash = Z_ARRVAL_PP(temp);
  for (i = 0; i < NCCS; i++) {
    if (zend_hash_index_find(cc_hash, i, (void**)&temp) == SUCCESS) {
      convert_to_long_ex(temp);
      tmodes.c_cc[i] = Z_LVAL_PP(temp);
    }
  }
  
  tcsetattr(fd, mode, &tmodes);
  
  RETURN_TRUE;
}

PHP_FUNCTION(tc_tcsadrain) {
  RETURN_LONG(TCSADRAIN);
}

PHP_MODULE(tc,
  PHP_FE(tc_tcgetpgrp, NULL)
  PHP_FE(tc_tcsetpgrp, NULL)
  PHP_FE(tc_tcgetattr, NULL)
  PHP_FE(tc_tcsetattr, NULL)
  PHP_FE(tc_tcsadrain, NULL)
)