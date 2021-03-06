#include <map>
#include <set>

#include "program_options.h"

enum OptionType { POSITIONAL = 0, NAMED, FLAG };

enum OptionRequired { REQUIRED = 0, OPTIONAL };

class OptionBase {
public:
    OptionRequired required;
    OptionType type;
    std::string valueName;
    std::string description;

    virtual void parse(const std::string &arg) const = 0;

    OptionBase(OptionRequired required, OptionType type, std::string valueName, std::string description)
        : required(required), type(type), valueName(valueName), description(description) {}
};

template <typename T>
class Option : public OptionBase {
private:
    T *dest;

public:
    Option(OptionRequired required, OptionType type, std::string valueName, std::string description, T *dest)
        : OptionBase(required, type, valueName, description), dest(dest) {}

    void parse(const std::string &arg) const;
};

template <>
void Option<std::string>::parse(const std::string &arg) const {
    *dest = arg;
}

template <>
void Option<int>::parse(const std::string &arg) const {
    *dest = std::atoi(arg.c_str());
}

template <>
void Option<bool>::parse(const std::string &arg) const {
    *dest = true;
}

template <>
void Option<double>::parse(const std::string &arg) const {
    *dest = std::atof(arg.c_str());
}

void printUsage() { std::cout << "Usage" << std::endl; }

ProgramOptions ProgramOptions::fromCommandLine(int argc, char *argv[]) {
    std::string sparseMatrixFile;
    int denseMatrixSeed;
    int replicationGroupSize;
    int multiplicationExponent;
    bool useInnerAlgorithm = false;
    bool printMatrix = false;
    bool printGreaterEqual = false;
    double printGreaterEqualValue;
    bool printStats = false;

    const std::map<std::string, OptionBase *> supportedOptions{
        {"-f", new Option<std::string>(REQUIRED, NAMED, "sparse_matrix_file", "", &sparseMatrixFile)},
        {"-s", new Option<int>(REQUIRED, NAMED, "seed_for_dense_matrix", "", &denseMatrixSeed)},
        {"-c", new Option<int>(REQUIRED, NAMED, "repl_group_size", "", &replicationGroupSize)},
        {"-e", new Option<int>(REQUIRED, NAMED, "exponent", "", &multiplicationExponent)},
        {"-g", new Option<double>(OPTIONAL, NAMED, "ge_value", "", &printGreaterEqualValue)},
        {"-v", new Option<bool>(OPTIONAL, FLAG, "", "", &printMatrix)},
        {"-i", new Option<bool>(OPTIONAL, FLAG, "", "", &useInnerAlgorithm)},
        {"-p", new Option<bool>(OPTIONAL, FLAG, "", "", &printStats)},
    };

    std::set<std::string> foundOptions;
    for (int i = 1; i < argc; i++) {
        std::string optionName = argv[i];
        auto optIt = supportedOptions.find(optionName);
        if (optIt != supportedOptions.end()) {
            if (foundOptions.find(optionName) != foundOptions.end()) {
                std::cout << "Duplicated option: " << optionName << std::endl;
                printUsage();
                exit(1);
            } else {
                foundOptions.insert(optionName);

                if (optIt->second->type == FLAG) {
                    optIt->second->parse("");
                } else {
                    optIt->second->parse(argv[++i]);
                }
            }
        } else {
            std::cout << "Unrecognized option: " << optionName << std::endl;
            printUsage();
            exit(1);
        }
    }

    for (auto it : supportedOptions) {
        if (it.second->required == REQUIRED && foundOptions.find(it.first) == foundOptions.end()) {
            std::cout << "Missing required option: " << it.first << std::endl;
            printUsage();
            exit(1);
        }
    }

    if (foundOptions.find("-g") != foundOptions.end()) {
        printGreaterEqual = true;
    }

    return ProgramOptions(sparseMatrixFile, denseMatrixSeed, replicationGroupSize, multiplicationExponent,
                          useInnerAlgorithm ? Algorithm::InnerABC : Algorithm::ColumnA, printMatrix, printGreaterEqual,
                          printGreaterEqualValue, printStats);
}

std::ostream &operator<<(std::ostream &os, ProgramOptions po) {
    os << "sparseMatrixFile: " << po.sparseMatrixFile << std::endl;
    os << "denseMatrixSeed: " << po.denseMatrixSeed << std::endl;
    os << "replicationGroupSize: " << po.replicationGroupSize << std::endl;
    os << "multiplicationExponent: " << po.multiplicationExponent << std::endl;
    os << "algorithm: " << po.algorithm << std::endl;
    os << "printMatrix: " << std::string(po.printMatrix ? "True" : "False") << std::endl;
    os << "printGreaterEqual: " << std::string(po.printGreaterEqual ? "True" : "False") << std::endl;
    os << "printGreaterEqualValue: " << po.printGreaterEqualValue << std::endl;
    return os;
}