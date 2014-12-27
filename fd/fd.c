#include "php.h"
#include "ext/standard/info.h"
#include <stdio.h>
#include <phpmodule.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <errno.h>
#include <stddef.h>
#include <ancillary.h>
#include <poll.h>

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
  
  int i, a, t;
  zval** elem;
  
  fd_set read_fds;
  fd_set write_fds;
  fd_set except_fds;
  struct timeval timeout;
  int ready, value;
  
  struct pollfd* polls;
  
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
  t = 0;
  for (
    zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(read_array), &pointer);
    zend_hash_get_current_data_ex(Z_ARRVAL_P(read_array), (void**)&elem, &pointer) == SUCCESS;
    zend_hash_move_forward_ex(Z_ARRVAL_P(read_array), &pointer)) {
    
    read_fd_list[a++] = Z_LVAL_PP(elem);
    t++;
  }
  
  write_fd_count = zend_hash_num_elements(Z_ARRVAL_P(write_array));
  write_fd_list = emalloc(sizeof(int) * write_fd_count);
  a = 0;
  for (
    zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(write_array), &pointer);
    zend_hash_get_current_data_ex(Z_ARRVAL_P(write_array), (void**)&elem, &pointer) == SUCCESS;
    zend_hash_move_forward_ex(Z_ARRVAL_P(write_array), &pointer)) {
    
    write_fd_list[a++] = Z_LVAL_PP(elem);
    t++;
  }
  
  except_fd_count = zend_hash_num_elements(Z_ARRVAL_P(except_array));
  except_fd_list = emalloc(sizeof(int) * except_fd_count);
  a = 0;
  for (
    zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(except_array), &pointer);
    zend_hash_get_current_data_ex(Z_ARRVAL_P(except_array), (void**)&elem, &pointer) == SUCCESS;
    zend_hash_move_forward_ex(Z_ARRVAL_P(except_array), &pointer)) {
    
    except_fd_list[a++] = Z_LVAL_PP(elem);
    t++;
  }
  
  /*FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  FD_ZERO(&except_fds);
  
  for (i = 0; i < read_fd_count; i++) { FD_SET(read_fd_list[i], &read_fds); }
  for (i = 0; i < write_fd_count; i++) { FD_SET(write_fd_list[i], &write_fds); }
  for (i = 0; i < except_fd_count; i++) { FD_SET(except_fd_list[i], &except_fds); }
  
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;*/
  
  // Use poll() first; this will tell us about closed pipes in the write FDs
  // list (which select won't).
  polls = malloc(sizeof(struct pollfd) * t);
  for (i = 0; i < read_fd_count; i++) {
    polls[i].fd = read_fd_list[i];
    polls[i].events = POLLIN | POLLERR | POLLHUP | POLLRDHUP | POLLNVAL;
  }
  for (i = 0; i < write_fd_count; i++) {
    polls[i + read_fd_count].fd = write_fd_list[i];
    polls[i + read_fd_count].events = POLLERR | POLLHUP | POLLRDHUP | POLLNVAL;
  }
  for (i = 0; i < except_fd_count; i++) {
    polls[i + read_fd_count + write_fd_count].fd = except_fd_list[i];
    polls[i + read_fd_count + write_fd_count].events = POLLERR | POLLHUP | POLLRDHUP | POLLNVAL;
  }
  
  ready = TEMP_FAILURE_RETRY(
    poll(
      polls,
      t,
      -1));
  
  array_init(return_value);
  add_assoc_long(return_value, "ready", ready);
  
  ALLOC_INIT_ZVAL(read_result_array);
  array_init(read_result_array);
  for (i = 0; i < read_fd_count; i++) {
    value = polls[i].revents;
    if (value & POLLIN) {
      add_index_string(read_result_array, read_fd_list[i], "ready", 1);
    } else if (value & POLLERR) {
      add_index_string(read_result_array, read_fd_list[i], "error", 1);
    } else if (value & POLLHUP) {
      add_index_string(read_result_array, read_fd_list[i], "error", 1);
    } else if (value & POLLRDHUP) {
      add_index_string(read_result_array, read_fd_list[i], "error", 1);
    } else if (value & POLLNVAL) {
      add_index_string(read_result_array, read_fd_list[i], "invalid", 1);
    } else if (value != 0) {
      add_index_string(read_result_array, read_fd_list[i], "unknown", 1);
    } else {
      add_index_bool(read_result_array, read_fd_list[i], 0);
    }
  }
  add_assoc_zval(return_value, "read", read_result_array);
  
  ALLOC_INIT_ZVAL(write_result_array);
  array_init(write_result_array);
  for (i = 0; i < write_fd_count; i++) {
    value = polls[i + read_fd_count].revents;
    if (value & POLLERR) {
      add_index_string(write_result_array, write_fd_list[i], "error", 1);
    } else if (value & POLLHUP) {
      add_index_string(write_result_array, write_fd_list[i], "error", 1);
    } else if (value & POLLRDHUP) {
      add_index_string(write_result_array, write_fd_list[i], "error", 1);
    } else if (value & POLLNVAL) {
      add_index_string(write_result_array, write_fd_list[i], "invalid", 1);
    } else if (value != 0) {
      add_index_string(write_result_array, write_fd_list[i], "unknown", 1);
    } else {
      add_index_bool(write_result_array, write_fd_list[i], 0);
    }
  }
  add_assoc_zval(return_value, "write", write_result_array);
  
  ALLOC_INIT_ZVAL(except_result_array);
  array_init(except_result_array);
  for (i = 0; i < except_fd_count; i++) { 
    value = polls[i + read_fd_count + write_fd_count].revents;
    if (value & POLLERR) {
      add_index_string(except_result_array, except_fd_list[i], "error", 1);
    } else if (value & POLLHUP) {
      add_index_string(except_result_array, except_fd_list[i], "error", 1);
    } else if (value & POLLRDHUP) {
      add_index_string(except_result_array, except_fd_list[i], "error", 1);
    } else if (value & POLLNVAL) {
      add_index_string(except_result_array, except_fd_list[i], "invalid", 1);
    } else if (value != 0) {
      add_index_string(except_result_array, except_fd_list[i], "unknown", 1);
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
  
  TRACE_CUSTOM("created native pipe (write) %d -> %d (read)", endpoint[1], endpoint[0]);
  
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

PHP_FUNCTION(fd_control_pipe) {
  int endpoint[2];
  
  TRACE_FUNCTION_CALL();
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    RETURN_FALSE;
  }
  
  if (socketpair(PF_UNIX, SOCK_STREAM, 0, endpoint) < 0) {
    RETURN_FALSE;
  }
  
  TRACE_CUSTOM("created socket pair (write) %d -> %d (read)", endpoint[1], endpoint[0]);
  
  array_init(return_value);
  add_assoc_long(return_value, "read", endpoint[0]);
  add_assoc_long(return_value, "write", endpoint[1]);
}

PHP_FUNCTION(fd_control_writefd) {
  long pipe_fd;
  long fd_to_send;
  
  TRACE_FUNCTION_CALL();
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &pipe_fd, &fd_to_send) == FAILURE) {
    RETURN_FALSE;
  }
  
  TRACE_CUSTOM("writing FD %d using socket FD %d", fd_to_send, pipe_fd);
  
  if (ancil_send_fd(pipe_fd, fd_to_send) < 0) {
    RETURN_FALSE;
  } else {
    RETURN_TRUE;
  }
}

PHP_FUNCTION(fd_control_readfd) {
  long pipe_fd;
  int fd_to_recv;
  
  TRACE_FUNCTION_CALL();
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &pipe_fd) == FAILURE) {
    RETURN_FALSE;
  }
  
  if (ancil_recv_fd(pipe_fd, &fd_to_recv) < 0) {
    TRACE_CUSTOM("failed to read FD from %d", pipe_fd);
    RETURN_FALSE;
  } else {
    TRACE_CUSTOM("read FD %d from socket FD %d", fd_to_recv, pipe_fd);
    RETURN_LONG(fd_to_recv);
  }
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
  PHP_FE(fd_control_pipe, NULL)
  PHP_FE(fd_control_writefd, NULL)
  PHP_FE(fd_control_readfd, NULL)
)