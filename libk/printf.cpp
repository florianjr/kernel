#include <kernel/drivers/terminal.h>
#include <libk/printf.h>

static void vsprintf_helper(char *str, void (*putchar)(char),
                            const char *format, uint32_t *pos, va_list arg) {
  char c;
  int sign, ival;
  char buf[512];
  uint32_t uval;
  uint32_t size = 8;
  uint32_t len;
  memset(buf, 0, 512);

  while ((c = *format++) != 0) {
    sign = 0;

    if (c == '%') {
      c = *format++;
      switch (c) {
      case 'd':
      case 'u':
      case 'x':
      case 'p':
        if (c == 'u') {
          uval = ival = va_arg(arg, uint64_t);
          u64toa(uval, buf);
        } else {
          uval = ival = va_arg(arg, int);
          if (c == 'd' && ival < 0) {
            sign = 1;
            uval = -ival;
          }
          itoa(uval, buf);
        }
        len = strlen(buf);
        // If use did not specify width, then just use len = width
        size = len;
        if ((c == 'x' || c == 'p' || c == 'd') && len < size) {
          for (auto i = 0; i < len; i++) {
            buf[size - 1 - i] = buf[len - 1 - i];
          }
          for (auto i = 0; i < size - len; i++) {
            buf[i] = '0';
          }
        }
        if (c == 'd' && sign) {
          if (str) {
            *(str + *pos) = '-';
            *pos = *pos + 1;
          } else
            (*putchar)('-');
        }
        if (str) {
          strcpy(str + *pos, buf);
          *pos = *pos + strlen(buf);
        } else {
          char *t = buf;
          while (*t) {
            putchar(*t);
            t++;
          }
        }
        break;
      case 'c':
        if (str) {
          *(str + *pos) = (char)va_arg(arg, int);
          *pos = *pos + 1;
        } else {
          (*putchar)((char)va_arg(arg, int));
        }
        break;
      case 's':
        if (str) {
          char *t = (char *)va_arg(arg, int);
          strcpy(str + (*pos), t);
          *pos = *pos + strlen(t);
        } else {
          char *t = (char *)va_arg(arg, int);
          while (*t) {
            putchar(*t);
            t++;
          }
        }
        break;
      default:
        break;
      }
      continue;
    }
    if (str) {
      *(str + *pos) = c;
      *pos = *pos + 1;
    } else {
      (*putchar)(c);
    }
  }
}

/*
 * Both printf and sprintf call this function to do the actual formatting
 * The only difference of printf and sprintf is, one writes to screen memory,
 * and another writes to normal memory buffer vsprintf should keeps track of
 * current mem pointer to place next character(for printf, print_char alread
 * keeps track of current screen posistion, so this is only true for sprintf)
 * */
void vsprintf(char *str, void (*putchar)(char), const char *format,
              va_list arg) {
  uint32_t pos = 0;
  vsprintf_helper(str, putchar, format, &pos, arg);
}

void kprintf(const char *s, ...) {
  va_list ap;
  va_start(ap, s);
  vsprintf(nullptr, Kernel::Drivers::VGATerminal::write, s, ap);
  va_end(ap);
}

size_t snprintf(char *s, size_t, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  vsprintf(s, nullptr, format, ap);
  va_end(ap);

  return strlen(s);
}
