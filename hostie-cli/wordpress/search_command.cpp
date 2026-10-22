#include "search_command.hpp"
#include "wordpress_org_client.hpp"
#include <algorithm>
#include <iostream>

using namespace std;
using namespace Wordpress;

int SearchCommand::run()
{
  if (!options.count("query"))
  {
    cerr << "Missing option query" << endl;
    return 2;
  }

  const string query = options["query"].as<string>();
  const unsigned int page = clamp<unsigned int>(options["page"].as<unsigned int>(), 1, 1000);
  const unsigned int per_page = clamp<unsigned int>(options["per-page"].as<unsigned int>(), 1, 30);

  if (auto result = Org::search_plugins(query, page, per_page))
  {
    cout << *result << endl;
    return 0;
  }
  cerr << "could not search the plugins of wordpress.org" << endl;
  return 4;
}
