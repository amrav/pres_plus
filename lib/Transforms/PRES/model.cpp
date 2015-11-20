#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <iostream>

using namespace std;

// Domain D_p is Z, set of integers
struct token {
  int value;
};

struct subnet;

struct place {
  string var_name;
  vector<token> tokens;
  vector<int> pre_set, post_set;
  int index;
  void print() {
    cout << "Place: " << index << endl;
    cout << "pre_set: ";
    for (auto t: pre_set) {
      cout << t << " ";
    }
    cout << "\n post_set: ";
    for (auto t: post_set) {
      cout << t << " ";
    }
    cout << endl;
  }
};

struct transition_function {
  string operand, op1, op2;
  void print() {
    cout << "operand: " << operand << " op1: " << op1 << " op2: " << op2 << endl;
  }
};

struct transition {
  vector<int> pre_set;
  vector<int> post_set;
  int index;
  void print() {
    cout << "Transition: " << index << endl;
    cout << "pre_set: ";
    for (auto t: pre_set) {
      cout << t << " ";
    }
    cout << "\n post_set: ";
    for (auto t: post_set) {
      cout << t << " ";
    }
    cout << endl;
  }
};

struct subnet {
  vector<place> places;
  vector<transition> transitions;
  set<place> out_ports;
  set<place> in_ports;
  /*void merge(subnet &subnet2) {
    for (auto o: out_ports) {
      for (auto i: subnet2.in_ports) {
        if (places[o].var_name == subnet2.places[i].var_name) {
          // update post transitions of in port to have out port as pre place
          for (auto t: subnet2.places[i].post_set) {
            auto tran = subnet2.transitions[t];
            std::replace(tran.pre_set.begin(), tran.pre_set.end(), i, o);
          }
        }
      }
    }
    for (auto p: places) {
      if (std::find(out_ports.begin(), out_ports.end(), p.index) != out_ports.end()) {

      }
    }
    }*/
  void print() {
    cout << "Subnet: " << endl;
    for (auto p: places) {
      p.print();
    }
    for (auto t: transitions) {
      t.print();
    }
  }
};
