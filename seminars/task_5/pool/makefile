# ya.contest build
build:
	TMP=$$(pwd) clang++ -std=c++14 -fsanitize=thread -pthreads -O3 -g -Wall -Wextra -Werror -o ./solution *.cpp

# ya.contest run
run:
	./solution 2>&1

tar:
	@tar cf ../$$(basename $$(pwd)).tar includes solutions tests makefile

local_build:
	rm -rf build && mkdir build && ln -s $$(pwd)/includes/* ./build/ \
	&& ln -s $$(pwd)/solutions/thread_pool.h ./build/ \
	&& cd build && clang++ -fsanitize=thread -std=c++14 -pthreads -O3 -g -Wall -Wextra -Werror -o ./solution *.cpp
