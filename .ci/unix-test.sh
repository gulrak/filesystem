#!/bin/sh
cd build
ctest -E Windows
if [ -f "test/std_filesystem_test" ]; then
  test/std_filesystem_test || true
fi
