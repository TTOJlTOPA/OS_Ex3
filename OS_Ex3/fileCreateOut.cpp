#include <fstream>
using namespace std;

ofstream fileCreateOut(char* fileName)
{
	ofstream fout(fileName, ios::binary);
	return fout;
}