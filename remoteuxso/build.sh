#set -x
#rm -f *.so *.bin

if [ $1 == 'DEBUG' ]
then
g++ -Xlinker -rpath -Xlinker -lutil -g -DDEBUG -shared RemoteInterface.cpp  remoteux.cpp -o libremoteux.so
g++ -Xlinker -rpath -Xlinker -lutil -g -DDEBUG -lutil -o IPscan.bin ipscan.cpp ./libremoteux.so
else
g++ -Xlinker -rpath -Xlinker -lutil -g -shared RemoteInterface.cpp  remoteux.cpp -o libremoteux.so
g++ -Xlinker -rpath -Xlinker -lutil -g -lutil -o IPscan.bin ipscan.cpp ./libremoteux.so
fi

if [ $1 == 'IPscan' ]
then
g++ -Xlinker -rpath -Xlinker -lutil -g -lutil -o IPscan.bin ipscan.cpp ./libremoteux.so
fi

if [ $1 == 'rX' ]
then
g++ -Xlinker -rpath -Xlinker -lutil -g -shared RemoteInterface.cpp  remoteux.cpp -o libremoteux.so
g++ -Xlinker -rpath -Xlinker -lutil -g -lutil -o rX.bin main.cpp ./libremoteux.so
fi

if [ $1 == 'lib' ]
then
g++ -Xlinker -rpath -Xlinker -lutil -g -shared RemoteInterface.cpp  remoteux.cpp -o libremoteux.so
fi