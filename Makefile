
.PHONY: debug release clean 


debug :
	g++ -std=c++11 -o qwp src/row.h src/qwp.cpp

release :
	g++ -std=c++11 -o3 -Wall -Wextra -Werror -Wpedantic -o qwp src/row.h src/qwp.cpp

clean :
	-rm qwp.exe

