mkdir build
cd build
cmake ../
cmake --build .
cd ..

mkdir release
cd release
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/mingw-w64-x86_64.cmake ../
cmake --build .
cd ..



