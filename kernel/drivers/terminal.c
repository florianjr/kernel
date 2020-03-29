#include "terminal.h"
#include "../libk/log.h"
#include "../scheduler.h"

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
volatile uint16_t *terminal_buffer;

volatile bool locked = false;

void terminal_lock() {
  while (locked)
    yield();
  locked = true;
}

void terminal_unlock() { locked = false; }

void terminal_setcolor(uint8_t color) { terminal_color = color; }

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
  const size_t index = y * VGA_WIDTH + x;
  terminal_buffer[index] = vga_entry(c, color);
}

void terminal_scroll() {
  if (terminal_row >= VGA_HEIGHT - 1) {
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
      for (size_t x = 0; x < VGA_WIDTH; x++) {
        const size_t index = y * VGA_WIDTH + x;
        const size_t index_old = (y + 1) * VGA_WIDTH + x;
        terminal_buffer[index] = terminal_buffer[index_old];
      }
    }
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      terminal_putentryat(' ', terminal_color, x, VGA_HEIGHT - 1);
    }
    terminal_row--;
  }
}

void terminal_putchar(char c) {
  if (c == '\n') {
    terminal_column = 0;
    terminal_row++;
    terminal_scroll();
    return;
  }

  if (c == '\r') {
    terminal_column = 0;
    return;
  }

  terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
  if (++terminal_column == VGA_WIDTH) {
    terminal_column = 0;
    if (++terminal_row == VGA_HEIGHT)
      terminal_row = VGA_HEIGHT - 1;
  }
}

void terminal_write(const char *data, size_t size) {
  for (size_t i = 0; i < size; i++)
    terminal_putchar(data[i]);
}

void terminal_writestring(const char *data) {
  terminal_write(data, strlen(data));
}

void terminal_clear() {
  terminal_row = 0;
  terminal_column = 0;
  terminal_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
  terminal_buffer = (uint16_t *)0xB8000;
  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t index = y * VGA_WIDTH + x;
      terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
  }
}

void terminal_cursor_disable() {
  outb(0x3D4, 0x0A);
  outb(0x3D5, 0x20);
}

void terminal_move_cursor(int x, int y) {
  uint16_t pos = y * VGA_WIDTH + x;

  outb(0x3D4, 0x0F);
  outb(0x3D5, (uint8_t)(pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static void terminal_task() {
  while (true) {
    Message *m = message_get(&runningTask->port);
    klog(m->message);

    char *cmd = strsep(&m->message, " ");

    if (streq(cmd, "clear")) {
      terminal_clear();
    } else if (streq(cmd, "setpos")) {
      char *xStr = strsep(&m->message, " ");
      char *yStr = strsep(&m->message, " ");

      size_t x = atoi(xStr);
      size_t y = atoi(yStr);

      terminal_row = y;
      terminal_column = x;
    } else if (streq(cmd, "print")) {
      terminal_writestring(m->message);
    } else if (streq(cmd, "cursor")) {
      char *arg = strsep(&m->message, " ");

      if (streq(arg, "disable")) {
        terminal_cursor_disable();
      } else if (streq(arg, "setpos")) {
        char *xStr = strsep(&m->message, " ");
        char *yStr = strsep(&m->message, " ");

        size_t x = atoi(xStr);
        size_t y = atoi(yStr);

        terminal_move_cursor(x, y);
      }
    }

    if (m->response == NULL)
      m->response = "";
    yield();
  }
}

static bool terminal_initialize() {
  terminal_clear();

  spawnTask(terminal_task, "terminal-driver");
  return true;
}

driverDefinition TERMINAL_DRIVER = {.name = "VGA Terminal",
                                    .isAvailable = driver_true,
                                    .initialize = terminal_initialize};
