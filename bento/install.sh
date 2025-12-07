echo "compiling bento..."
clang++ bento/bento.cpp -o bentoc -std=c++23
echo "moving bentoc to /usr/local/bin/..."
mv bentoc /usr/local/bin/bentoc
echo "finished installing"