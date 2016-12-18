#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <string>
#include <map>
#include <list>


using namespace std;

string toBinary (int a){
    if (a == 0){
        return "0";
    }
    string res = "";
    while (a != 0){
        res = to_string(a % 2) + res;
        a /= 2;
    }
    return res;
}

class Symbol {
public:
    char a1, a2, a3;
    Symbol(char a1,char a2,char a3){
        this->a1 = a1;
        this->a2 = a2;
        this->a3 = a3;
    }
    Symbol(){
    }
    bool operator == (const Symbol &b) const{
       return (b.a1 == a1 && b.a2 == a2 && a3 == b.a3);
    }
    bool operator != (const Symbol &b) const{
        return ! this->operator ==(b);
    }

    bool operator < (const Symbol &b) const{
       return (b.a1> a1 || b.a1 == a1  && b.a2 > a2 || b.a1 == a1  && b.a2 == a2 && a3 > b.a3);
    }
    bool operator > (const Symbol &b) const{
       return (b.a1< a1 || b.a1 == a1  && b.a2 < a2 || b.a1 == a1  && b.a2 == a2 && a3 < b.a3);
    }
    bool operator >= (const Symbol &b) const{
        return ! (this->operator <(b));
    }
    bool operator <= (const Symbol &b) const{
        return ! (this->operator >( b));
    }

    string toString(){
        return string("") + a1 + ',' + a2 + ',' + a3  ;
    }
    string toStringX(){
        return string("") + a2 + ',' + a3  ;
    }
};

typedef pair<int, Symbol> Input;

class Output{
public:
    // state, shift
    int q , sh;
    //symbol on tape
    Symbol s;
    Output(){

    }
    bool isNull (){
        return sh == 0;
    }

    Output(int a, Symbol b, int c){
        this->q = a;
        this->s = b;
        this->sh = c;

    }

};

class Automata{
private:
    const int startState = 0;
    const int endState = 14;
    const int acceptState = 1000;
    const int rejectState = 2000;

    const array<char, 2> alphabet = array<char,2> {'0', '1'};
    const array<char, 3> gamma = array<char,3> {'0', '1', 'B'};

    vector <Symbol> lenta = vector<Symbol>();
    map <Input, Output> transitions;

    map <string, pair<string, string>> rules;

    Symbol getSymbol (int i){
        if ((i == lenta.size()) ){
            return Symbol('$', '$', '$');
        }
        if ((i == -1)){
            return Symbol('c', 'c', 'c');
        }
        if ((i < -1) || (i > lenta.size()) ){
            throw "getSymbol out of bounds";
        }
        return lenta[i];
    }
    void setSymbol(int i, Symbol ch){
        if (((i == -1)) && (ch.a1 == 'c')){
            return;
        }
        if (( (i == lenta.size())) && (ch.a1 == '$') ){
            return;
        }
        if ((i < -1) || (i > lenta.size()) ){
            throw "setSymbol out of bounds";
        }
        lenta[i] = ch ;
    }

    Symbol getSymbol(stringstream &iss){
        string token;
        getline(iss, token, ' ');
        char a1 = token[0];

        getline(iss, token, ' ');
        char a2 = token[0];

        getline(iss, token, ' ');
        char a3 = token[0];

        return Symbol(a1, a2, a3);
    }

    void readTransition(string line){
        stringstream iss;
        string token;

        iss << line;
        getline(iss, token, ' ');
        int q1 = atoi(token.c_str());
        Symbol a = getSymbol(iss);

        getline(iss, token, ' ');
        int q2 = atoi(token.c_str());

        Symbol b = getSymbol(iss);

        getline(iss, token);
        int shift =  (token[0] == 'R')? 1 : -1;

        iss.clear();

        Input in_state = Input (q1, a);
        Output out_state = Output (q2, b, shift);
        transitions[in_state] = out_state;
    }

    void readTransitions(){
        ifstream file ("transitions.txt");
        if (file.is_open())
          {
            string line;
            while ( getline(file, line) )
               {
                  readTransition(line);
               }
            file.close();
          }
          else throw "Unable to open read file";
    }

