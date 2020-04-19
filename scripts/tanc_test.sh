#!/usr/bin/env bash
fail() {
  echo "$1 failed"
  exit 1
}

for f in src/test/test_src/*.tan; do
  echo "=========================="
  ./bin/tanc $f || fail $f
  ./a.out || fail $f
  echo "=========================="
done
