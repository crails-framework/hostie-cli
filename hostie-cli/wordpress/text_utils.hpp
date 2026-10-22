#pragma once
#include <string>
#include <string_view>

namespace Wordpress
{
  // Percent-encodes everything but unreserved characters (RFC 3986): for a
  // value in a query string.
  std::string url_encode(std::string_view);

  // wordpress.org sends names and descriptions as HTML text ("Foo &#8211; Bar").
  // This turns the common entities back into characters (as UTF-8). Markup is
  // not interpreted, and unknown entities are left alone.
  std::string decode_html_entities(std::string_view);
}
