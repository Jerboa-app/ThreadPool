if [ -f "a.out" ]; then
    rm a.out
fi
g++ -std=c++11 -O3 -pthread main.cpp