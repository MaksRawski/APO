#include "ImageProcessor.h"
#include <iostream>

int main(){ 
	LUT neg = negate();	
	std::cout << neg[0] << std::endl;
}
