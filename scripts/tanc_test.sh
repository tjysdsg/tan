#!/usr/bin/env bash
for f in src/test/test_src/*.tan; do
  echo "=========================="
  ./bin/tanc $f || exit 1
  ./a.out || exit 1
  echo "=========================="
done
