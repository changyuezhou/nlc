#!/bin/bash
TOPDIR=$(pwd)
OUTDIR=${TOPDIR}/target/build
LIBDIR=${TOPDIR}/target/ext
WORKDIR=${TOPDIR}/target/work
DISTDIR=${TOPDIR}/deps

DISTFILES=$(cat <<EOF
zlib
librdkafka
protobuf
jansson
EOF
)

problem=$(command -v pkg-config)
osflags=""
echo Checking for pkg-config: $problem
if [ -z "$problem" ]; then
    echo "No pkg-config. Please install it first."
    exit 1
fi

platform="unknown"
unamestr=$(uname -s)
if [ "$unamestr" = "Linux" ]; then
    platform="linux"
    osflags="-lrt"
elif [ "$unamestr" = "FreeBSD" ]; then
    platform="freebsd"
elif [ "$unamestr" = "Darwin" ]; then
    platform="darwin"
fi

test -d ${OUTDIR} || mkdir -p ${OUTDIR}
test -d ${LIBDIR} || mkdir -p ${LIBDIR}
test -d ${WORKDIR} || mkdir -p ${WORKDIR}

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

copy() {
	package=$1
	from="${DISTDIR}/$package"
	to="${WORKDIR}/$package"
	cp -r $from $to
}

build() {
    package=$1
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
            export LDFLAGS="${LDFLAGS} -L${LIBDIR}/lib ${osflags}"
            params="--enable-static"
            ;;
        *jansson*)
			autoreconf -i
            params="--disable-shared --enable-static"
            ;;
         *protobuf*)
			./autogen.sh
			params="--disable-shared --enable-static"
			;;
        *)
            ;;
    esac
    if [ -e "$cmd" ]; then
        ./$cmd --prefix=${LIBDIR} $params && make && make install || die "Failed to build $package"
    else
        make $makeopts && make $installopts install || die "Failed to build $package"
    fi
    exit 0
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

        copy $package
        test -d "${WORKDIR}/$package" || die "$package not found"
        echo "${WORKDIR}/$package"
        (cd "${WORKDIR}/${package}" && build $package) || die "build $package failed"
    done
}

build_lts() {
    echo "Build lts..."
    export CFLAGS="${CFLAGS} -I${LIBDIR}/include"
    export LDFLAGS="${LDFLAGS} -L${LIBDIR}/lib ${osflags}"
    cd ${OUTDIR} \
    && /usr/src/myapp/tmpcmake/bin/cmake -DCMAKE_INSTALL_PREFIX:PATH=${OUTDIR} \
    -DPROTOBUF_LIBRARY=${LIBDIR}/lib/libprotobuf.a -DPROTOBUF_INCLUDE_DIR=${LIBDIR}/include \
    -DJANSSON_LIBRARY=${LIBDIR}/lib/libjansson.a -DJANSSON_INCLUDE_DIR=${LIBDIR}/include \
    -DKAFKA_LIBRARY=${LIBDIR}/lib/librdkafka.a -DKAFKA_INCLUDE_DIR=${LIBDIR}/include \
    -DZLIB_LIBRARY=${LIBDIR}/lib/libz.a -DZLIB_INCLUDE_DIR=${LIBDIR}/include \
    -DCMAKE_BUILD_TYPE=Release ${TOPDIR} && make && make install
}

build_packages && build_lts
