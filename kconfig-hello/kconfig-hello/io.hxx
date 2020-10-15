#pragma once

#include <string>

// Input/output abstraction layer.
//
void
print_out (const std::string&);

void
print_err (const std::string&);

std::string
ask (const std::string& prompt);
