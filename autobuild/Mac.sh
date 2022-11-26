#!/bin/bash
brew install llvm
echo "CMAKE_CONFIGURE_ARGS=-DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_COMPILER=/usr/local/opt/llvm/bin/clang -DCMAKE_CXX_COMPILER=/usr/local/opt/llvm/bin/clang++ -DCMAKE_CXX_FLAGS=-Wno-parentheses -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9" >> $GITHUB_ENV

cat autobuild/mac_any_cast.h >> src/C4InteractiveThread.h
