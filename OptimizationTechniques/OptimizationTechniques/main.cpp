#include <iostream>

#include "math/Math.h"

int main()
{
	agarzon::Vec3 a;
	agarzon::Vec3 b(10.0f, 0.0f, 0.0f);
	agarzon::Vec3 c(2.0f, 2.0f, 2.0f);

	std::cout << agarzon::Distance(a, b) << " " << agarzon::Distance(a, c) << " " << agarzon::Distance(b, c);

	std::cout << "Press any key to continue";
	std::getchar();
	return 0;
}
