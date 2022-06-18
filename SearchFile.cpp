#include "SearchFileByName.h"
#include <string>
#include <iostream>

int main()
{
	std::string directory_name = "C:\\";
	std::string file_name = "my.txt";

	std::cout << "File name: ";
	std::cin >> file_name;

	std::cout << "Start search!" << std::endl;

	std::string file_path = controller(directory_name, file_name);
	if (file_path != "")
		std::cout << "Path: " << file_path << std::endl;
	system("pause");
	return 0;
}
