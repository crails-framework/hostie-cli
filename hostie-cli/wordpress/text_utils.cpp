#include "text_utils.hpp"
#include <cstdlib>
#include <cstring>

using namespace std;

namespace
{
  void append_utf8(string& out, unsigned long code)
  {
    if (code == 0 || code > 0x10FFFF || (code >= 0xD800 && code < 0xE000))
      code = 0xFFFD;
    if (code < 0x80)
      out += static_cast<char>(code);
    else if (code < 0x800)
    {
      out += static_cast<char>(0xC0 | (code >> 6));
      out += static_cast<char>(0x80 | (code & 0x3F));
    }
    else if (code < 0x10000)
    {
      out += static_cast<char>(0xE0 | (code >> 12));
      out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
      out += static_cast<char>(0x80 | (code & 0x3F));
    }
    else
    {
      out += static_cast<char>(0xF0 | (code >> 18));
      out += static_cast<char>(0x80 | ((code >> 12) & 0x3F));
      out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
      out += static_cast<char>(0x80 | (code & 0x3F));
    }
  }

  struct NamedEntity { const char* name; unsigned long code; };

  const NamedEntity named_entities[] = {
    {"amp", '&'}, {"lt", '<'}, {"gt", '>'}, {"quot", '"'}, {"apos", '\''},
    {"nbsp", ' '}, {"ndash", 0x2013}, {"mdash", 0x2014}, {"hellip", 0x2026},
    {"lsquo", 0x2018}, {"rsquo", 0x2019}, {"ldquo", 0x201C}, {"rdquo", 0x201D}
  };

  // `entity` is what is between "&" and ";". Returns false if it is unknown.
  bool decode_entity(string_view entity, string& out)
  {
    if (entity.size() >= 2 && entity[0] == '#')
    {
      const bool hex = entity[1] == 'x' || entity[1] == 'X';
      const string digits(entity.substr(hex ? 2 : 1));

      if (digits.empty() || digits.length() > 8 || digits.find_first_not_of(hex ? "0123456789abcdefABCDEF" : "0123456789") != string::npos)
        return false;
      append_utf8(out, strtoul(digits.c_str(), nullptr, hex ? 16 : 10));
      return true;
    }
    for (const NamedEntity& named : named_entities)
    {
      if (entity == named.name)
      {
        append_utf8(out, named.code);
        return true;
      }
    }
    return false;
  }
}

string Wordpress::url_encode(string_view value)
{
  static const char* hex = "0123456789ABCDEF";
  string result;

  for (char raw : value)
  {
    const unsigned char c = static_cast<unsigned char>(raw);

    if (isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~')
      result += raw;
    else
    {
      result += '%';
      result += hex[c >> 4];
      result += hex[c & 0xF];
    }
  }
  return result;
}

string Wordpress::decode_html_entities(string_view value)
{
  string result;

  for (size_t i = 0 ; i < value.size() ; ++i)
  {
    if (value[i] == '&')
    {
      const size_t end = value.find(';', i + 1);

      // The longest entity worth looking at is a few characters long
      if (end != string_view::npos && end - i <= 12 && decode_entity(value.substr(i + 1, end - i - 1), result))
      {
        i = end;
        continue ;
      }
    }
    result += value[i];
  }
  return result;
}
