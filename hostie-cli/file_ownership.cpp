#include "file_ownership.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

using namespace std;

namespace Crails
{
  long uid_from_username(const string& name)
  {
    struct passwd* pwd = getpwnam(name.c_str());

    return pwd ? pwd->pw_uid : -1;
  }

  long gid_from_groupname(const string& name)
  {
    struct group* grp = getgrnam(name.c_str());

    return grp ? grp->gr_gid : -1;
  }

  bool chown(const filesystem::path& path, const string_view username)
  {
    uid_t uid = uid_from_username(string(username));

    return uid > 0 && chown(path, uid);
  }

  bool chgrp(const filesystem::path& path, const string_view groupname)
  {
    gid_t gid = gid_from_groupname(string(groupname));

    return gid > 0 && chgrp(path, gid);
  }

  bool chown(const filesystem::path& path, long uid)
  {
    int result = -1;

    if (filesystem::exists(path))
      result = ::chown(path.string().c_str(), uid, -1);
    return result == 0;
  }

  bool chgrp(const filesystem::path& path, long gid)
  {
    int result = -1;

    if (filesystem::exists(path))
      result = ::chown(path.string().c_str(), -1, gid);
    return result == 0;
  }
}
