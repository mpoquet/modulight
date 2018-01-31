#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <modulight/module.hpp>
#include <QTime>

using namespace std;
using namespace modulight;

int main(int argc, char ** argv)
{
	srand(time(0));

	Module m("Generator", argc, argv);
	m.addOutputPort("out");

	if (!m.initialize())
		return 0;

	QVector<float> data;
	data.resize(50);

	for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < data.size(); ++j)
			data[j] = (rand() / (float)RAND_MAX) * 89 + 10;

		m.send("out", (char*)data.data(), data.size() * sizeof(float));
	}

	return 0;
}