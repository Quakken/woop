#include "log.hpp"

int main() {
  woop::log("This is a trace message!");
  woop::log_standard("This is a standard message!");
  woop::log_warning("This is a warning message!");
  woop::log_error("This is an error message!");
  woop::log_fatal("This is a fatal messgae!");
}