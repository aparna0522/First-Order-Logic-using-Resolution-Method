#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
using namespace std;

#define checkIfVariable(x) (x[0] >= 'a' && x[0] <= 'z')

set<string> myset;

class Tree {
  string value;
  Tree *leftChild;
  Tree *rightChild;

 public:
  Tree(vector<string> &val, int start, int end) {
    int splitIndex = -1;

    // Find the highest precedence operator from the given val vector, save in
    // splitIndex
    for (int i = start; i < end; i++) {
      if (splitIndex == -1) {
        splitIndex = i;
      } else if (val[splitIndex] == "|") {
        if (val[i] == "=>") splitIndex = i;
      } else if (val[splitIndex] == "&") {
        if (val[i] == "|")
          splitIndex = i;
        else if (val[i] == "=>")
          splitIndex = i;
      } else if (val[splitIndex] != "|" && val[splitIndex] != "=>" &&
                 val[splitIndex] != "&") {
        if (val[i] == "|" || val[i] == "=>" || val[i] == "&") splitIndex = i;
      }
    }

    // Use the splitIndex to create the Tree
    /*
    For A|B=>C
           =>
         /   \
        |      C
      /   \
     A     B
    */
    if (val[splitIndex] != "|" && val[splitIndex] != "=>" &&
        val[splitIndex] != "&") {
      value = val[splitIndex];
      leftChild = NULL;
      rightChild = NULL;
    } else {
      value = val[splitIndex];
      leftChild = new Tree(val, start, splitIndex);
      rightChild = new Tree(val, splitIndex + 1, end);
    }
  }

  Tree(string val, Tree *left, Tree *right) {
    value = val;
    leftChild = left;
    rightChild = right;
  }

  void Negation() {
    if (value == "|")
      value = "&";
    else if (value == "&")
      value = "|";
    else {
      if (value[0] == '~')
        value = value.substr(1, value.size() - 1);
      else
        value = "~" + value;
    }
    if (leftChild) leftChild->Negation();

    if (rightChild) rightChild->Negation();
  }

  vector<Tree *> DistributiveProp() {
    vector<Tree *> left;
    vector<Tree *> right;

    /*
    A|B=>C
    */
    if (leftChild) left = leftChild->DistributiveProp();
    if (rightChild) right = rightChild->DistributiveProp();

    if (value == "&") {
      vector<Tree *> answer;
      for (auto x : left) {
        answer.push_back(x);
      }
      for (auto x : right) {
        answer.push_back(x);
      }
      return answer;
    } else if (value == "|") {
      vector<Tree *> answer;
      for (auto x : left) {
        for (auto y : right) {
          Tree *resultantTree = new Tree("|", x, y);
          answer.push_back(resultantTree);
        }
      }
      return answer;
    } else {
      return {this};
    }
  }

  void conversionToCNF() {
    if (value == "=>") {
      value = "|";
      leftChild->Negation();
    }
  }

  vector<string> finalAnswer() {
    if (leftChild == NULL && rightChild == NULL) return {value};
    vector<string> left;
    vector<string> right;
    if (leftChild) left = leftChild->finalAnswer();
    if (rightChild) right = rightChild->finalAnswer();
    vector<string> answer;
    for (auto &x : left) answer.push_back(x);
    for (auto &x : right) answer.push_back(x);
    return answer;
  }

  void display() {
    if (leftChild) leftChild->display();
    cout << value << " ";
    if (rightChild) rightChild->display();
  }
};

class UnionFind {
 public:
  map<string, string> parentCategory;

  void Include(string A) { parentCategory[A] = A; }
  string Find(string A) {
    if (parentCategory[A] == A)
      return A;
    else
      return Find(parentCategory[A]);
  }
  void Union(string &A, string &B) {
    parentCategory[Find(A)] = parentCategory[Find(B)];
  }
};

bool areComplements(string &literal, string &query) {
  string temp = "";
  if (literal[0] == '~') {
    temp = literal.substr(1, literal.size() - 1);
    int i;
    for (i = 0; i < temp.size(); i++) {
      if (i >= query.size()) return false;
      if (temp[i] == '(') break;
      if (temp[i] != query[i]) return false;
    }
    return query[i] == '(';
  } else if (query[0] == '~') {
    temp = query.substr(1, query.size() - 1);
    int i;
    for (i = 0; i < temp.size(); i++) {
      if (i >= literal.size()) return false;
      if (temp[i] == '(') break;
      if (temp[i] != literal[i]) return false;
    }
    return literal[i] == '(';
  } else
    return false;
}

