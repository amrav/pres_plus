#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <map>
#include <utility>
#include "llvm/Support/raw_ostream.h"

using namespace std;

// Domain D_p is Z, set of integers
struct token {
  int value;
};

struct Place {
  string var_name;
  vector<token> tokens;
  vector<int> pre_set, post_set;
  int id;
  void print(llvm::raw_ostream& out) {
    out << "Place: " << id << "\n";
    out << "pre_set: ";
    for (auto t: pre_set) {
      out << t << " ";
    }
    out << "\n post_set: ";
    for (auto t: post_set) {
      out << t << " ";
    }
    out << "\n variable: " << var_name << "\n";
  }
  Place() {
    id = rand();
  }
};

struct TransitionFunction {
  string operator_, op1, op2;
  void print(llvm::raw_ostream& out) {
    out << "operator: " << operator_ << " op1: " << op1 << " op2: " << op2 << "\n";
  }
};

struct Transition {
  vector<int> pre_set;
  vector<int> post_set;
  int id;
  TransitionFunction tf;
  void print(llvm::raw_ostream& out) {
    out << "Transition: " << id << "\n";
    out << "pre_set: ";
    for (auto t: pre_set) {
      out << t << " ";
    }
    out << "\npost_set: ";
    for (auto t: post_set) {
      out << t << " ";
    }
    out << "\ntransition function: ";
    tf.print(out);
  }
  Transition() {
    id = rand();
  }
};

struct Subnet {
  set<int> out_ports;
  set<int> in_ports;
  map<int, Place*> places;
  map<int, Transition*> transitions;
  llvm::raw_ostream &out;
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
  void print(llvm::raw_ostream& out) {
    out << "Subnet: " << "\n";
    for (auto p: places) {
      p.second->print(out);
    }
    for (auto t: transitions) {
      t.second->print(out);
    }
  }

  Subnet(llvm::raw_ostream& out) : out(out) {}

  int add_place(string var_name="") {
    auto place = new Place();
    place->var_name = var_name;
    places[place->id] = place;
    in_ports.insert(place->id);
    out_ports.insert(place->id);
    return place->id;
  }

  int add_transition(string operator_) {
    auto transition = new Transition();
    transition->tf.operator_ = operator_;
    transitions[transition->id] = transition;
    return transition->id;
  }

  void add_arc_p2t(int place_id, int transition_id) {
    auto p = places[place_id];
    auto t = transitions[transition_id];
    p->post_set.push_back(t->id);
    t->pre_set.push_back(p->id);
    out_ports.erase(p->id);
  }

  void add_arc_t2p(int t_id, int p_id) {
    auto p = places[p_id];
    auto t = transitions[t_id];
    t->post_set.push_back(p_id);
    p->pre_set.push_back(t_id);
    in_ports.erase(p_id);
  }

  void make_consistent() {
    vector <int> in_p, out_p;
    for (auto i : in_ports) {
      if (places[i]->pre_set.size() != 0) {
        in_p.push_back(i);
      }
    }

    for (auto p : in_p) {
      in_ports.erase(p);
    }

    for (auto o : out_ports) {
      out << "Processing out port: " << o << "\n";
      if (places[o]->post_set.size() != 0) {
        out_p.push_back(o);
      }
    }
    for (auto p : out_p) {
      out_ports.erase(p);
    }

    out << "Done erasing faux out ports";


    vector<int> empty_places;
    for (auto p : places) {
      if (p.second->pre_set.size() == 0 && p.second->post_set.size() == 0) {
        empty_places.push_back(p.first);
      }
    }
    for (auto p : empty_places) {
      places.erase(p);
      out_ports.erase(p);
      in_ports.erase(p);
    }

  }

  void attachSubnet(Subnet& net) {
    vector<pair<int, int> > merged_places;

    for (auto i: net.in_ports) {
      for (auto o: out_ports)
      if (places[o]->var_name != "" &&
          places[o]->var_name == net.places[i]->var_name) {
        out << "Found matching out and in: " << o << " " << i << "\n";
        auto in_port = net.places[i];
        for (auto t: in_port->post_set) {
          auto post_trans = net.transitions[t];
          replace(post_trans->pre_set.begin(), post_trans->pre_set.end(), i, o);
          places[o]->post_set.push_back(t);
          merged_places.push_back(make_pair(o, i));
        }
      }
    }

    for (auto p : merged_places) {
      net.places.erase(p.second);
      net.in_ports.erase(p.second);
      out_ports.erase(p.first);
    }

    out << "Done merging outs and ins\n";

    for (auto p: places) {
      // get all intermediate places in first subnet
      if (out_ports.find(p.first) != out_ports.end())
        continue;
      if (in_ports.find(p.first) != in_ports.end())
        continue;
      auto place = p.second;
      for (auto i : net.in_ports) {
        if (place->var_name != "" &&
            place->var_name == net.places[i]->var_name) {
          out << "Found matching intermediates: " << p.first << " " << i << "\n";
          for (auto t : place->pre_set) {
            transitions[t]->post_set.push_back(i);
            net.places[i]->pre_set.push_back(t);
          }
        }
      }
    }

    for (auto p: net.places) {
      places[p.first] = p.second;
    }

    for (auto t: net.transitions) {
      transitions[t.first] = t.second;
    }

    for (auto p : net.in_ports) {
      in_ports.insert(p);
    }

    for (auto p : net.out_ports) {
      out_ports.insert(p);
    }

    out << "\nmaking consistent!\n";

    make_consistent();

  }
};
