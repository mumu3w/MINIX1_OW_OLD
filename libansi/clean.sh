#!/bin/bash

find . -name '*.obj' -type f -print -exec rm -rf {} \;
find . -name '*.err' -type f -print -exec rm -rf {} \;
find . -name '*.exe' -type f -print -exec rm -rf {} \;
rm -fr libansi.*