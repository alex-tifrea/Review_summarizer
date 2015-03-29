#include <iostream>
#include "IO.h"
#include "Worker.h"
#include "InterogateNGRAM.h"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <signal.h>
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

void signal_callback_handler(int signum)
{
    std::cout<<"Caught signal "<<signum;
    std::cout<<"\nExiting ..."<<std::endl;

    // Cleanup and close up stuff here
    InterogateNGRAM::Finalize();

    // Terminate program
    exit(signum);
}

int main ()
{
    InterogateNGRAM::Init();
    // Register signal and signal handler
    signal(SIGINT, signal_callback_handler);

    string _in_name = "input_example";
    string _out_name = "output_example";
    IO *init_io = new IO(_in_name, _out_name);
    Worker *work = new Worker(init_io);
    work->init();
    work->initBigrams();
    work->printNgrams(std::cout);
//     work->generateInteractiveLoop();
    work->generateLoop();
    work->printNgrams(std::cout);
    InterogateNGRAM::Finalize();
    return 0;
}