    bool isReject (int state){
        return state == rejectState;
    }
    bool isAccept(int state){
        return state == acceptState;
    }

    // need for execute
    pair <int,int> step(pair <int,int> state_index){
        int state = state_index.first;
        int index = state_index.second;

        Symbol symbol;
        try {
             symbol = getSymbol(index);
        } catch (char const* e){
            cout << "get " << index << ' '<< state << ' '<< lenta.size()<<  endl;
        }

        Input in_state = Input (state, symbol);
        Output out_state = transitions[in_state];
        if (out_state.isNull()){
            cout << state << ' ' << symbol.toString() << " no transition"<< endl;
            throw "No transition";
        }
        try {
             setSymbol(index, out_state.s);
        } catch (char const* e){
            cout << "set " << index <<' '<<  state << ' ' << ' '<< endl;
            throw "Index error";
        }
        return pair <int,int>(out_state.q, index + out_state.sh);
    }

    void registerGrammarRule(ofstream& outfile, string startTerminal, string outTerminal1, string outTerminal2){
        outfile << startTerminal << "->"<< outTerminal1 << outTerminal2 << std::endl;
        rules[startTerminal] = pair<string,string>(outTerminal1, outTerminal2);
    }

    string stateToString (int n){
        return string("q") + to_string(n);
    }

    void generateGrammar (){
        ofstream outfile ("grammar.txt");

        string startTerminal, outTerminal1, outTerminal2;
        // (4.1) S -> V[c, q0, B, B, a] T
        for (char elem : alphabet){
             startTerminal = "S";
             outTerminal1 =  string("V(c,")+ stateToString(startState) + "," + "B,B," + elem + ")";
             outTerminal2 = "T";
             registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);
        }


        for (char elem : alphabet){
            //(4.2) T → V(B, B, a)T
            startTerminal = "T";
            outTerminal1 =  string("V(") +"B,B," + elem +")";
            outTerminal2 = "T";
            registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);