bool areConstantComplements(string &literal, string &query) {
  string temp = "";
  if (literal[0] == '~') {
    temp = literal.substr(1, literal.size() - 1);
    return temp == query;
  } else if (query[0] == '~') {
    temp = query.substr(1, query.size() - 1);
    return temp == literal;
  } else
    return false;
}

bool isConstant(vector<string> &sentence) {
  for (auto &predicate : sentence) {
    for (int i = 0; i < predicate.size(); i++) {
      if (predicate[i] == '(' || predicate[i] == ',') {
        if (predicate[i + 1] >= 'a' && predicate[i + 1] <= 'z') return false;
      }
    }
  }
  return true;
}

bool unify(string &A, string &B, map<string, string> &theta) {
  UnionFind ufo;
  string term = "";
  vector<string> paramsA, paramsB;
  for (int i = 0; i < A.size(); i++) {
    if (A[i] == '(')
      term = "";
    else if (A[i] == ',') {
      if (checkIfVariable(term))
        ufo.Include(term);
      else
        ufo.Include(term);
      paramsA.push_back(term);
      term = "";
    } else if (A[i] == ')') {
      if (checkIfVariable(term))
        ufo.Include(term);
      else
        ufo.Include(term);
      paramsA.push_back(term);
      term = "";
    } else {
      term += A[i];
    }
  }

  term = "";

  for (int i = 0; i < B.size(); i++) {
    if (B[i] == '(')
      term = "";
    else if (B[i] == ',') {
      if (checkIfVariable(term))
        ufo.Include(term + "2");
      else
        ufo.Include(term);
      paramsB.push_back(term);
      term = "";
    } else if (B[i] == ')') {
      if (checkIfVariable(term))
        ufo.Include(term + "2");
      else
        ufo.Include(term);
      paramsB.push_back(term);
      term = "";
    } else {
      term += B[i];
    }
  }

  for (int i = 0; i < paramsA.size(); i++) {
    string catA, catB;
    catA = ufo.Find(paramsA[i]);

    if (checkIfVariable(paramsB[i]))
      catB = ufo.Find(paramsB[i] + "2");
    else
      catB = ufo.Find(paramsB[i]);

    if (checkIfVariable(catA)) {
      if (checkIfVariable(catB)) {
        ufo.Union(catB, catA);
      } else {
        ufo.Union(catA, catB);
      }
    } else {
      if (checkIfVariable(catB)) {
        ufo.Union(catB, catA);
      } else {
        if (catA != catB) return false;
      }
    }
  }

  for (int i = 0; i < paramsA.size(); i++) {
    if (checkIfVariable(paramsA[i])) theta[paramsA[i]] = ufo.Find(paramsA[i]);
    if (checkIfVariable(paramsB[i]))
      theta[paramsB[i] + "2"] = ufo.Find(paramsB[i] + "2");
  }

  return true;
}

bool checkIn(vector<string> &sentence, vector<vector<string>> &KB) {
  for (int i = 0; i < KB.size(); i++) {
    int occurs = 0;
    for (int j = 0; j < KB[i].size(); j++) {
      for (int k = 0; k < sentence.size(); k++) {
        if (sentence[k] == KB[i][j]) {
          occurs++;
          break;
        }
      }
    }
    if (occurs == KB[i].size()) return true;
  }
  return false;
}

