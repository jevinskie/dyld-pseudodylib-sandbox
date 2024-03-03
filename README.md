# dyld-pseudodylib-sandbox
Playing around with dyld’s new pseudo-dylib APIs introduced in macOS 14/iOS 17

## Building
`cmake -W dev ../git/dyld-pseudodylib-sandbox -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$(greadlink -f ../prefix) -DCMAKE_C_COMPILER=clang-19 -DCMAKE_CXX_COMPILER=clang++-19 -DCMAKE_RANLIB=llvm-ranlib -DCMAKE_AR=llvm-ar`
