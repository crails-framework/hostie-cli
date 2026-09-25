# hostie-cli

A command line interface tool to create, configure and manage web applications.

Supports the following web applications:
- [Wordpress](https://wordpress.com/)
- [CrailsCMS](https://github.com/crails-cms/crails-cms)
- [NextCloud](https://nextcloud.com)
- [Odoo](https://www.odoo.com)

## Installing

### Repositories

Packaged releases of hostie-cli are available at the following repositories:

- [Ubuntu](https://github.com/planed-es/ubuntu-ppa)
- [FreeBSD](https://github.com/planed-es/freebsd-repo)

### Building from source

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

For each type of application, you may use the `create` command to generate a
new instance. Common options are:

- `-n` for the instance name
- `-u` for the system user which will own the instance files
- `-x` for the domain names the instance will answer to

For instance, you could create a Wordpress instance with the following command:

```sh
hostie-cli wordpress create -n my_site -u site_user -x mydomain.com www.mydomain.com
```

Once an instance has been created, you will need to register it on a web
server, such as NGINX:

```sh
hostie-cli wizard nginx # only if you haven't run this command before
hostie-cli nginx configure -n my_site
```

Assuming you have created the appropriate DNS records, your site should now
be up and available. You'll probably want to add a certificate and SSL support:

```sh
hostie-cli nginx certify -n my_site
```

### Application ports

Some application may require a port to run (Odoo, CrailsCMS), in which case,
you may specify the port with the `-p` option:

```sh
hostie-cli crailscms create -n my_cms -u my_cms -x mycms.com -p 3001
```

### Odoo options

The Odoo creator provides additional options. Odoo needs a port for _gevent_:
by default, it will use the provided port increased by one. You may also
specify the gevent port manually:

```sh
hostie-cli odoo create -n my_odoo -u odoo -x myodoo.com -p 3002 --gevent-port 3003
```

You may also want to disable demo data for modules:

```sh
hostie-cli odoo create -n my_odoo -u odoo -x myodoo.com -p 3002 --without-demo
```