            //(4.3) T -> V (B,B,a, $)
            outTerminal1 =  string("V(")+ elem + ",B,B,$)";
            outTerminal2 = "";
            registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);
        }
        //for transitions
        for(auto iterator = transitions.begin(); iterator != transitions.end(); iterator++) {
            Input in_state = iterator->first;
            Output out_state = iterator->second;

            int q = in_state.first;
            Symbol x = in_state.second;

            int p = out_state.q;
            Symbol y = out_state.s;
            int shift = out_state.sh;

            for (char a : alphabet){

                if ((shift == 1) && (x.a1 == 'c') && (y.a1 == 'c')){
                    // (5.1)  δ(q, ¢)=(p, 'c', R)
                    // [q, ¢, XX1, XX2, a] → [ ¢, p, XX1, XX2, a]

                    for (auto xx1 : gamma){
                        for (auto xx2 : gamma){
                            Symbol xx = Symbol (-1, xx1, xx2);
                            startTerminal =string("V(") + stateToString(q) + ",c," + xx.toStringX() + ','+ a + ")";
                            outTerminal1 =  string("V(c,") + stateToString(p) + ',' +xx.toStringX() + ',' + a + ")";
                            outTerminal2 = "";
                            registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);
                        }

                    }
                } else if ((shift == -1) && (x.a1 == '$') && (y.a1 == '$')){
                    // (7.2) δ(q, $) = ( p, $, L)
                    // [XX1, XX2 , a, q, $] → [p, XX1, XX2 , a, $]
                    for (auto xx1 : gamma){
                        for (auto xx2 : gamma){
                            Symbol xx = Symbol (-1, xx1, xx2);
                            startTerminal =string("V(") + xx.toStringX() + ',' + a + "," + stateToString(q) + ",$)";
                            outTerminal1 =  string("V(") + stateToString(p) + ',' + xx.toStringX()+ ',' + a + ",$)";
                            outTerminal2 = "";
                            registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);

                        }
                    }

                }
            }

            char a = x.a1; // == y.a1;
                // (5.3) (6.1) (6.3) (7.1) δ(q, X ) = ( p, Y, R)
            if ((shift == 1) && (x.a1 != 'c') && (x.a1 != '$')){
                for (char z2 : gamma){
                    for (char z3 : gamma){
                        for (char b : alphabet){
                            //(5.3) [¢, q, X2, X3, a] [Z2, Z3 , b]→ [¢, Y2, Y3, a] [p, Z2, Z3, b]
                            Symbol z = Symbol(-1, z2,z3);
                            startTerminal =string("V(c,") + stateToString(q) + ',' + x.toStringX() + ',' + a + ")";
                            startTerminal +=string("V(") + z.toStringX() + ',' + b + ")";
                            outTerminal1 =  string("V(c,") +  y.toStringX() + ',' + a + ")";
                            outTerminal2 = string("V(") + stateToString(p) + ',' + z.toStringX() + ',' + b + ")";
                            registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);

                            //(6.1) [q, X2, X3, a] [Z2, Z3, b] → [Y2, Y3, a][ p, Z2, Z3, b]
                            startTerminal =string("V(") + stateToString(q) + ',' + x.toStringX() + ',' + a + ")";
                            startTerminal +=string("V(") + z.toStringX() + ',' + b + ")";
                            outTerminal1 =  string("V(") +  y.toStringX() + ',' + a + ")";
                            outTerminal2 = string("V(") + stateToString(p) + ',' + z.toStringX() + ',' + b + ")";
                            registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);

                            //(6.3) [q, X2, X3, a] [Z2, Z3, b, $]→ [Y2, Y3, a] [p, Z2, Z3, b, $]
                            startTerminal =string("V(") + stateToString(q) + ',' + x.toStringX() + ',' + a + ")";
                            startTerminal +=string("V(") + z.toStringX() + ',' + b + ",$)";
                            outTerminal1 =  string("V(") +  y.toStringX()+ ',' + a + ")";
                            outTerminal2 = string("V(") + stateToString(p) + ',' + z.toStringX() + ',' + b + ",$)";
                            registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);
                        }


                        //(7.1) [q, X2, X3, a, $] → [ Y2, Y3 , a, p, $]

                        startTerminal =string("V(") + stateToString(q) + ',' + x.toStringX() + ',' + a + ",$)";
                        outTerminal1 =  string("V(") + y.toStringX() + ',' + a  + ","+ stateToString(p) +  +",$)";
                        outTerminal2 = "";
                        registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);

                }
            }
        }
        else if ((shift == -1)  && (x.a1 != 'c') && (x.a1 != '$') ){
            //(5.2) [¢, q, X2, X3, a] → [p, ¢, Y2, Y3, a]
            startTerminal =string("V(c,") + stateToString(q) + ',' + x.toStringX() + ',' + a + ")";
            outTerminal1 = string("V(") + stateToString(p) + ",c," + y.toStringX() + ',' + a + ")";
            outTerminal2 = "";
            registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);

            for (char z2 : gamma){
                for (char z3 : gamma){
                    Symbol z = Symbol (-1, z2, z3);
                    for (char b : alphabet){
                        // (6.2) [Z2, Z3, b] [q, X2, X3, a] → [p, Z2,Z3, b] [Y2,Y3, a]
                        startTerminal =string("V(") + z.toStringX() + ',' + b + ")";
                        startTerminal +=string("V(") + stateToString(q) + ',' + x.toStringX() + ',' + a + ")";
                        outTerminal1 = string("V(") + stateToString(p) + ',' + z.toStringX() + ',' + b + ")";
                        outTerminal2 =  string("V(") +  y.toStringX() + ',' + a + ")";
                        registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);


                        // (6.4) [c, Z , b][q, X, a] → [c, p, Z, b][Y, a]
                        startTerminal =string("V(c,") + z.toStringX() + ',' + b + ")";
                        startTerminal +=string("V(") + stateToString(q) + ',' + x.toStringX() + ',' + a + ")";
                        outTerminal1 = string("V(c,") + stateToString(p) + ',' + z.toStringX() + ',' + b + ")";
                        outTerminal2 =  string("V(") +  y.toStringX() + ',' + a + ")";
                        registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);


                        //(7.3) [Z , b] [q, X , a, $] → [p, Z , b] [Y , a, $]
                        startTerminal =string("V(") + z.toStringX() + ',' + b + ")";
                        startTerminal +=string("V(") + stateToString(q) + ',' + x.toStringX() + ',' + a + ",$)";
                        outTerminal1 = string("V(") + stateToString(p) + ',' + z.toStringX() + ',' + b + ")";
                        outTerminal2 =  string("V(") +  y.toStringX() + ',' + a + ",$)";
                        registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);
                    }
                }
            }
        }
    }
    // final state

    for (char a : alphabet){
        for (char x2 : gamma){
            for (char x3 : gamma){
                Symbol x = Symbol (-1, x2,x3);

                //(8.1) [q, ¢ , X , a] → a;
                startTerminal =string("V(")+ stateToString(acceptState) +",c," + x.toStringX() + ',' + a + ")";
                outTerminal1 = a;
                outTerminal2 = "";
                registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);

                //(8.2) [¢, q, X, a] → a;
                startTerminal =string("V(c,")+ stateToString(acceptState) +"," + x.toStringX() + ',' + a + ")";
                outTerminal1 = a;
                outTerminal2 = "";
                registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);

                //(8.3) [q, X, a] → a;
                startTerminal =string("V(")+ stateToString(acceptState) +"," + x.toStringX() + ',' + a + ")";
                outTerminal1 = a;
                outTerminal2 = "";
                registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);

                //(8.4) [q, X, a, $] → a;
                startTerminal =string("V(")+ stateToString(acceptState) +"," + x.toStringX() + ',' + a + ",$)";
                outTerminal1 = a;
                outTerminal2 = "";
                registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);

                //(8.5) [X, a, q, $] → a
                startTerminal =string("V(")+ x.toStringX() + ',' + a  +","+ stateToString(acceptState) +  ",$)";
                outTerminal1 = a;
                outTerminal2 = "";
                registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);
            }
        }
    }

    // recovery of input
    for (char a : alphabet){
        for (char x2 : gamma){
            for (char x3 : gamma){
                Symbol x = Symbol (-1, x2, x3);
                for (char b : alphabet){
                    //(9.1) a[X, b] → ab;
                    startTerminal =  a + string("V(") + x.toStringX()  + ','+ b + ")";
                    outTerminal1 = a;
                    outTerminal2 = b;
                    registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);

                    //(9.2) a[X, b, $] → ab
                    startTerminal = a+ string("V(") + x.toStringX() + ','+ b + ",$)";
                    outTerminal1 = a;
                    outTerminal2 = b;
                    registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);

                    //(9.3) [X , a]b → ab;
                    startTerminal = string("V(") + x.toStringX() + ','+ a + ")" + b;
                    outTerminal1 = a;
                    outTerminal2 = b;
                    registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);

                    //(9.4) [¢, X, a]b → ab
                    startTerminal = string("V(c,") + x.toStringX() + ','+ a + ")" + b;
                    outTerminal1 = a;
                    outTerminal2 = b;
                    registerGrammarRule(outfile,startTerminal, outTerminal1, outTerminal2);
                }

            }
        }
    }
    outfile.close();
}

