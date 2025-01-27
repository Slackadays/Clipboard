default:
  @just --list

build:
  if [ ! -d "build" ]; then mkdir build; fi 

  cd build; cmake .. -DCMAKE_BUILD_TYPE=Release

  cd build; cmake --build . -j 12

  cd build; sudo cmake --install .

clean:
  if [ -d "build" ]; then rm -rf build; fi
