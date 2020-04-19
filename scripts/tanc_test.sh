#!/usr/bin/env bash
fail() {
  echo "$1 failed"
  exit 1
}

for f in src/test/test_src/*.tan; do
  ./bin/tanc runtime/print.tan $f || fail $f
  ./a.out || fail $f
  echo "=========================="
done
