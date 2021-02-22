Debug build:
`clang++ -std=c++2a -Wall -Werror -Og -g3 -glldb -fno-omit-frame-pointer -lpthread parallel.cc -o parallel`

Release build:
`clang++ -std=c++2a -Wall -Werror -Ofast -march=native -lpthread parallel.cc -o parallel`