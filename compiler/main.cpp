#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "antlr4-runtime.h"
#include "generated/ifccLexer.h"
#include "generated/ifccParser.h"
#include "generated/ifccBaseVisitor.h"

#include "CodeGenVisitor.h"
#include "SymbolTableVisitor.h"
#include "IRGenVisitor.h"

using namespace antlr4;
using namespace std;

int main(int argn, const char **argv)
{
  stringstream in;
  if (argn == 2)
  {
    ifstream lecture(argv[1]);
    if (!lecture.good())
    {
      cerr << "error: cannot read file: " << argv[1] << endl;
      exit(1);
    }
    in << lecture.rdbuf();
  }
  else
  {
    cerr << "usage: ifcc path/to/file.c" << endl;
    exit(1);
  }

  ANTLRInputStream input(in.str());

  ifccLexer lexer(&input);
  CommonTokenStream tokens(&lexer);

  tokens.fill();

  ifccParser parser(&tokens);
  tree::ParseTree *tree = parser.axiom();

  if (parser.getNumberOfSyntaxErrors() != 0)
  {
    cerr << "error: syntax error during parsing" << endl;
    exit(1);
  }

  ifccParser::AxiomContext *axiom = dynamic_cast<ifccParser::AxiomContext *>(tree);
  std::map<std::string, FunctionSignature> functionTable;

  for (auto prog : axiom->prog())
  {
    std::string fname = prog->ID()->getText();

    SymbolTableVisitor stv;
    stv.functionTable = &functionTable;
    stv.visit(prog);

    if (stv.error == 0)
    {
      DefFonction defFunc(fname, {}); // tu peux gérer les params plus tard
      CFG cfg(&defFunc, stv);
      IRGenVisitor cgv;
      cgv.cfg = &cfg;
      cgv.functionTable = stv.functionTable;
      (*cgv.functionTable)["putchar"] = FunctionSignature{"int", {"int"}};
      (*cgv.functionTable)["getchar"] = FunctionSignature{"int", {}};
      cgv.visit(prog);

      std::cerr << "Function: " << fname << "\n";
      // stv.print_symbol_table();
      // cfg.current_bb->print_instrs();
      cfg.gen_asm(std::cout);
    }
  }

  return 0;
}
