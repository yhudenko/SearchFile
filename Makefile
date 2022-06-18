all: program

program: SearchFile.o
	g++ SearchFile.cpp libSearchFileByName.a -o program
	
SearchFile.o: SearchFile.cpp
	g++ -c SearchFile.cpp
	
clean:
	rm -rf *o program