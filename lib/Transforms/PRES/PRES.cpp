#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace std;

#define DEBUG_TYPE "PRES"

STATISTIC(PRESCounter, "Counts number of functions greeted");

namespace {
  // PRES - The second implementation with getAnalysisUsage implemented.
  struct PRES : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    PRES() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
      ++PRESCounter;
      map<string, Instruction*> variable_to_inst;
      map<string, string> stack_to_variable;
      for (Function::iterator bb = F.begin(), e = F.end(); bb != e; ++bb) {
        errs() << "Basic block (name=" << bb->getName() << ") has "
             << bb->size() << " instructions.\n";

        // blk is a pointer to a BasicBlock instance
        for (BasicBlock::iterator i = bb->begin(), e = bb->end(); i != e; ++i) {
          // The next statement works since operator<<(ostream&,...)
          // is overloaded for Instruction&
          errs() << *i << "\n";
          errs() << i->getOpcodeName(i->getOpcode()) << "\n";
          string opcodename = i->getOpcodeName(i->getOpcode());
          if (opcodename == "load") {
            string var_name = i->getOperand(0)->getName();
            errs() << "num_operators " << i->getNumOperands() << "\n";
            errs() << "var_name: " << var_name << "\n";
            variable_to_inst[var_name] = i->clone();
          }
          if (opcodename == "store") {
            errs() << "num_operands: " << i->getNumOperands();
            errs() << "operands:" << i->getOperand(0)->getName() << " | " << i->getOperand(1)->getName() << "\n";
            bool same_inst = isa<Instruction>(*i->getOperand(0)) && cast<Instruction>(*i->getOperand(0)).isIdenticalTo(variable_to_inst["x"]);
            if (same_inst) {
              errs() << "same inst as x: true" << "\n";
            }
          }
        }

      }
      return false;
    }

    // We don't modify the program, so we preserve all analyses.
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }
  };
}

char PRES::ID = 0;
static RegisterPass<PRES>
Y("PRES", "PRES+ Pass (with getAnalysisUsage implemented)");
