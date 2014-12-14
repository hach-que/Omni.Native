#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <readline/readline.h>
#include <histedit.h>
#include <phpmodule.h>

PHP_MODULE_BEGIN()

static History *h = NULL;
static EditLine* e = NULL;

static char* _get_prompt(EditLine* e) {
  return "#> ";
}

PHP_FUNCTION(editline_begin) {
  HistEvent ev;
  
  e = el_init("EditLine", stdin, stdout, stderr);
  h = history_init();
  
  if (!e || !h) {
    RETURN_LONG(-1);
  }
  
  history(h, &ev, H_SETSIZE, INT_MAX);
  el_set(e, EL_HIST, history, h);
  el_set(e, EL_PROMPT, _get_prompt);
  
  el_source(e, NULL);
  
  el_set(e, EL_UNBUFFERED, 1);
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
  add_assoc_stringl_ex(return_value, "input", 6, buf, count + 1, 1);
}

PHP_FUNCTION(editline_end) {
  el_set(e, EL_UNBUFFERED, 0);
}

PHP_MODULE(editline, 
  PHP_FE(editline_begin, NULL)
  PHP_FE(editline_read, NULL)
  PHP_FE(editline_end, NULL)
)
