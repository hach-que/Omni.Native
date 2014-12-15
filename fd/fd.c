#include "php.h"
#include "ext/standard/info.h"
#include <stdio.h>
#include <phpmodule.h>
#include <fcntl.h>

// Get the original functionality back after PHP overrides it.
#ifdef snprintf
#undef snprintf
#endif

PHP_MODULE_BEGIN()

PHP_FUNCTION(fd_select) {
  zval *read_array;
  zval *write_array;
  zval *except_array;
  int *read_fd_list, read_fd_count;
  int *write_fd_list, write_fd_count;
  int *except_fd_list, except_fd_count;
  HashPointer pointer;
  
  int i, a;
  zval** elem;
  
  fd_set read_fds;
  fd_set write_fds;
  fd_set except_fds;
  struct timeval timeout;
  int ready;
  
  zval *read_result_array;
  zval *write_result_array;
  zval *except_result_array;
  
  TRACE_FUNCTION_CALL();
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "aaa", &read_array, &write_array, &except_array) == FAILURE) {
    RETURN_FALSE;
  }
  
  read_fd_count = zend_hash_num_elements(Z_ARRVAL_P(read_array));
  read_fd_list = emalloc(sizeof(int) * read_fd_count);
  a = 0;
  for (
    zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(read_array), &pointer);
    zend_hash_get_current_data_ex(Z_ARRVAL_P(read_array), (void**)&elem, &pointer) == SUCCESS;
    zend_hash_move_forward_ex(Z_ARRVAL_P(read_array), &pointer)) {
    
    read_fd_list[a++] = Z_LVAL_PP(elem);
  }
  
  write_fd_count = zend_hash_num_elements(Z_ARRVAL_P(write_array));
  write_fd_list = emalloc(sizeof(int) * write_fd_count);
  a = 0;
  for (
    zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(write_array), &pointer);
    zend_hash_get_current_data_ex(Z_ARRVAL_P(write_array), (void**)&elem, &pointer) == SUCCESS;
    zend_hash_move_forward_ex(Z_ARRVAL_P(write_array), &pointer)) {
    
    write_fd_list[a++] = Z_LVAL_PP(elem);
  }
  
  except_fd_count = zend_hash_num_elements(Z_ARRVAL_P(except_array));
  except_fd_list = emalloc(sizeof(int) * except_fd_count);
  a = 0;
  for (
    zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(except_array), &pointer);
    zend_hash_get_current_data_ex(Z_ARRVAL_P(except_array), (void**)&elem, &pointer) == SUCCESS;
    zend_hash_move_forward_ex(Z_ARRVAL_P(except_array), &pointer)) {
    
    except_fd_list[a++] = Z_LVAL_PP(elem);
  }
  
  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  FD_ZERO(&except_fds);
  
  for (i = 0; i < read_fd_count; i++) { FD_SET(read_fd_list[i], &read_fds); }
  for (i = 0; i < write_fd_count; i++) { FD_SET(write_fd_list[i], &write_fds); }
  for (i = 0; i < except_fd_count; i++) { FD_SET(except_fd_list[i], &except_fds); }
  
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  
  ready = TEMP_FAILURE_RETRY(
    select(
      FD_SETSIZE,
      &read_fds,
      &write_fds,
      &except_fds,
      NULL));
  
  array_init(return_value);
  add_assoc_long(return_value, "ready", ready);
  
  ALLOC_INIT_ZVAL(read_result_array);
  array_init(read_result_array);
  for (i = 0; i < read_fd_count; i++) { 
    if (FD_ISSET(read_fd_list[i], &read_fds)) {
      add_index_bool(read_result_array, read_fd_list[i], 1);
    } else {
      add_index_bool(read_result_array, read_fd_list[i], 0);
    }
  }
  add_assoc_zval(return_value, "read", read_result_array);
  
  ALLOC_INIT_ZVAL(write_result_array);
  array_init(write_result_array);
  for (i = 0; i < write_fd_count; i++) { 
    if (FD_ISSET(read_fd_list[i], &write_fds)) {
      add_index_bool(write_result_array, write_fd_list[i], 1);
    } else {
      add_index_bool(write_result_array, write_fd_list[i], 0);
    }
  }
  add_assoc_zval(return_value, "write", write_result_array);
  
  ALLOC_INIT_ZVAL(except_result_array);
  array_init(except_result_array);
  for (i = 0; i < except_fd_count; i++) { 
    if (FD_ISSET(read_fd_list[i], &except_fds)) {
      add_index_bool(except_result_array, except_fd_list[i], 1);
    } else {
      add_index_bool(except_result_array, except_fd_list[i], 0);
    }
  }
  add_assoc_zval(return_value, "except", except_result_array);

  /* We wrote the array directly into return_value */
}