void simplification(vector<string> &sentence) {
  bool simplify = true;
  while (simplify) {
    simplify = false;
    for (int i = 0; i < sentence.size(); i++) {
      for (int j = i + 1; j < sentence.size(); j++) {
        if (areComplements(sentence[i], sentence[j])) {
          map<string, string> theta;
          if (unify(sentence[i], sentence[j], theta)) {
            bool consistent = true;
            for (auto &entry : theta) {
              string var = entry.first;
              string complement = var[var.size() - 1] == '2'
                                      ? var.substr(0, var.size() - 1)
                                      : var + "2";
              if (theta.find(complement) != theta.end() &&
                  theta[complement] != entry.second) {
                consistent = false;
                break;
              }
            }
            if (!consistent) continue;

            set<string> negatedPresent;
            simplify = true;
            for (int k = 0; k < sentence.size(); k++) {
              string temp = "", finalString = "";
              for (int l = 0; l < sentence[k].size(); l++) {
                if (sentence[k][l] == '(') {
                  finalString += temp + "(";
                  temp = "";
                } else if (sentence[k][l] == ',') {
                  if (theta.find(temp) != theta.end())
                    finalString += theta[temp] + ",";
                  else if (theta.find(temp + "2") != theta.end())
                    finalString += theta[temp + "2"] + ",";
                  else
                    finalString += temp + ",";
                  temp = "";
                } else if (sentence[k][l] == ')') {
                  if (theta.find(temp) != theta.end())
                    finalString += theta[temp] + ")";
                  else if (theta.find(temp + "2") != theta.end())
                    finalString += theta[temp + "2"] + ")";
                  else
                    finalString += temp + ")";
                  temp = "";
                } else {
                  temp += sentence[k][l];
                }
              }
              string finalNegated =
                  finalString[0] == '~'
                      ? finalString.substr(1, finalString.size() - 1)
                      : "~" + finalString;
              if (negatedPresent.find(finalNegated) != negatedPresent.end())
                negatedPresent.erase(finalNegated);
              else
                negatedPresent.insert(finalString);
            }
            sentence.clear();
            for (auto i : negatedPresent) {
              sentence.push_back(i);
            }
            break;
          }
        }
      }
      if (simplify) break;
    }
  }
  return;
}

vector<string> substitute(vector<string> &sentence, vector<string> &queries,
                          map<string, string> &theta) {
  vector<string> A;
  for (int i = 0; i < sentence.size(); i++) {
    string substitute = "";
    string temp = "";
    for (int j = 0; j < sentence[i].size(); j++) {
      if (sentence[i][j] == '(') {
        substitute += temp + "(";
        temp = "";
      } else if (sentence[i][j] == ',') {
        if (theta.find(temp) != theta.end())
          substitute += theta[temp] + ",";
        else
          substitute += temp + ",";
        temp = "";
      } else if (sentence[i][j] == ')') {
        if (theta.find(temp) != theta.end())
          substitute += theta[temp] + ")";
        else
          substitute += temp + ")";
        temp = "";
      } else {
        temp += sentence[i][j];
      }
    }
    A.push_back(substitute);
  }

  vector<string> B;
  for (int i = 0; i < queries.size(); i++) {
    string substitute = "";
    string temp = "";
    for (int j = 0; j < queries[i].size(); j++) {
      if (queries[i][j] == '(') {
        substitute += temp + "(";
        temp = "";
      } else if (queries[i][j] == ',') {
        if (theta.find(temp + "2") != theta.end())
          substitute += theta[temp + "2"] + ",";
        else
          substitute += temp + ",";
        temp = "";
      } else if (queries[i][j] == ')') {
        if (theta.find(temp + "2") != theta.end())
          substitute += theta[temp + "2"] + ")";
        else
          substitute += temp + ")";
        temp = "";
      } else {
        temp += queries[i][j];
      }
    }
    B.push_back(substitute);
  }

  set<string> temp;
  for (auto &x : A) {
    string orig = x;
    string n_orig = x[0] == '~' ? orig.substr(1, orig.size() - 1) : "~" + orig;
    if (temp.count(orig) > 0)
      continue;
    else if (temp.count(n_orig) > 0)
      temp.erase(n_orig);
    else
      temp.insert(orig);
  }
  for (auto &y : B) {
    string orig = y;
    string n_orig = y[0] == '~' ? orig.substr(1, orig.size() - 1) : "~" + orig;
    if (temp.count(orig) > 0)
      continue;
    else if (temp.count(n_orig) > 0)
      temp.erase(n_orig);
    else
      temp.insert(orig);
  }

  vector<string> resolutionAnswer;
  for (auto &x : temp) {
    resolutionAnswer.push_back(x);
  }

  return resolutionAnswer;
}

void unitResolve(vector<string> &sentence, vector<vector<string>> &KB) {
  bool resolved = true;
  while (resolved) {
    resolved = false;
    for (int i = 0; i < KB.size(); i++) {
      if (KB[i].size() == 1 && isConstant(KB[i])) {
        for (int j = 0; j < sentence.size(); j++) {
          if (areComplements(KB[i][0], sentence[j]) &&
              areConstantComplements(KB[i][0], sentence[j])) {
            set<string> temp;
            for (auto &x : KB[i]) {
              string orig = x;
              string n_orig =
                  x[0] == '~' ? orig.substr(1, orig.size() - 1) : "~" + orig;
              if (temp.count(orig) > 0)
                continue;
              else if (temp.count(n_orig) > 0)
                temp.erase(n_orig);
              else
                temp.insert(orig);
            }
            for (auto &y : sentence) {
              string orig = y;
              string n_orig =
                  y[0] == '~' ? orig.substr(1, orig.size() - 1) : "~" + orig;
              if (temp.count(orig) > 0)
                continue;
              else if (temp.count(n_orig) > 0)
                temp.erase(n_orig);
              else
                temp.insert(orig);
            }
            sentence.clear();
            for (auto &x : temp) sentence.push_back(x);

            resolved = true;
            break;
          }
        }
      }
      if (resolved) break;
    }
  }
}

