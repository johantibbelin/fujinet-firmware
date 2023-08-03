#!/bin/bash

# a simple interface to pio so I don't have to remember all the args it needs, and can stop the dependency spam from filling the scroll buffer.
# args can be combined, e.g. '-cbxufm' and in any order. switches with values must be specified separately.
# typical usage:
#   ./build.sh -h           # display help for this script!
#   ./build.sh -c           # clean
#   ./build.sh -bx          # build, but don't output the dependency graph spam
#   ./build.sh -m           # monitor device
#   ./build.sh -ux          # upload image, no spam in build
#   ./build.sh -fx          # upload file system, no spam in build

RUN_BUILD=0
ENV_NAME=""
DO_CLEAN=0
SHOW_GRAPH=0
SHOW_MONITOR=0
TARGET_NAME=""
UPLOAD_IMAGE=0
UPLOAD_FS=0
EXCLUDE_GRAPH=0
DEV_MODE=0
ZIP_MODE=0
AUTOCLEAN=1

function show_help {
  echo "Usage: $(basename $0) [-b|-e ENV|-c|-m|-x|-t TARGET|-h]"
  echo "   -b       # run build"
  echo "   -c       # run clean before build"
  echo "   -d       # add dev flag to build"
  echo "   -m       # run monitor after build"
  echo "   -n       # do not autoclean"
  echo "   -u       # upload image (device code)"
  echo "   -f       # upload filesystem (webUI etc)"
  echo "   -x       # exclude dep graph output from logging"
  echo "   -e ENV   # use specific environment"
  echo "   -t TGT   # run target task (default of none means do build, but -b must be specified"
  echo "   -z       # build flashable zip"
  echo "   -h       # this help"
  exit 1
}

if [ $# -eq 0 ] ; then
  show_help
fi

while getopts ":bcde:fmnt:uxzh" flag
do
  case "$flag" in
    b) RUN_BUILD=1 ;;
    c) DO_CLEAN=1 ;;
    d) DEV_MODE=1 ;;
    e) ENV_NAME=${OPTARG} ;;
    f) UPLOAD_FS=1 ;;
    m) SHOW_MONITOR=1 ;;
    n) AUTOCLEAN=0 ;;
    t) TARGET_NAME=${OPTARG} ;;
    u) UPLOAD_IMAGE=1 ;;
    x) EXCLUDE_GRAPH=1 ;;
    z) ZIP_MODE=1 ;;
    h) show_help ;;
    *) show_help ;;
  esac
done
shift $((OPTIND - 1))

# remove any AUTOADD option left from previous run, and delete the generated backup file
sed -i.bu '/# AUTOADD/d' platformio.ini
rm 2>/dev/null platformio.ini.bu

if [ ${ZIP_MODE} -eq 1 ] ; then
  # find line with post:build_firmwarezip.py and add before it the option uncommented
  sed -i.bu '/^;[ ]*post:build_firmwarezip.py/i\
    post:build_firmwarezip.py # AUTOADD
' platformio.ini
  pio run -t clean -t buildfs
  pio run --disable-auto-clean
  sed -i.bu '/# AUTOADD/d' platformio.ini
  rm 2>/dev/null platformio.ini.bu
  exit 0
fi


ENV_ARG=""
if [ -n "${ENV_NAME}" ] ; then
  ENV_ARG="-e ${ENV_NAME}"
fi

TARGET_ARG=""
if [ -n "${TARGET_NAME}" ] ; then
  TARGET_ARG="-t ${TARGET_NAME}"
fi

DEV_MODE_ARG=""
if [ ${DEV_MODE} -eq 1 ]; then
  DEV_MODE_ARG="-a dev"
fi

if [ ${DO_CLEAN} -eq 1 ] ; then
  pio run -t clean ${ENV_ARG}
fi

AUTOCLEAN_ARG=""
if [ ${AUTOCLEAN} -eq 0 ] ; then
  AUTOCLEAN_ARG="--disable-auto-clean"
fi

if [ ${RUN_BUILD} -eq 1 ] ; then
  if [ ${EXCLUDE_GRAPH} -eq 1 ] ; then
    pio run ${DEV_MODE_ARG} $ENV_ARG $TARGET_ARG $AUTOCLEAN_ARG 2>&1 | egrep -av '^\|   \|'
  else
    pio run ${DEV_MODE_ARG} $ENV_ARG $TARGET_ARG $AUTOCLEAN_ARG 2>&1
  fi
fi

if [ ${UPLOAD_FS} -eq 1 ]; then
  if [ ${EXCLUDE_GRAPH} -eq 1 ]; then
    pio run ${DEV_MODE_ARG} -t uploadfs 2>&1 | egrep -av '^\|   \|'
  else
    pio run ${DEV_MODE_ARG} -t uploadfs 2>&1
  fi
fi

if [ ${UPLOAD_IMAGE} -eq 1 ]; then
  if [ ${EXCLUDE_GRAPH} -eq 1 ]; then
    pio run ${DEV_MODE_ARG} -t upload 2>&1 | egrep -av '^\|   \|'
  else
    pio run ${DEV_MODE_ARG} -t upload 2>&1
  fi
fi

if [ ${SHOW_MONITOR} -eq 1 ]; then
  if [ ${EXCLUDE_GRAPH} -eq 1 ]; then
    pio device monitor 2>&1 | egrep -av '^\|   \|'
  else
    pio device monitor 2>&1
  fi
fi
