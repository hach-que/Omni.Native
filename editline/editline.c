#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <readline/readline.h>
#include <histedit.h>
#include <phpmodule.h>

PHP_MODULE_BEGIN()

static History *h = NULL;
static EditLine* e = NULL;
static char* prompt = NULL;

static char* _get_prompt(EditLine* e) {
  return prompt;
}

PHP_FUNCTION(editline_begin) {
  char* prompt_set;
  int prompt_len;
  HistEvent ev;
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &prompt_set, &prompt_len) == FAILURE) {
    RETURN_FALSE;
  }
  
  e = el_init("EditLine", stdin, stdout, stderr);
  h = history_init();
  
  if (!e || !h) {
    RETURN_LONG(-1);
  }
  
  _set_prompt(prompt_set, prompt_len);
  
  history(h, &ev, H_SETSIZE, INT_MAX);
  el_set(e, EL_HIST, history, h);
  el_set(e, EL_PROMPT, _get_prompt);
  //el_set(e, EL_EDITOR, "vi");
  
  el_source(e, NULL);
  
  el_set(e, EL_UNBUFFERED, 1);
  
  RETURN_TRUE;
}

PHP_FUNCTION(editline_set_prompt) {
  char* prompt_set;
  int prompt_len;
  
  TRACE_FUNCTION_CALL();
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &prompt_set, &prompt_len) == FAILURE) {
    RETURN_FALSE;
  }
  
  _set_prompt(prompt_set, prompt_len);
  
  RETURN_TRUE;
}

void _set_prompt(char* prompt_set, int prompt_len) {
  int i, is_zero;
  
  prompt_len++;
  
  if (prompt != NULL) {
    efree(prompt);
  }
  prompt = emalloc(prompt_len);
  is_zero = 0;
  for (i = 0; i < prompt_len; i++) {
    if (prompt_set[i] == '\0' || i == prompt_len - 1) {
      is_zero = 1;
    }
    if (is_zero) {
      prompt[i] = '\0';
    } else {
      prompt[i] = prompt_set[i];
    }
  }
}

PHP_FUNCTION(editline_read) {
  int count = 0, done = 0;
  const char* buf = el_gets(e, &count);
  
  if (buf == NULL || count-- <= 0) {
    return;
  }
  
  if (count == 0 && buf[0] == CTRL('d')) {
    done = 1;
  }
  
  if (buf[count] == '\n' || buf[count] == '\r') {
    done = 2;
  }
  
  array_init(return_value);
  switch (done) {
    case 0:
      add_assoc_string_ex(return_value, "status", 7, "typing", 1);
      break;
    case 1:
      add_assoc_string_ex(return_value, "status", 7, "cancelled", 1);
      break;
    case 2:
      add_assoc_string_ex(return_value, "status", 7, "complete", 1);
      break;
  }
  add_assoc_long_ex(return_value, "cursor", 7, el_cursor(e, 0));
  add_assoc_stringl_ex(return_value, "input", 6, buf, count + 1, 1);
}

PHP_FUNCTION(editline_end) {
  el_set(e, EL_UNBUFFERED, 0);
  el_end(e);
}

PHP_MODULE(editline, 
  PHP_FE(editline_begin, NULL)
  PHP_FE(editline_set_prompt, NULL)
  PHP_FE(editline_read, NULL)
  PHP_FE(editline_end, NULL)
)