void DisplayQuery(vector<string> &sentence, vector<string> &query,
                  vector<string> &resultant) {
  for (int i = 0; i < sentence.size(); i++) {
    cout << sentence[i];
    if (i + 1 < sentence.size()) cout << " | ";
  }
  cout << " Resolved with: ";
  for (int i = 0; i < query.size(); i++) {
    cout << query[i];
    if (i + 1 < query.size()) cout << " | ";
  }
  cout << " Gives: ";

  for (int i = 0; i < resultant.size(); i++) {
    cout << resultant[i];
    if (i + 1 < resultant.size()) cout << " | ";
  }
  cout << "\n\n";
}

bool subsumes(string &A, string &B) {
  string A_name = "";
  vector<string> A_param;
  string temp = "";
  for (int i = 0; i < A.size(); i++) {
    if (A[i] == '(') {
      A_name = temp;
      temp = "";
    } else if (A[i] == ',') {
      A_param.push_back(temp);
      temp = "";
    } else if (A[i] == ')') {
      A_param.push_back(temp);
    } else
      temp += A[i];
  }

  temp = "";
  string B_name = "";
  vector<string> B_param;
  for (int i = 0; i < B.size(); i++) {
    if (B[i] == '(') {
      B_name = temp;
      temp = "";
    } else if (B[i] == ',') {
      B_param.push_back(temp);
      temp = "";
    } else if (B[i] == ')') {
      B_param.push_back(temp);
    } else
      temp += B[i];
  }

  if (A_name != B_name || A_param.size() != B_param.size()) return false;

  map<string, string> theta;
  if (unify(A, B, theta)) {
    for (auto &x : theta) {
      if (x.first[x.first.size() - 1] == '2') return false;
    }
    return true;
  } else {
    return false;
  }
}

bool subsumption(vector<string> &query, vector<vector<string>> &KB) {
  for (auto &sentence : KB) {
    int subsumptions = 0;
    for (auto &q_pred : query) {
      bool subsumptionFound = false;
      for (auto &s_pred : sentence) {
        if (subsumes(s_pred, q_pred)) {
          subsumptions++;
          subsumptionFound = true;
          break;
        }
      }
      if (subsumptionFound) break;
    }

    if (subsumptions >= sentence.size()) return true;
  }
  return false;
}

bool resolution(vector<vector<string>> &KB, vector<string> &queries,
                int closed) {
  sort(KB.begin(), KB.end(),
       [](const vector<string> &a, const vector<string> &b) -> bool {
         return a.size() < b.size();
       });

  for (auto &query : queries) {
    for (int i = 0; i < KB.size(); i++) {
      if (closed == i) continue;
      vector<string> sentence = KB[i];
      /*  Order(X,Y) Stock X Y */
      for (auto &literal : sentence) {
        if (areComplements(literal, query)) {
          vector<string> resolutionAnswer;

          if (areConstantComplements(literal, query)) {
            // Leave(x) | Stocked(x,y) | ~Stocked(x,z)
            // ~Leave(x) | Stocked(x,y)

            set<string> temp;
            for (auto &x : queries) {
              string orig = x;
              string n_orig =
                  x[0] == '~' ? orig.substr(1, orig.size() - 1) : "~" + orig;
              if (temp.count(orig) > 0)
                continue;
              else if (temp.count(n_orig) > 0)
                temp.erase(n_orig);
              else
                temp.insert(orig);
            }
            for (auto &y : sentence) {
              string orig = y;
              string n_orig =
                  y[0] == '~' ? orig.substr(1, orig.size() - 1) : "~" + orig;
              if (temp.count(orig) > 0)
                continue;
              else if (temp.count(n_orig) > 0)
                temp.erase(n_orig);
              else
                temp.insert(orig);
            }

            for (auto &x : temp) {
              resolutionAnswer.push_back(x);
            }
          } else {
            map<string, string> theta;
            if (unify(literal, query, theta)) {
              resolutionAnswer = substitute(sentence, queries, theta);
            } else {
              continue;
            }
          }

          // Simplify the resolution + do unit propogation + tautology
          // elimination
          simplification(resolutionAnswer);
          unitResolve(resolutionAnswer, KB);

          if (subsumption(resolutionAnswer, KB)) continue;
          // DisplayQuery(sentence, queries, resolutionAnswer);

          string lookup_string = "";
          for (auto &x : resolutionAnswer) lookup_string += x + " | ";
          if (lookup_string.size() > 3)
            lookup_string = lookup_string.substr(0, lookup_string.size() - 3);

          if (myset.find(lookup_string) != myset.end())
            continue;
          else
            myset.insert(lookup_string);

          if (resolutionAnswer.size() == 0) {
            // DisplayQuery(sentence, queries, resolutionAnswer);
            return true;
          } else if (!checkIn(resolutionAnswer,
                              KB))  // Check if resolution is not in KB instead
          {
            KB.push_back(resolutionAnswer);
            if (resolution(KB, resolutionAnswer, i) == true) {
              // DisplayQuery(sentence, queries, resolutionAnswer);
              return true;
            }
            for (int kbi = 0; kbi < KB.size(); kbi++) {
              if (KB[kbi] == resolutionAnswer) {
                KB.erase(KB.begin() + kbi);
                break;
              }
            }
          }
        }
      }
    }
  }

  return false;
}

