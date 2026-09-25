#pragma once
#include "create.hpp"
#include "../restore_into_command.hpp"
#include "../databases/mysql.hpp"

namespace Wordpress
{
  typedef ::RestoreIntoCommand<Wordpress::CreateCommand, MysqlDatabase>
    RestoreIntoCommand;
}
