// this code was written by team member: Dina Janđel
#include <iostream>
#include <string>

/*
    declaration of function runExperiments()
    this function runs tests of the implemented Bamboo filter and saves results to a file

    parameters:
    outputFile -> name of the output file where results are saved
    fastaFile  -> optional FASTA file with DNA sequences

    if fastaFile is not provided:
    a randomly generated DNA sequence is used
*/
void runExperiments(
    const std::string &outputFile,
    const std::string &fastaFile = "");

int main(
    int argc,
    char *argv[])
{
    /*
        example call:
        ./bamboo_project --file test.fasta --out results.txt
        argc: 5
        argv:
        argv[0] -> program name
        argv[1] -> "--file"
        argv[2] -> "test.fasta"
        argv[3] -> "--out"
        argv[4] -> "results.txt"
    */

    // by default there is no FASTA file
    std::string fastaFile = "";

    // default output file name (if not provided via command line arguments)
    std::string outputFile = "results.txt";

    // iterate through all command line arguments
    for (int i = 1; i < argc; i++)
    {
        // convert current argument to string
        std::string arg = argv[i];

        // if user writes --file, store FASTA filename
        if (arg == "--file")
        {
            // argv[++i] takes the next argument
            fastaFile = argv[++i];
        }

        // if user writes --out, store output file name
        else if (arg == "--out")
        {
            outputFile = argv[++i];
        }
    }

    /*
        run experiments on the Bamboo filter

        if FASTA file exists:
        -> use real DNA data

        if not:
        -> use randomly generated DNA sequences
    */
    runExperiments(
        outputFile,
        fastaFile);

    // message after program finishes that experiments are completed and results are saved
    std::cout
        << "experiments completed.\n";

    return 0;
}