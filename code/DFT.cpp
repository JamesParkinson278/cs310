#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <string.h>
#include <string>
#include <deque>
#include <vector>
#include <map>
#include <tuple>
#include <set>
#include <math.h>

using namespace std;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"

string bool_to_string(bool b){
    return b? "True" : "False";
}

enum DIRECTION {
    LEFT = -1,
    STAY = 0,
    RIGHT = 1
};

enum END_MARKER {
    LEFT_ENDMARKER = '<',
    RIGHT_ENDMARKER = '>'
};

typedef int STATE;

class Two_DFT {
    set<char> input_alphabet;
    set<char> output_alphabet;
    int numStates = 0;
    STATE start_state;
    STATE accept_state;
    deque<map<char, tuple<STATE, DIRECTION, string>>> transitions;
public:
    Two_DFT(string in_alpha, string out_alpha) {
        for (char ch : in_alpha){ 
            input_alphabet.insert(ch);
        }
        for (char ch : out_alpha){
            output_alphabet.insert(ch);
        }
    }

    set<char> getInputAlpha(){
        return input_alphabet;
    }

    set<char> getOutputAlpha(){
        return output_alphabet;
    }

    int getStates(){
        return numStates;
    }

    STATE getStartState(){
        return start_state;
    }

    STATE get_accept_state(){
        return accept_state;
    }

    deque<map<char, tuple<STATE, DIRECTION, string>>> getTransitions(){
        return transitions;
    }

    STATE addState(){
        STATE newState = numStates++;
        map<char, tuple<STATE, DIRECTION, string>> t;
        transitions.push_back(move(t));
        return newState;
    }

    deque<STATE> addStates(int n){
        deque<STATE> states;
        for (int i = 0; i < n; i++){
            states.push_back(addState());
        }
        return move(states);
    }

    void makeAccept(STATE q){
        accept_state = q;
    }

    void makeStart(STATE q){
        start_state = q;
    }

    void addTransition(STATE start, char ch, STATE end, DIRECTION dir, string out){
        tuple<STATE, DIRECTION, string> output = make_tuple(end, dir, out);
        transitions[start].insert_or_assign(ch, output);
    }

    string run(string w){
        string output = "";
        STATE currentState = start_state;
        int pos = 0;     
        w.insert(0, 1, LEFT_ENDMARKER);
        w.push_back(RIGHT_ENDMARKER);
        const char* word = w.c_str();
        while (true){
            map<char, tuple<STATE, DIRECTION, string>> ts = transitions[currentState];
            if (ts.find(word[pos]) == ts.end()) return "";

            tuple<STATE, DIRECTION, string> transition = ts[word[pos]];
            STATE newState = get<0>(transition);
            DIRECTION dir = get<1>(transition);
            string out = get<2>(transition);

            if (newState == accept_state){
                return output;
            }
            else if (word[pos] == LEFT_ENDMARKER && dir == LEFT || word[pos] == RIGHT_ENDMARKER && dir == RIGHT){
                return "";
            }

            currentState = newState;
            pos += dir;
            output += out;
        }
    }
};

Two_DFT *compose(Two_DFT *B, Two_DFT *A){
    //initialise variables
    set<char> input_alphabet = A->getInputAlpha();
    set<char> intermediate_alphabet = B->getInputAlpha();
    set<char> output_alphabet = B->getOutputAlpha();
    input_alphabet.insert(LEFT_ENDMARKER);
    input_alphabet.insert(RIGHT_ENDMARKER);
    intermediate_alphabet.insert(LEFT_ENDMARKER);
    intermediate_alphabet.insert(RIGHT_ENDMARKER);
    output_alphabet.insert(LEFT_ENDMARKER);
    output_alphabet.insert(RIGHT_ENDMARKER);
    int num_states_A = A->getStates();
    int num_states_B = B->getStates();
    auto transitions_A = A->getTransitions();
    auto transitions_B = B->getTransitions();

    //create the new 2DFT with the correct input and output alphabets
    string input_alphabet_s = "";
    string output_alphabet_s = "";
    for (auto ch : input_alphabet){
        if (ch != LEFT_ENDMARKER && ch != RIGHT_ENDMARKER) input_alphabet_s += ch;
    }
    for (auto ch : output_alphabet){
        if (ch != LEFT_ENDMARKER && ch != RIGHT_ENDMARKER) output_alphabet_s += ch;
    }
    Two_DFT *C = new Two_DFT(input_alphabet_s, output_alphabet_s);
    
    //generate the alphabet that represents the configuration graph
    map<char, deque<tuple<STATE, STATE, DIRECTION, string>>> graph_alphabet;
    for (auto ch : input_alphabet){
        deque<tuple<STATE, STATE, DIRECTION, string>>> alpha;
        for (STATE s = 0; s < num_states_A; s++){
            tuple<STATE, DIRECTION, string> transition = transitions_A[s][ch];
            tuple<STATE, STATE, DIRECTION, string> t = make_tuple(s, get<0>(transition), get<1>(transition), get<2>(transition));
            alpha.push_back(t);
        }
        graph_alphabet.insert({ch, alpha});
    }
}



void test(Two_DFT *B, Two_DFT *A, int numTests){
    srand(time(0));
    set<char> alpha = A->getAlpha();
    Two_DFT *C = compose(A, B);
    int states = C->numStates;
    int numRight = 0;

    for (int i = 0; i < numTests; i++){
        int len = rand() % states*2;
        string input = "";
        for (int j = 0; j < len; j++){
            int k = rand() % alpha.size();
            set<char>::iterator it = alpha.begin();
            for (;k != 0; k--) it++;
            char letter = *it;
            input += letter;
        }
        string output1 = B->run(A->run(input));
        string output2 = C->run(input);

        if (output1 == output2){
            numRight += 1;
        }

        printf("Input w = %s\nB(A(w)) = %s\nC(w) = %s\n\n", input.c_str(), bool_to_string(output2DFA).c_str(), bool_to_string(output1DFA).c_str());
    }

    printf("# Correct Outputs: %d / %d\n", numRight, numTests);
}

