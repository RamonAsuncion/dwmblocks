#!/bin/bash

update() {
    sum=0
    for arg; do
        read -r i < "$arg"
        sum=$(( sum + i ))
    done
    cache="$HOME/.cache/${1##*/}"
    mkdir -p "$HOME/.cache"
    [ -f "$cache" ] && read -r old < "$cache" || old=0
    printf %d\\n "$sum" > "$cache"
    printf %d\\n $(( sum - old ))
}

rx=$(update /sys/class/net/[ew]*/statistics/rx_bytes)
tx=$(update /sys/class/net/[ew]*/statistics/tx_bytes)

# FIXME: Fix the formatting the spacing is crazy.
printf "up: %4sB / down: %4sB\n" $(numfmt --to=iec "$tx" "$rx")
