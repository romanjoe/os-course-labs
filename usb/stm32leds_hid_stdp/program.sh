#!/bin/bash

# set initial openOCD from env
OPENOCD_SCRIPTS=${OPENOCD_PATH}
OPENOCD_BIN=${OPENOCD_BIN}

# # if TARGET is not passed as argument
# # use $TARGET defined as environment var
# # VSCode task set this var before run
TARGET=${TARGET}

# take default build configuration from
# env variables
BUILDCFG=${BUILDCFG}

# set time to wait after programming
# before exit
WAIT_FOR=3

# base directory
PWD=$(pwd)
BUILD=$PWD/BUILD

USAGE=$'USAGE: program.sh -a:t:uw:b
        Options:
            -a APP_NAME - name of binary to program
            -t TARGET_NAME - name of target to program to
            -w (optional) - time to wait in sec before ending flashing job
            -o (optional) - path to openOCD root directory
            -e (no args) - erase chip
            -p (no args) - program chip with $APP_NAME.hex file
            '

while getopts "a:t:w:epo:" OPTION; do
    case $OPTION in
    a)
        # check if app name passed, add _signed suffix
        # for user application type of images
        APP_NAME_PASSED=$OPTARG
        if [[ $APP_NAME_PASSED != "" ]]
        then
            APP_NAME=$APP_NAME_PASSED
            BUILD_DIR_APP=$BUILD/$APP_NAME_PASSED/$TARGET/$BUILDCFG
        else
            echo "$USAGE"
            exit 1
        fi
        ;;
    t)
        # assing name of cmd if present 
        TARGET=$OPTARG
        if [[ $TARGET == "" ]]
        then
            echo "$USAGE"
            exit 1
        fi
        ;;
    w)
        WAIT_FOR=$OPTARG
        ;;
    e)  COMMAND="flash erase_address 0x08000000 0x100000"
        ;;
    p)  COMMAND="program $BUILD_DIR_APP/$APP_NAME.hex"
        ;;
    o)
        # update path to openOCD if passed
        if [[ $OPTARG != "" ]]
        then
            OPENOCD=$OPTARG
        else
            echo "$USAGE"
            exit 1
        fi
        ;;
    *)
        echo "Incorrect usage!"
        echo "$USAGE"
        exit 1
        ;;
    esac
done

echo Flashing "$APP_NAME"
echo To target "$TARGET"

$OPENOCD_BIN -s $OPENOCD_SCRIPTS/scripts \
                -f $OPENOCD_SCRIPTS/scripts/board/stm32f4discovery.cfg \
                -c "init; reset init" \
                -c "$COMMAND"\
                -c "reset; shutdown"

#sleep $WAIT_FOR

exit 0