void test1(){ //swap a and b
    Two_DFT *m = new Two_DFT("ab", "ab");
    STATE q = m->addState();
    STATE accept = m->addState();
    m->makeAccept(accept);

    m->addTransition(q, LEFT_ENDMARKER, q, RIGHT, "");
    m->addTransition(q, RIGHT_ENDMARKER, accept, STAY, "");
    m->addTransition(q, 'a', q, RIGHT, "b");
    m->addTransition(q, 'b', q, RIGHT, "a");

    string w1 = "abab"; //baba
    string w2 = "aabba"; //bbaab
    string w3 = "bababb"; //ababaa
    string w4 = "aabababbbabbbababaaba"; //bbababaaabaaabababbab
    string w5 = "abababababaaaaabbbaaaba"; //babababababbbbbaaabbbab
    string w6 = ""; //

    printf((w1 + " : 2DFT - " + m->run(w1) + " - " + bool_to_string(m->run(w1) == "baba") + "\n").c_str());
    printf((w2 + " : 2DFT - " + m->run(w2) + " - " + bool_to_string(m->run(w2) == "bbaab") + "\n").c_str());
    printf((w3 + " : 2DFT - " + m->run(w3) + " - " + bool_to_string(m->run(w3) == "ababaa") + "\n").c_str());
    printf((w4 + " : 2DFT - " + m->run(w4) + " - " + bool_to_string(m->run(w4) == "bbababaaabaaabababbab") + "\n").c_str());
    printf((w5 + " : 2DFT - " + m->run(w5) + " - " + bool_to_string(m->run(w5) == "babababababbbbbaaabbbab") + "\n").c_str());
    printf((w6 + " : 2DFT - " + m->run(w6) + " - " + bool_to_string(m->run(w6) == "") + "\n").c_str());

}

void test2(){ //if word ends with b, replace all a with c, otherwise do nothing
    Two_DFT *m = new Two_DFT("ab", "abc");
    deque<STATE> states = m->addStates(6);
    STATE accept = m->addState();
    m->makeAccept(accept);

    m->addTransition(states[0], LEFT_ENDMARKER, states[0], RIGHT, "");
    m->addTransition(states[0], RIGHT_ENDMARKER, states[1], LEFT, "");
    m->addTransition(states[0], 'a', states[0], RIGHT, "");
    m->addTransition(states[0], 'b', states[0], RIGHT, "");

    m->addTransition(states[1], LEFT_ENDMARKER, states[3], STAY, "");
    m->addTransition(states[1], RIGHT_ENDMARKER, states[1], LEFT, "");
    m->addTransition(states[1], 'a', states[3], RIGHT, "");
    m->addTransition(states[1], 'b', states[2], RIGHT, "");

    m->addTransition(states[2], LEFT_ENDMARKER, states[4], RIGHT, "");
    m->addTransition(states[2], RIGHT_ENDMARKER, states[2], LEFT, "");
    m->addTransition(states[2], 'a', states[2], LEFT, "");
    m->addTransition(states[2], 'b', states[2], LEFT, "");

    m->addTransition(states[3], LEFT_ENDMARKER, states[5], RIGHT, "");
    m->addTransition(states[3], RIGHT_ENDMARKER, states[3], LEFT, "");
    m->addTransition(states[3], 'a', states[3], LEFT, "");
    m->addTransition(states[3], 'b', states[3], LEFT, "");

    m->addTransition(states[4], LEFT_ENDMARKER, states[4], RIGHT, "");
    m->addTransition(states[4], RIGHT_ENDMARKER, accept, STAY, "");
    m->addTransition(states[4], 'a', states[4], RIGHT, "c");
    m->addTransition(states[4], 'b', states[4], RIGHT, "b");

    m->addTransition(states[5], LEFT_ENDMARKER, states[5], RIGHT, "");
    m->addTransition(states[5], RIGHT_ENDMARKER, accept, STAY, "");
    m->addTransition(states[5], 'a', states[5], RIGHT, "a");
    m->addTransition(states[5], 'b', states[5], RIGHT, "b");

    string w1 = "abab"; //cbcb
    string w2 = "aabba"; //aabba
    string w3 = "bababb"; //bcbcbb
    string w4 = "aabababbbabbbababaaba"; //aabababbbabbbababaaba
    string w5 = "abababababaaaaabbbaaabb"; //cbcbcbcbcbcccccbbbcccbb
    string w6 = ""; //

    printf((w1 + " : 2DFT - " + m->run(w1) + " - " + bool_to_string(m->run(w1) == "cbcb") + "\n").c_str());
    printf((w2 + " : 2DFT - " + m->run(w2) + " - " + bool_to_string(m->run(w2) == "aabba") + "\n").c_str());
    printf((w3 + " : 2DFT - " + m->run(w3) + " - " + bool_to_string(m->run(w3) == "bcbcbb") + "\n").c_str());
    printf((w4 + " : 2DFT - " + m->run(w4) + " - " + bool_to_string(m->run(w4) == "aabababbbabbbababaaba") + "\n").c_str());
    printf((w5 + " : 2DFT - " + m->run(w5) + " - " + bool_to_string(m->run(w5) == "cbcbcbcbcbcccccbbbcccbb") + "\n").c_str());
    printf((w6 + " : 2DFT - " + m->run(w6) + " - " + bool_to_string(m->run(w6) == "") + "\n").c_str());

}

int main(){
    
    // test1();
    test2();

}