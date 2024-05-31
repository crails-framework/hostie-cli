BASEDIR=$(dirname $0)
REL_VERSION=$1

if [ $REL_VERSION == 24.04 ] ; then
  export BOOST_VERSION=1.83.0
else
  export BOOST_VERSION=1.74.0
fi

for package in libcrails-cli libcrails-crontab libcrails-database-url libcrails-logger libcrails-random libcrails-readfile libcrails-semantics libcrails-encrypt
do
  bash "$BASEDIR/build-package.sh" $package 2.0.0
done

bash "$BASEDIR/build-package.sh" libbacktrace 1.0.0
bash "$BASEDIR/build-package.sh" crails-backup 1.0.0
bash "$BASEDIR/build-package.sh" hostie-cli 0.1.0

mv /opt/dist/*.deb /opt/shared/packages/ubuntu/$REL_VERSION