int main() {
  ifstream inputFile("input.txt");
  string queryAsked = "";
  vector<vector<string>> KB;
  string inputQuery = "";
  if (inputFile.fail())
    cout << "File unable to parse\n";
  else {
    int lineNumber = 0;
    while (getline(inputFile, inputQuery)) {
      if (lineNumber == 0) {
        string tem = "";
        for (int i = 0; i < inputQuery.size(); i++) {
          if (inputQuery[i] == ' ') continue;
          tem += inputQuery[i];
        }
        queryAsked = tem;
        lineNumber++;
        continue;
      } else if (lineNumber < 2) {
        lineNumber++;
        continue;
      }

      vector<string> breakDown;
      string temporary = "";
      for (int i = 0; i < inputQuery.size(); i++) {
        if (inputQuery[i] == ' ') continue;

        temporary += inputQuery[i];
      }
      int start = 0;
      for (int i = 0; i < temporary.size(); i++) {
        if (temporary[i] == '&' || temporary[i] == '|' || temporary[i] == '=') {
          breakDown.push_back(temporary.substr(start, i - start));
          start = i + 1;
          if (temporary[i] == '=') start++;

          switch (temporary[i]) {
            case '&': {
              breakDown.push_back("&");
              break;
            }
            case '|': {
              breakDown.push_back("|");
              break;
            }
            case '=': {
              breakDown.push_back("=>");
              break;
            }
          }
        }
      }

      breakDown.push_back(temporary.substr(start, temporary.size() - start));
      Tree *x = new Tree(breakDown, 0, breakDown.size());
      x->conversionToCNF();
      vector<Tree *> solution = x->DistributiveProp();
      for (auto m : solution) {
        KB.push_back(m->finalAnswer());
        delete (m);
      }
    }

    // cout << queryAsked << "\n";
    // for (int j = 0; j < KB.size(); j++)
    //{
    //     cout << j << " ";

    //    for (int i = 0; i < KB[j].size(); i++)
    //    {
    //        cout << KB[j][i];
    //        if (i + 1 < KB[j].size())
    //            cout << " | ";
    //        else
    //            cout << "\n";
    //    }
    //}

    queryAsked = queryAsked[0] == '~'
                     ? queryAsked.substr(1, queryAsked.size() - 1)
                     : "~" + queryAsked;
    KB.push_back({queryAsked});
    vector<string> negatedQuery = {queryAsked};
    bool answer = resolution(KB, negatedQuery, -1);
    ofstream outputFile("output.txt");
    if (answer) {
      cout << "TRUE\n";
      outputFile << "TRUE";
    } else {
      cout << "FALSE\n";
      outputFile << "FALSE";
    }
    // map<string, string> theta;
    // string predA = "A(x,Pizza)";
    // string predB = "A(x,y)";
    // if (unify(predA, predB, theta))
    // {
    //     cout << predA << " & " << predB << " Unification possible, mapping
    //     is\n"; for (auto &x : theta)
    //         cout << x.first << ", " << x.second << "\n";
    // }
    // else
    // {
    //     cout << predA << " & " << predB << " Unification not possible :(\n";
    // }
  }

  return 0;
}