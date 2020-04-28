#!/bin/bash
./scripts/tanc_test.sh || exit 1

pushd src/test
../../bin/lexer_tests || exit 1
popd
