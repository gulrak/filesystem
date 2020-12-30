#!/bin/sh
cd build
echo "Tests run as user: $USER"
ctest -E Windows
if [ -f "test/std_filesystem_test" ]; then
  test/std_filesystem_test || true
fi
