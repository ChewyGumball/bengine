// Benchmark.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <hayai/hayai.hpp>

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
