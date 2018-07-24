#!/bin/sh

TOPDIR=$(pwd)
OUTDIR=${TOPDIR}/static/build
COMMLIB=${TOPDIR}/src/lib
COMMINC=${TOPDIR}/src
LIBDIR=${TOPDIR}/static/ext
WORKDIR=${TOPDIR}/static/work
DISTDIR=${TOPDIR}/static/dist
DISTFILES=$(cat <<EOF
http://zlib.net/zlib-1.2.8.tar.gz
https://github.com/edenhill/librdkafka/archive/df3be26a76476d6271649c2340db7d1268b8aa46.tar.gz librdkafka-df3be26a76476d6271649c2340db7d1268b8aa46.tar.gz
https://github.com/google/protobuf/releases/download/v2.6.1/protobuf-2.6.1.tar.gz
http://www.digip.org/jansson/releases/jansson-2.7.tar.gz
EOF
)

problem=$(command -v wget)
echo Checking for wget: $problem
if [ -z "$problem" ]; then
    echo "No wget. Please install it first."
    exit 1
fi

problem=$(command -v pkg-config)
echo Checking for pkg-config: $problem
if [ -z "$problem" ]; then
    echo "No pkg-config. Please install it first."
    exit 1
fi

platform="unknown"
unamestr=$(uname -s)
if [ "$unamestr" = "Linux" ]; then
    platform="linux"
elif [ "$unamestr" = "FreeBSD" ]; then
    platform="freebsd"
elif [ "$unamestr" = "Darwin" ]; then
    platform="darwin"
fi

test -d ${OUTDIR} || mkdir -p ${OUTDIR}
test -d ${LIBDIR} || mkdir -p ${LIBDIR}
test -d ${WORKDIR} || mkdir -p ${WORKDIR}
test -d ${DISTDIR} || mkdir -p ${DISTDIR}

if ! echo ${PKG_CONFIG_PATH} | grep -q ${LIBDIR} ; then
    if [ -n "${PKG_CONFIG_PATH}" ]; then
        export PKG_CONFIG_PATH="${LIBDIR}/lib/pkgconfig":"${LIBDIR}/lib64/pkgconfig":${PKG_CONFIG_PATH}
    else
        export PKG_CONFIG_PATH="${LIBDIR}/lib/pkgconfig":"${LIBDIR}/lib64/pkgconfig"
    fi
fi

if ! echo ${PATH} | grep -q ${LIBDIR} ; then
    export PATH=${LIBDIR}/bin:$PATH
fi

die() {
    message=${1:-Error}
    echo $message
    exit 1
}

extract() {
    file=$1
    distfile=$(basename $file)
    package=${distfile%.tar.*}
    extname=${distfile##*.}
    test -f "${WORKDIR}/.extracted-$package" && exit 0
    echo Extracting $distfile
    params=""
    case $extname in
        "gz")
            params="-zxf"
            ;;
        "bz2")
            params="-jxf"
            ;;
        "xz")
            params="-Jxf"
            ;;
        *)
            die "Unable to extract file: $package"
    esac
    tar -C ${WORKDIR} $params $file && touch "${WORKDIR}/.extracted-$package"
    echo .
}

build() {
    package=$1
    test -f "${WORKDIR}/.done-$package" && echo "${package} is built" && exit 0
    echo Start to build ${package}
    cmd="configure"
    params=""
    makeopts=""
    installopts=""
    case $package in
        *zlib*)
            export CFLAGS="${CFLAGS} -fPIC"
            params="--static"
            ;;
        *librdkafka*)
            export CFLAGS="${CFLAGS} -I${LIBDIR}/include"
            export LDFLAGS="${LDFLAGS} -L${LIBDIR}/lib"
            params="--enable-static"
            ;;
        *jansson*|*protobuf*)
            params="--disable-shared --enable-static"
            ;;
        *)
            ;;
    esac
    if [ -x "$cmd" ]; then
        ./$cmd --prefix=${LIBDIR} $params && make && make install || die "Failed to build $package"
    else
        make $makeopts && make $installopts install || die "Failed to build $package"
    fi
    touch "${WORKDIR}/.done-$package"
    exit 0
}

get_packages() {
    echo "Get packages..."
    echo "${DISTFILES}" | while read line
    do
        url=$(echo "$line" | cut -d' ' -f1)
        distfile=$(echo "$line" | cut -d' ' -s -f2)
        if [ -z "$distfile" ]; then
            distfile=$(basename $url)
        fi
        echo "Getting $url"
        wget -nc --no-check-certificate $url -O ${DISTDIR}/$distfile
        test -f "${DISTDIR}/$distfile" || die "$distfile not downloaded" \
            && (cd ${DISTDIR} && extract $distfile) || die "extract $distfile failed"
    done
}

build_packages() {
    echo "Build packages..."
    echo "${DISTFILES}" | while read line
    do
        url=$(echo "$line" | cut -d' ' -f1)
        distfile=$(echo "$line" | cut -d' ' -s -f2)
        if [ -z "$distfile" ]; then
            distfile=$(basename $url)
        fi
        package=${distfile%.tar.*}
        if echo "$package" | grep "ncurses"; then
            package=$(basename $(ls -d ${WORKDIR}/ncurses*))
        fi
        test -d "${WORKDIR}/$package" || die "$package not found"
        (cd "${WORKDIR}/${package}" && build $package) || die "build $package failed"
    done
}
#export LDFLAGS="${LDFLAGS} -L${LIBDIR}/lib -L/bruce/project/test_lib/app/result/src -lrt"
build_lts() {
    echo "Build lts..."
    export CFLAGS="${CFLAGS} -I${LIBDIR}/include -I${COMMINC}/include"
    export LDFLAGS="${LDFLAGS} -L${LIBDIR}/lib -L${COMMLIB} -lrt"
    cd ${OUTDIR} \
    && cmake -DCMAKE_INSTALL_PREFIX:PATH=${OUTDIR} \
    -DPROTOBUF_LIBRARY=${LIBDIR}/lib64/libprotobuf.a -DPROTOBUF_INCLUDE_DIR=${LIBDIR}/include \
    -DJANSSON_LIBRARY=${LIBDIR}/lib64/libjansson.a -DJANSSON_INCLUDE_DIR=${LIBDIR}/include \
    -DKAFKA_LIBRARY=${LIBDIR}/lib/librdkafka.a -DKAFKA_INCLUDE_DIR=${LIBDIR}/include \
    -DZLIB_LIBRARY=${LIBDIR}/lib/libz.a -DZLIB_INCLUDE_DIR=${LIBDIR}/include \
    -DCMAKE_BUILD_TYPE=Release ${TOPDIR} && make && make install
}

get_packages && build_packages && build_lts