public:
    Automata(){
        this->readTransitions();
        this->generateGrammar();
    }
    ~Automata(){

    }
    void execute(string word, bool isOut){
        for (int i = 0; i < word.length(); ++i ){
            Symbol s = Symbol(word[i], 'B', 'B');
            lenta.push_back(s);
        }
        pair <int, int> prev_state;
        pair <int, int> state_index = pair <int, int> (startState, 0);
        if (isOut){
            cout << state_index.first << endl;
        }

        while (true){
            prev_state = state_index;
            state_index = step(state_index);
            if (isOut){
                cout << state_index.first << endl;
            }

            if (isAccept(state_index.first)){
                cout << "accept" << endl;
                return;
            }
            if (isReject(state_index.first)){
                cout << "reject" << endl;
                return;
            }
        }
    }

    void showCurrOut(ofstream &outfile, list<string> &currOut){
        for(std::list<string>::iterator curr2 = currOut.begin(); curr2 != currOut.end(); curr2++){
            outfile << *curr2 ;

         }
         outfile << endl<< endl;
    }

    void startPreparation(ofstream &outfile, list<string> &currOut, int number){
        string str = toBinary(number);

        // (4.1) S -> [c, q0, 'B', 'B', a] T
        string startTerminal = "S";
        string outTerminal1 =  string("V(c,")+ stateToString(startState) +",B,B," + str[0] + ")";
        string outTerminal2 = "T";
        outfile << "S" << endl  << endl;
        outfile << startTerminal << "->" <<  outTerminal1 <<  outTerminal2 << endl;
        currOut.push_back(outTerminal1);
        showCurrOut(outfile, currOut);

        for (int i = 1; i < str.size(); ++i){
            //(4.2) T → V(B, B, a)T
            string startTerminal = "T";
            string outTerminal1 =  string("V(") +"B,B,"+ str[i] + ")";
            string outTerminal2 = "T";
            if (i == str.size() -1){
                //(4.3) T -> V (a,a, $)
                outTerminal1 =  string("V(") + "B,B,"+ str[i] + ",$)";
                outTerminal2 = "";
            }
            outfile << startTerminal << "->" <<  outTerminal1 <<  outTerminal2 << endl;
            currOut.push_back(outTerminal1);
            showCurrOut(outfile, currOut);
        }
    }

    bool isAccepted (list<string> &currOut){
        for(std::list<string>::iterator curr2 = currOut.begin(); curr2 != currOut.end(); curr2++){
            if ((*curr2).size() != 1){
                return false;
            }
        return true;
        }
    }

    bool stepGrammar(ofstream &outfile, list<string> &currOut){
        string left;
        std::list<string>::iterator next;
        for(std::list<string>::iterator curr = currOut.begin(); curr != currOut.end(); curr++){
            next = curr;
            ++ next;

            if (next != currOut.end()){
                left = (*curr) + (*next);
                if (rules.find(left) != rules.end()){
                    pair <string, string> right = rules.at(left);
                    outfile << left << "->" <<  right.first <<  right.second << endl;

                    auto nnext = currOut.erase(curr);
                    nnext = currOut.erase(nnext);
                    currOut.insert(nnext, right.first );
                    if (right.second != ""){
                         currOut.insert(nnext, right.second );
                    }
                    return true;
                }
            }

            left = (*curr);
            if (rules.find(left) != rules.end()){
                pair <string, string> right = rules.at(left);
                outfile << left << "->" <<  right.first <<  right.second << endl;

                auto nnext = currOut.erase(curr);
                currOut.insert(nnext, right.first );
                if (right.second != ""){
                     currOut.insert(nnext, right.second );
                }

                return true;
            }
        }
        return false;
    }

    void generateGrammarOutput (int number){
        ofstream outfile (string("grammar output ") + to_string(number)+ ".txt");
        list<string> currOut = list<string>();

        startPreparation(outfile, currOut, number);


        while (stepGrammar(outfile,currOut)){
            showCurrOut(outfile,currOut);

        }

        outfile.close();
        /*if (!isAccepted(currOut)){
            ofstream outfile1 (string("grammar output ") + to_string(number)+ ".txt");
            outfile1.close();
        }*/

    }

};


int main(){

    int n = 3;
    Automata automata = Automata();
    /*string number = toBinary (n);
    cout << number << endl;
    automata.execute(number, true);*/
    automata.generateGrammarOutput(3);//179425027

}
