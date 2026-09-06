#!/bin/sh

set -eu

if [ "$#" -eq 0 ]; then
    set -- assets/icons/icon.svg
fi

for svg in "$@"; do
    base=$(basename "$svg" | sed 's/\.[^.]*$//')
    directory=$(dirname "$svg")
    iconset="$directory/$base.iconset"
    output="$directory/$base.icns"

    rm -rf "$iconset"
    mkdir -p "$iconset"
    trap 'rm -rf "$iconset"' EXIT INT TERM

    for params in \
        16,16x16 \
        32,16x16@2x \
        32,32x32 \
        64,32x32@2x \
        128,128x128 \
        256,128x128@2x \
        256,256x256 \
        512,256x256@2x \
        512,512x512 \
        1024,512x512@2x
    do
        size=${params%%,*}
        label=${params#*,}
        svg2png -w "$size" -h "$size" "$svg" "$iconset/icon_$label.png"
    done

    iconutil -c icns "$iconset" -o "$output"
    rm -rf "$iconset"
    trap - EXIT INT TERM
    printf 'Generated %s\n' "$output"
done
