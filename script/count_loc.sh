#!/bin/sh
loc_regex='[a-zA-Z0-9].*[a-zA-Z0-9]'
printf "lines of code   "
find src/ include/ -type f | xargs egrep -h "$loc_regex" | wc -l
printf "  headers         "
find include/ -type f | xargs egrep -h "$loc_regex" | wc -l
printf "  sources         "
find src/ -type f | xargs egrep -h "$loc_regex" | wc -l

assert_regex='assert\('
printf "asserts         "
find src/ include/ -type f | xargs egrep -h "$assert_regex" | wc -l

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
