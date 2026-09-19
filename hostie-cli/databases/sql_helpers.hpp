#pragma once
#include <charconv>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

// Quotes a value as an SQL string literal.
// MySQL treats backslashes as escape characters by default, PostgreSQL doesn't.
inline std::string sql_string_literal(std::string_view value, bool backslash_escapes)
{
  std::string result("'");

  for (char c : value)
  {
    if (c == '\'' || (backslash_escapes && c == '\\'))
      result += c;
    result += c;
  }
  result += '\'';
  return result;
}

// Reads the last line of a command's output as an unsigned integer.
// (`mysql -e` prints a header line before the actual result)
inline std::optional<std::uint64_t> parse_trailing_integer(std::string_view output)
{
  std::uint64_t value = 0;

  while (!output.empty() && (output.back() == '\n' || output.back() == '\r' || output.back() == ' '))
    output.remove_suffix(1);
  if (auto newline = output.find_last_of('\n'); newline != std::string_view::npos)
    output.remove_prefix(newline + 1);
  auto [end, error] = std::from_chars(output.data(), output.data() + output.size(), value);
  if (error != std::errc() || end != output.data() + output.size())
    return std::nullopt;
  return value;
}
