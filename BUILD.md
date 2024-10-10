```
cmake -G Ninja -S llvm -B build -DLLVM_ENABLE_PROJECTS="clang" -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind" -DCMAKE_BUILD_TYPE=Debug
```

latest mac build cmd:
```
cmake -G Ninja -S llvm -B build -DLLVM_ENABLE_PROJECTS="clang" -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind" -DCMAKE_BUILD_TYPE=Debug -DLLVM_OPTIMIZED_TABLEGEN=ON -DCMAKE_INSTALL_PREFIX=$PWD/install
```

latest docker build cmd:
```
cmake -G Ninja -S llvm -B build_docker -DLLVM_ENABLE_PROJECTS="clang" -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind" -DCMAKE_BUILD_TYPE=Debug -DLLVM_OPTIMIZED_TABLEGEN=ON -DCMAKE_INSTALL_PREFIX=$PWD/install_docker
```
