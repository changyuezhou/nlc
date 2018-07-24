#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
docker run --rm -u $UID -v "$DIR":/usr/src/myapp -w /usr/src/myapp gcc:4.9 sh -c "chmod +x ./cmake-3.3.0-Linux-x86_64.sh && mkdir tmpcmake && chmod -R 777 deps/ && sh ./cmake-3.3.0-Linux-x86_64.sh --prefix=/usr/src/myapp/tmpcmake --skip-license && sh ./build.sh && rm -r /usr/src/myapp/tmpcmake"
