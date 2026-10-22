#pragma once
#include <crails/cli/command.hpp>

namespace Wordpress
{
  // Searches the plugins of wordpress.org, and prints the result as JSON (see
  // Org::normalize_search for its shape). Exit status 4 if wordpress.org could
  // not be reached, or did not answer with a list of plugins.
  class SearchCommand : public Crails::Command
  {
  public:
    std::string_view description() const override
    {
      return "searches the plugins of wordpress.org and prints the results as JSON";
    }

    void options_description(boost::program_options::options_description& options) const override
    {
      using namespace std;
      options.add_options()
        ("query,q", boost::program_options::value<string>(), "search terms")
        ("page,p", boost::program_options::value<unsigned int>()->default_value(1), "page of results")
        ("per-page,s", boost::program_options::value<unsigned int>()->default_value(12), "results per page (1 to 30)");
    }

    int run() override;
  };
}
