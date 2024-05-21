# hostie-cli

A command line interface tool to create, configure and web applications.

Supports the following web applications:
- [Wordpress](https://wordpress.com/)
- [CrailsCMS](https://github.com/crails-cms/crails-cms)
- [NextCloud](https://nextcloud.com) (TODO)
- [Odoo](https://www.odoo.com) (TODO)

## Installing

crails-backup uses the [build2](https://www.build2.org/) build system, and you
may use this git repository as a build2 package.

Assuming you have installed build2's bpkg tool, as well as boost's development
files, you may build this project from source using the following commands:

```sh
BUILD_DIR=build-hostie
bpkg create -d $BUILD_DIR cc config.cxx=g++ config.bin.rpath=/usr/local/lib config.install.root=/usr/local config.install.sudo=sudo
cd $BUILD_DIR
bpkg add --type git "https://github.com/crails-framework/hostie-cli.git#master"
bpkg fetch
bpkg build hostie-cli '?sys:libboost-program-options/*' '?sys:libboost-date-time/*' '?sys:libboost-process/*' '?sys:libboost-asio/*'
```

## How to use

The hostie-cli command supports various web applications that you may host on
your server. The first argument of any call to `hostie-cli` should indicate
the type of web application you want to interact with.

The second argument should indicate the type of action you wish to execute.
Common actions are:
- **list** - lists instances of the given type managed by hostie-cli
- **config** - gets or sets environment variables for a given web application instance
- **create** - creates a new web application instance
- **remove** - removes a web application instance
- **backup** - manages backup for an instance (scheduling, listing)
- **restore** - restores an instance's backup

Some additional actions are available for specific application types. You
may lists available actions for each application type simply by not
mentionning any action argument, such as:

```
hostie-cli wordpress # will list all available actions for Wordpress applications
```

Some preparation steps must be taken for each application type before hostie-cli
can create and manage new instances.

### Wordpress

TODO

### CrailsCMS

TODO
