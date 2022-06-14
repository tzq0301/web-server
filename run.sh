cmake . -Bbuild
cd build || exit
make
cd .. || exit
chmod +x ./build/WebServer
./build/WebServer