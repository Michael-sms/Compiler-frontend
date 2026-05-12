#include <iostream>
#include <string>

#include "parser/slr_parser_a.h"

int main(int argc, char** argv) {
    std::string output_dir = "./parser_a_output";
    if (argc >= 2) {
        output_dir = argv[1];
    }

    parser_a::SLRParserABuilder builder(parser_a::BuildDefaultCMinusGrammar(), "Program");
    builder.build();
    builder.dumpAll(output_dir);

    std::cout << "Parser A build finished.\n";
    std::cout << "Output dir: " << output_dir << "\n";
    std::cout << "Productions: " << builder.productions().size() - 1 << " (excluding augmented)\n";
    std::cout << "LR(0) states: " << builder.canonicalCollection().size() << "\n";
    std::cout << "Action entries: " << builder.actionTable().size() << "\n";
    std::cout << "Goto entries: " << builder.gotoTable().size() << "\n";
    std::cout << "Conflicts: " << builder.conflicts().size() << "\n";
    return 0;
}

