#include <iostream>
#include <string>

void runExperiments(
    const std::string &outputFile,
    const std::string &fastaFile = "");

int main(
    int argc,
    char *argv[])
{

    std::string fastaFile = "";
    std::string outputFile = "results.txt";

    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "--file")
        {
            fastaFile = argv[++i];
        }
        else if (arg == "--out")
        {
            outputFile = argv[++i];
        }
    }

    runExperiments(
        outputFile,
        fastaFile);

    std::cout
        << "Experiments completed.\n";

    return 0;
}
