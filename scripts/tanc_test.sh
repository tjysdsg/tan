#!/usr/bin/env bash
fail() {
  echo "$1 failed"
  exit 1
}

for f in src/test/test_src/*.tan; do
  ./bin/tanc $f -l runtime/runtime.so || fail $f
  ./a.out || fail $f
  echo "=========================="
done
