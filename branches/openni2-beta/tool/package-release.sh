#!/bin/sh
VER=1.0b

TARGET_NAME=kinect-kamehameha_${VER}_for_Linux
TARGET=release/$TARGET_NAME
rm -rf $TARGET
mkdir -p $TARGET
cd $TARGET
ln -s ../../../Linux_OpenNI2_Release/kinect-kamehameha .
find ../../../Linux_OpenNI2_Release -type l | xargs -I X ln -s X .
ln -s ../../../LICENSE.TXT .
ln -s ../../../README_*.TXT .
ln -s ../../../HISTORY_*.TXT .
ln -s ../../../buildLinux/install.sh .
cd ..
tar cvfhj $TARGET_NAME.tar.bz2 $TARGET_NAME
cd ..
