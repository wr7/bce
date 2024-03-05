rm -rf ./build
meson setup build

alias b='meson compile -C build'
alias r='b && build/bce'
