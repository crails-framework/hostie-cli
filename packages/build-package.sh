package_path="/opt/hostie-package/$1-$2"
manifest="$package_path/manifest"
depends=()
arch=amd64

rm -rf /opt/package-src /opt/$1_$2_$arch
mkdir /opt/package-src

cd /opt/package-src
mkdir -p bin
mkdir -p lib/pkgconfig
cp "$package_path/$1/$1"      bin 2> /dev/null || echo "empty bin folder"
cp "$package_path/$1/$1"*.so  lib 2> /dev/null || echo "empty lib folder"
cp "$package_path/$1/$1"*.pc  lib/pkgconfig 2> /dev/null || echo "no pkgconfig file"

depends+=("libstdc++6") # gcc-13 something

if [ $1 == "hostie-cli" ] ; then
  depends+=("curl")
fi

if grep libcrypto "manifest" ; then
  echo "+ Found dependency to openssl"
  depends+=("openssl")
fi

if grep libbacktrace "manifest" && $1 != "libbacktrace" ; then
  echo "+ Found dependency to libbacktrace"
  depends+=("libbacktrace")
fi

for boostlib in date-time program-options
do
  if grep libboost-$boostlib "$manifest" | grep depends ; then
    echo "+ Found dependency to libboost-$boostlib"
    depends+=("libboost-$boostlib$BOOST_VERSION")
  fi
done

for crailslib in encrypt semantics cli logger random readfile database-url crontab
do
  if grep libcrails-$crailslib "$manifest" | grep depends ; then
    echo "+ Found dependency to libcrails-$crailslib"
    echo "++ grep libcrails-$crailslib $manifest"
    depends+=("libcrails-$crailslib (>= 2.0.0)")
  fi
done

echo "GENERATING DEB PACKAGE"

depends_content=$(IFS=, ; echo "${depends[*]}")

mkdir DEBIAN
touch DEBIAN/control
echo "Package: $1" > DEBIAN/control
echo "Version: $2" >> DEBIAN/control
echo "Architecture: $arch" >> DEBIAN/control
echo "Maintainer: PlanED S.L. <michael@planed.es>" >> DEBIAN/control
echo "Description: PlanED provided package" >> DEBIAN/control
echo "Depends: $depends_content" >> DEBIAN/control

cd /opt
mv /opt/package-src /opt/$1_$2_$arch
dpkg-deb --build --root-owner-group $1_$2_$arch

mkdir -p /opt/dist
mv $1_$2_$arch.deb /opt/dist
