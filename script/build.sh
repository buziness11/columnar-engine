sudo apt-get install libgtest-dev clang-format clang-tidy boost

git clone --depth 1 --branch v0.7.1 https://github.com/google/glog.git
cd glog
cmake -S . -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
sudo cmake --build build --target install
sudo ldconfig # Обновляем кэш библиотек
cd ..

mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
