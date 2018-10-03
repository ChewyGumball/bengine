// Benchmark.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <hayai.hpp>

#include <Core/Algorithms/Strings.h>

BENCHMARK(StringParsing, StringToIntCustom, 10, 1000)
{
    Core::Algorithms::String::ParseInt64("63987987");
}
BENCHMARK(StringParsing, StringToFloatCustom, 10, 1000)
{
    Core::Algorithms::String::ParseFloat("-838563.91887687668795965");
}

BENCHMARK(StringParsing, StringToIntSTD, 10, 1000)
{
    std::stoll("63987987");
}
BENCHMARK(StringParsing, StringToFloatSTD, 10, 1000)
{
    std::stof("-838563.91887687668795965");
}

int main()
{
    hayai::ConsoleOutputter consoleOutputter;

    hayai::Benchmarker::AddOutputter(consoleOutputter);
    hayai::Benchmarker::RunAllTests();
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
