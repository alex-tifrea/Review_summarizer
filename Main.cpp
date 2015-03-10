#include <iostream>
#include "IO.h"
#include "Worker.h"
#include "Interogate.h"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
using namespace std;

template <typename T1, typename T2>
struct comp_frequencies
{
    typedef pair <T1, T2> type;
    bool operator ()(type const& a, type const& b) const
    {
        return a.second > b.second;
    }
};

int main ()
{
    Interogate::Init();
    string _in_name = "input_example";
    string _out_name = "output_example";
    IO *init_io = new IO(_in_name, _out_name);
    Worker *work = new Worker(init_io);
    work->init();
    work->initBigrams();
    work->printNgrams();
    work->generateInteractiveLoop();
    work->printNgrams();
    Interogate::Finalize();
    return 0;
}
