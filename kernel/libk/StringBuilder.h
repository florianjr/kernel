#pragma once

#include "String.h"

class StringBuilder {
public:
  StringBuilder();
  ~StringBuilder();

  void append(char);
  void append(const char *);
  void append(String);
  void append(size_t);

  String to_string() const;
  size_t length() const;

  void unsafe_set_length(size_t);

private:
  char *m_buffer;
  size_t m_size;
};