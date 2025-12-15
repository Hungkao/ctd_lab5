#!/bin/bash

echo "===== BUILD PROJECT ====="
cd incompleted || exit 1
make || exit 1
cd ..

echo ""
echo "===== RUN ALL TESTS ====="

for f in tests/example*.kpl
do
  echo ""
  echo "=============================="
  echo "TEST FILE: $f"
  echo "------------------------------"

  # chạy parser và in trực tiếp ra màn hình
  incompleted/kplc "$f"

  echo "=============================="
done

echo ""
echo "===== DONE ====="
