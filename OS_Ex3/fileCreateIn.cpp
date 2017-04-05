#include <fstream>
using namespace std;

ifstream fileCreateIn(char* fileName)
{
	ifstream fin(fileName, ios::binary);
	return fin;
}