PHP_FUNCTION(fd_read) {
  long fd;
  long length;
  void* buffer;
  ssize_t result;
  
  TRACE_FUNCTION_CALL();
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &fd, &length) == FAILURE) {
    RETURN_FALSE;
  }
  
  TRACE_CUSTOM("reading from file descriptor %d", fd);
  
  buffer = emalloc(length);
  result = read(fd, buffer, length);
  
  if (result == -1) {
    // Error
    if (errno == EAGAIN) {
      RETVAL_TRUE;
    } else {
      RETVAL_FALSE;
    }
  } else if (result == 0) {
    // EOF
    RETVAL_NULL();
  } else {
    // Data received
    RETVAL_STRINGL(buffer, result, 1);
  }
  
  efree(buffer);
}

PHP_FUNCTION(fd_write) {
  long fd;
  char* buffer;
  int buffer_len;
  int result;
  
  TRACE_FUNCTION_CALL();
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &fd, &buffer, &buffer_len) == FAILURE) {
    RETURN_FALSE;
  }
  
  TRACE_CUSTOM("writing to file descriptor %d", fd);
  
  result = write(fd, buffer, buffer_len);
  
  if (result == -1) {
    // Error
    if (errno == EAGAIN) {
      RETURN_TRUE;
    } else if (errno == EPIPE) {
      RETURN_NULL();
    } else {
      RETURN_FALSE;
    }
  } else {
    RETURN_LONG(result);
  }
}

PHP_FUNCTION(fd_close) {
  long fd;
  
  TRACE_FUNCTION_CALL();
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &fd) == FAILURE) {
    RETURN_FALSE;
  }
  
  TRACE_CUSTOM("closing file descriptor %d", fd);
  
  RETURN_LONG(close(fd));
}

PHP_FUNCTION(fd_dup2) {
  long oldFD;
  long newFD;
  
  TRACE_FUNCTION_CALL();
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &oldFD, &newFD) == FAILURE) {
    RETURN_FALSE;
  }
  
  TRACE_CUSTOM("duplicating file descriptor %d to %d", oldFD, newFD);
  
  RETURN_LONG(dup2(oldFD, newFD));
}

PHP_FUNCTION(fd_pipe) {
  int endpoint[2];
  
  TRACE_FUNCTION_CALL();
  
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

PHP_FUNCTION(fd_set_blocking) {
  long fd;
  int flags;
  int blocking;
  
  TRACE_FUNCTION_CALL();
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lb", &fd, &blocking) == FAILURE) {
    RETURN_FALSE;
  }
  
  flags = fcntl(fd, F_GETFL);
  if (blocking == 0) {
    TRACE_CUSTOM("making file descriptor %d non-blocking", fd);
    flags |= O_NONBLOCK;
  } else {
    TRACE_CUSTOM("making file descriptor %d blocking", fd);
    flags &= ~O_NONBLOCK;
  }
  fcntl(fd, F_SETFL, flags);
  
  RETURN_NULL();
}

PHP_FUNCTION(fd_get_error) {
  TRACE_FUNCTION_CALL();
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    RETURN_FALSE;
  }
  
  char* buffer = emalloc(sizeof(char) * 257);
  memset(buffer, 0, 257);
  snprintf(buffer, 256, "%m");
  
  array_init(return_value);
  add_assoc_long(return_value, "errno", errno);
  add_assoc_stringl(return_value, "error", buffer, strlen(buffer), 1);
  efree(buffer);
}

PHP_MODULE(fd, 
  PHP_FE(fd_select, NULL)
  PHP_FE(fd_read, NULL)
  PHP_FE(fd_write, NULL)
  PHP_FE(fd_close, NULL)
  PHP_FE(fd_dup2, NULL)
  PHP_FE(fd_pipe, NULL)
  PHP_FE(fd_set_blocking, NULL)
  PHP_FE(fd_get_error, NULL)
)