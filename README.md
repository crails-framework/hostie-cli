# hostie-cli

A command line interface tool to create, configure and web applications.

Supports the following web applications:
- [Wordpress](https://wordpress.com/)
- [CrailsCMS](https://github.com/crails-cms/crails-cms)
- [NextCloud](https://nextcloud.com)
- [Odoo](https://www.odoo.com)

## Installing

hostie-cli uses the [build2](https://www.build2.org/) build system, and you
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

You may also list all available options for any action by using the _help_ option:

```
hostie-cli wordpress create --help
```

Note that some preparation steps must be taken for each application type before
hostie-cli can create and manage new instances. Wizards are provided to
make such preparations in an easy and streamlined way:

## Wizards

Wizards will prepare your system for running the application you may later
create. They may be used to set up MySQL or PostgreSQL databases, a nginx
web server, or the specific dependencies for any web supported application.

For instance, the following command...

```sh
hostie-cli wizard nextcloud
```

... will install MySQL, php-fpm, and all the php packages required by NextCloud
on your system. Then, it will download the latest version of NextCloud and
install it in the `/opt` folder.

### Wordpress

The wordpress wizard can be used to support multiple versions of wordpress. By
default, it will download the latest version of Wordpress, and update hostie-cli's
configuration so all newly created wordpress application use the latest installed
version of Wordpress.

You may also force the installation of an older release by specifying a version
yourself:

```sh
WORDPRESS_VERSION=6.6.1 hostie-cli wizard wordpress
```

Note that, after doing that, all new Wordpress application you create will use
the 6.6.1 version. Update the `wordpress-version` and `wordpress-source` variables
in the `/etc/hostie.rc` file to modify that behavior.

## Creating applications

TODO

## Setting up the webserver

TODO

```sh
hostie-cli wizard nginx
```

```sh
hostie-cli nginx configure -n APPLICATION_NAME
```

### Acquiring and refreshing SSL certificates

```sh
hostie-cli nginx certify -n APPLICATION_NAME
```
