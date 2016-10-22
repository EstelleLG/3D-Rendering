// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "Calculator.h"




int main(int argc, char** argv)
{

	Estelle::Calculator test;
	auto output = test.Parse(argv[1]);
	printf("%f", output);
	std::cin.get();


    return 0;
}


