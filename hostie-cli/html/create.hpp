#pragma once
#include "../standard_creator.hpp"

namespace Html
{
  class CreateCommand : public StandardCreator
  {
  public:
    std::string_view description() const override
    {
      return "creates a raw HTML site";
    }

    int run() override;
  private:
  };
}
