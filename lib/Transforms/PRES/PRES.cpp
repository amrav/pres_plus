#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "model.cpp"
#include <cstdlib>
#include <ctime>
#include "llvm/IR/Constants.h"

using namespace llvm;
using namespace std;

#define DEBUG_TYPE "PRES"

STATISTIC(PRESCounter, "Counts number of functions greeted");

namespace {
  // PRES - The second implementation with getAnalysisUsage implemented.
  struct PRES : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    PRES() : FunctionPass(ID) {
      srand(time(NULL));
    }

    bool runOnFunction(Function &F) override {
      ++PRESCounter;
      Subnet pres_plus(errs());
      map<string, Instruction*> variable_to_inst;
      for (Function::iterator bb = F.begin(), e = F.end(); bb != e; ++bb) {
        errs() << "Basic block (name=" << bb->getName() << ") has "
             << bb->size() << " instr.\n";

        // blk is a pointer to a BasicBlock instance
        for (BasicBlock::iterator i = bb->begin(), e = bb->end(); i != e; ++i) {
          // The next statement works since operator<<(ostream&,...)
          // is overloaded for Instruction&
          errs() << *i << "\n";
          string opcodename = i->getOpcodeName(i->getOpcode());

          if (isa<AllocaInst>(i)) {

            auto alloc_i = dyn_cast<AllocaInst>(i);

            auto net = new Subnet(errs());
            int p1 = net->add_place();
            int p2 = net->add_place(i->getName());
            int t1 = net->add_transition("alloca");
            if (alloc_i->isArrayAllocation()) {
              errs() << "Is array alloc!\n";
              ConstantInt *ci = dyn_cast<ConstantInt>(alloc_i->getArraySize());
              net->transitions[t1]->tf.op1 = ci->getValue().toString(10, true);
            } else {
              net->transitions[t1]->tf.op1 = "1";
            }
            net->add_arc_p2t(p1, t1);
            net->add_arc_t2p(t1, p2);
            net->print(errs());

            errs() << "\n";
            pres_plus.attachSubnet(*net);

            errs() << "\nPRES+ model so far: \n";
            pres_plus.print(errs());
            errs() << "\n";
          }

          else if (isa<StoreInst>(i)) {

            auto store_i = dyn_cast<StoreInst>(i);

            auto net = new Subnet(errs());
            int p2 = net->add_place(store_i->getPointerOperand()->getName());
            int t1 = net->add_transition("store");

            if (isa<ConstantInt>(store_i->getValueOperand())) {
              auto ci = dyn_cast<ConstantInt>(store_i->getValueOperand());
              net->transitions[t1]->tf.op1 = ci->getValue().toString(10, true);
            } else {
              int p1 = net->add_place(i->getOperand(0)->getName());
              net->add_arc_p2t(p1, t1);
            }

            net->add_arc_t2p(t1, p2);
            net->print(errs());

            errs() << "\n";
            pres_plus.attachSubnet(*net);

            errs() << "\nPRES+ model so far: \n";
            pres_plus.print(errs());
            errs() << "\n";

            string var_name = i->getOperand(0)->getName();

            /*errs() << "num_operators: " << i->getNumOperands() << "\n";
            errs() << "var_name: " << var_name << "\n";
            variable_to_inst[var_name] = i->clone();*/
          }

          else if (isa<GetElementPtrInst>(i)) {

            auto gep_i = dyn_cast<GetElementPtrInst>(i);

            auto net = new Subnet(errs());
            int p1 = net->add_place(i->getOperand(0)->getName());
            int p2 = net->add_place(i->getName());
            int t1 = net->add_transition("getelementptr");
            auto ci = dyn_cast<ConstantInt>((gep_i->idx_begin() + 1)->get());
            net->transitions[t1]->tf.op1 = ci->getValue().toString(10, true);
            net->add_arc_p2t(p1, t1);
            net->add_arc_t2p(t1, p2);
            net->print(errs());

            errs() << "\n";
            pres_plus.attachSubnet(*net);

            errs() << "\nPRES+ model so far: \n";
            pres_plus.print(errs());
            errs() << "\n";

          }

          else if (opcodename == "load") {

            auto net = new Subnet(errs());
            int p1 = net->add_place(i->getOperand(0)->getName());
            int p2 = net->add_place(i->getName());
            int t1 = net->add_transition("load");
            net->add_arc_p2t(p1, t1);
            net->add_arc_t2p(t1, p2);
            net->print(errs());

            errs() << "\n";
            pres_plus.attachSubnet(*net);

            errs() << "\nPRES+ model so far: \n";
            pres_plus.print(errs());
            errs() << "\n";


            /*errs() << "num_operands: " << i->getNumOperands();
            errs() << "operands:" << i->getOperand(0)->getName() << " | " << i->getOperand(1)->getName() << "\n";
            bool same_inst = isa<Instruction>(*i->getOperand(0)) && cast<Instruction>(*i->getOperand(0)).isIdenticalTo(variable_to_inst["x"]);
            if (same_inst) {
              errs() << "same inst as x: true" << "\n";
              }*/
          }

          else if (opcodename == "add" || opcodename == "sub") {

            auto net = new Subnet(errs());
            int p1 = net->add_place(i->getName());
            int t1 = net->add_transition(opcodename);
            net->add_arc_t2p(t1, p1);

            if (i->getOperand(0)->hasName()) {
              int p2 = net->add_place(i->getOperand(0)->getName());
              net->add_arc_p2t(p2, t1);
            } else {
              auto ci = dyn_cast<ConstantInt>(i->getOperand(0));
              net->transitions[t1]->tf.op1 = ci->getValue().toString(10, true);
            }

            if (i->getOperand(1)->hasName()) {
              int p2 = net->add_place(i->getOperand(1)->getName());
              net->add_arc_p2t(p2, t1);
            } else {
              auto ci = dyn_cast<ConstantInt>(i->getOperand(1));
              net->transitions[t1]->tf.op2 = ci->getValue().toString(10, true);
            }

            net->print(errs());

            errs() << "\n";
            pres_plus.attachSubnet(*net);

            errs() << "\nPRES+ model so far: \n";
            pres_plus.print(errs());
            errs() << "\n";

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
