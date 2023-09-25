#! /bin/bash
TOOLS=../dsma-library/tools
ROBOT=./

python3 $TOOLS/md5_to_dsma.py \
    --model $ROBOT/apoco.md5mesh \
    --name apoco \
    --output models \
    --texture 128 128 \
    --anim $ROBOT/apoco.md5anim \
    --skip-frames 1 \
    --bin \
    --blender-fix