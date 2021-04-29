#include <cstdlib>
#include <cstring>
#include <cctype>
#include <sstream>
#include <vector>
#include <iostream>
#include <map>
#include "calc.h"

struct Calc {
private:
  //fields
  std::map<std::string, int> variables;
  pthread_mutex_t lock;
  
private:
    // private member functions
    std::vector<std::string> tokenize(const std::string &expr) {
    std::vector<std::string> vec;
    std::stringstream s(expr);

    std::string tok;
    while (s >> tok) {
      vec.push_back(tok);
    }
    
    return vec;
    }

    // check if str is alphanumeric
    // return 0 if not
    // return 1 if alpha
    // return 2 if num
    int isAlphaNum(std::string str) {
      long unsigned int digit = 0, alpha = 0;
      if (str.c_str()[0] == '-') {
        digit++;
      }

      unsigned length = str.length();
      for (unsigned i = 0; i < length; i++) {
        if (std::isalnum(str.c_str()[i]) == 0 && str.c_str()[0] != '-') {
          return 0;
        }
        if (std::isalpha(str.c_str()[i]) != 0) {
          alpha++;
        }
        if (std::isdigit(str.c_str()[i]) != 0) {
          digit++;
        }
      }

      if (alpha == length) {
        return 1;
      } else if (digit >= length) {
        return 2;
      }
      return 3;
    }

    // return 0 if none
    // return 1 if =
    // return 2 if + - * /
    int isOp(std::string str, char &ch) {
      if (str.length() > 1) {
        return 0;
      }
      ch = str.c_str()[0];
      if (ch == '=') {
        return 1;
      }
      if (ch == '+' || ch == '-' || ch == '*' || ch == '/') {
        return 2;
      }
      return 0;
    }

    // return 0 if fail
    // return 1 if valid
    int math(std::string a, std::string b, char op, int &result) {
      int A = 0;
      int B = 0;
      // if alpha + in map
      
      if (isAlphaNum(a) == 1 && variables.find(a) != variables.end()) {
        A = variables.at(a);
      } else if (isAlphaNum(a) == 2) {
        A = std::stoi(a);
      }
      
      if (isAlphaNum(b) == 1 && variables.find(b) != variables.end()) {
        B	= variables.at(b);
      } else if (isAlphaNum(b) == 2) {
        B = std::stoi(b);
      }


      if (op == '+') {
        result = A + B;
        return 1;
      } else if (op == '-') {
        result = A - B;
        return 1;
      } else if (op == '*') {
        result = A * B;
        return 1;
      } else if (op == '/' && B != 0) {
        result = A / B;
        return 1;
      }
      return 0;
    }

public:
  // public member functions                                                  
  Calc() {
    pthread_mutex_init(&this->lock, NULL);
  }

  ~Calc() {
    variables.clear(); 
    pthread_mutex_destroy(&this->lock);
  }

  void unlock() {
    pthread_mutex_unlock(&this->lock);
  }

  void lock_() {
    pthread_mutex_lock(&this->lock);
  }

  // return 1 if success                                                        
  // return 0 is fail                                                           
  int evalExpr(const std::string &expr, int &result) {
    // split expression into an array of strings                               
    std::vector<std::string> str = tokenize(expr);
    if (str.size() == 0 || isAlphaNum(str.front()) == 0 || str.size() > 5) {
      return 0;
    }

    char ch;
    if (str.size() == 1) {
      // operand
      if (isAlphaNum(str.at(0)) == 1 && variables.find(str.at(0)) != variables.end()) {
        result = variables.at(str.at(0));
        return 1;
      } else if (isAlphaNum(str.at(0)) == 2) {
        result = std::stoi(str.at(0));
        return 1;
      }
      return 0;
      
    } else if (str.size() == 3) {      
      if (isOp(str.at(1), ch) == 2) {
        // operand op operand
        return math(str.at(0), str.at(2), ch, result);
	
      } else if (isOp(str.at(1), ch) == 1) {
	      //var = operand
	
        if (math(str.at(2), "0", '+',  result) == 1) {
          if (variables.find(str.at(0)) == variables.end()) {
            variables.insert({str.at(0), result});
          } else {
            variables.at(str.at(0)) = result;
          }
          return 1;
	      } 
      }
      
    } else if (str.size() == 5) {
      // var = operand op operand
      if (isOp(str.at(1), ch) != 1 || isOp(str.at(3), ch) != 2) {
	      return 0;
      }
      
      if (math(str.at(2), str.at(4), ch, result) == 1) {	
        if (variables.find(str.at(0)) == variables.end()) {
          variables.insert({str.at(0), result});
        } else {
          variables.at(str.at(0)) = result;
        }
          
        return 1;
      }
    }
    return 0;
  }
};


extern "C" struct Calc *calc_create(void) {
  return new Calc;;
}

extern "C" void calc_destroy(struct Calc *calc) {
  delete calc;
}

extern "C" int calc_eval(struct Calc *calc, const char *expr, int *result) {
  calc->lock_();
  int val = calc->evalExpr(expr, *result);
  calc->unlock();
  return val;
}
