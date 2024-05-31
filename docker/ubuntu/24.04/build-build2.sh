#!/bin/sh -ex

export BUILD2_VERSION="0.17.0-a.0"
export BUILD2_FINGERPRINT="EC:50:13:E2:3D:F7:92:B4:50:0B:BF:2A:1F:7D:31:04:C6:57:6F:BC:BE:04:2E:E0:58:14:FA:66:66:21:1F:14"

curl -sSfO https://stage.build2.org/0/$BUILD2_VERSION/build2-install-$BUILD2_VERSION-stage.sh
chmod +x build2-install-$BUILD2_VERSION-stage.sh
sh build2-install-$BUILD2_VERSION-stage.sh --yes --trust "$BUILD2_FINGERPRINT"
