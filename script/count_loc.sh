#!/bin/sh
printf "lines of code   "
find src/ include/ -type f | xargs egrep -h '[^ ].*[^ ]' | wc -l
printf "  headers         "
find include/ -type f | xargs egrep -h '[^ ].*[^ ]' | wc -l
printf "  sources         "
find src/ -type f | xargs egrep -h '[^ ].*[^ ]' | wc -l

printf "physical lines  "
find src/ include/ -type f | xargs cat | wc -l
printf "  headers         "
find include/ -type f | xargs cat | wc -l
printf "  sources         "
find src/ -type f | xargs cat | wc -l

printf "files           "
find src/ include/ -type f | wc -l
printf "  headers         "
find include/ -type f | wc -l
printf "  sources         "
find src/ -type f | wc -l
