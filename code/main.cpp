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

class Two_DFA {
    set<char> alphabet;
    int numStates = 0;
    STATE start_state;
    set<STATE> accept_states;
    deque<map<char, tuple<STATE, DIRECTION>>> transitions;
public:
    Two_DFA(string _alphabet) {
        for (char ch : _alphabet){
            alphabet.insert(ch);
        }
    }

    set<char> getAlpha(){
        return move(alphabet);
    }

    int getStates(){
        return numStates;
    }

    STATE getStartState(){
        return start_state;
    }

    set<STATE> get_accept_states(){
        return move(accept_states);
    }

    deque<map<char, tuple<STATE, DIRECTION>>> getTransitions(){
        return move(transitions);
    }

    STATE addState(){
        STATE newState = numStates++;
        map<char, tuple<STATE, DIRECTION>> t;
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
        accept_states.insert(q);
    }

    void makeStart(STATE q){
        start_state = q;
    }

    void addTransition(STATE start, char ch, STATE end, DIRECTION dir){
        tuple<STATE, DIRECTION> output = make_tuple(end, dir);
        transitions[start].insert_or_assign(ch, output);
    }

    bool run(string w){
        STATE currentState = start_state;
        int pos = 0;     
        w.insert(0, 1, LEFT_ENDMARKER);
        w.push_back(RIGHT_ENDMARKER);
        const char* word = w.c_str();
        while (true){
            map<char, tuple<STATE, DIRECTION>> ts = transitions[currentState];
            if (ts.find(word[pos]) == ts.end()) return false;

            tuple<STATE, DIRECTION> transition = ts[word[pos]];
            STATE newState = get<0>(transition);
            DIRECTION dir = get<1>(transition);

            if (word[pos] == RIGHT_ENDMARKER && dir == RIGHT){
                return accept_states.find(newState) != accept_states.end();
            }
            else if (word[pos] == LEFT_ENDMARKER && dir == LEFT){
                return false;
            }

            currentState = newState;
            pos += dir;
        }
    }
};

class DFA {
    set<char> alphabet;
    int numStates = 0;
    STATE start_state;
    set<STATE> accept_states;
    deque<map<char, STATE>> transitions;
public:
    DFA(string _alphabet) {
        for (char ch : _alphabet) {
            alphabet.insert(ch);
        }
    }
    DFA(set<char> _alphabet) {
        alphabet = _alphabet;
    }

    void setAlpha(set<char> alpha){
        alphabet = move(alpha);
    }

    STATE addState(){
        STATE newState = numStates++;
        map<char, STATE> t;
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
        accept_states.insert(q);
    }

    void makeStart(STATE q){
        start_state = q;
    }

    bool addTransition(STATE start, char ch, STATE end){
        if (alphabet.find(ch) != alphabet.end()){
            transitions[start].insert_or_assign(ch, end);
            return true;
        }
        return false;
    }

    bool run(string w){
        STATE currentState = start_state;
        const char* word = w.c_str();
        for (int pos = 0; pos < w.length(); pos++){
            map<char, STATE> ts = transitions[currentState];
            if (ts.find(word[pos]) == ts.end()) return false;

            STATE newState = ts[word[pos]];
            currentState = newState;
        }
        return accept_states.count(currentState) == 1;
    }
};

DFA *convert_to_DFA(Two_DFA *M){
    set<char> alphabet = M->getAlpha();
    int numStates = M->getStates();
    int numDeltas = pow((numStates + 1), numStates);
    deque<map<char, tuple<STATE, DIRECTION>>> transitions = M->getTransitions();
    set<STATE> accept_states = M->get_accept_states();

    DFA *D = new DFA(alphabet);
    STATE new_states[numStates][numDeltas];
    for (int i = 0; i < numStates; i++){
        for (int j = 0; j < numDeltas; j++){
            new_states[i][j] = -1;
        }
    }

    STATE R = -1;
    STATE deltas[numDeltas][numStates];
    for (int j = 0; j < numStates; j++){
        deltas[0][j] = R;
    }
    for (int i = 1; i < numDeltas; i++){
        for (int j = 0; j < numStates; j++){
            deltas[i][j] = -2;
        }
    }

    STATE trap_state = D->addState();
    D->addTransition(trap_state, LEFT_ENDMARKER, trap_state, RIGHT);
    D->addTransition(trap_state, RIGHT_ENDMARKER, trap_state, RIGHT);
    for (auto ch : alphabet){
        D->addTransition(trap_state, ch, trap_state, RIGHT);
    }
    
    STATE current_state = 0;
    int current_delta = 0;
    new_states[current_state][current_delta] = D->addState();
    deque<tuple<STATE, int>> new_state_queue;
    new_state_queue.push_back(make_tuple(current_state, current_delta));
    while (!new_state_queue.empty()){
        tuple<STATE, int> tmp = new_state_queue.front();
        new_state_queue.pop_front();
        current_state = get<0>(tmp);
        current_delta = get<1>(tmp);
        
    }



    D->makeStart(new_states[0][0]);
    for (auto itr : accept_states){
        for (int j = 0; j < numDeltas; j++){
            if (new_states[itr][j] != -1);
            D->makeAccept(new_states[itr][j]);
        }
    }

    return D;
}


string bool_to_string(bool b){
    return b? "True" : "False";
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"

int main(){
    Two_DFA *m = new Two_DFA("ab");
    deque<STATE> states = m->addStates(7);
    m->makeStart(states[0]);
    m->makeAccept(states[5]);
    m->addTransition(states[0], LEFT_ENDMARKER, states[0], RIGHT);
    m->addTransition(states[0], 'a', states[1], RIGHT);
    m->addTransition(states[0], 'b', states[0], RIGHT);
    m->addTransition(states[0], RIGHT_ENDMARKER, states[3], LEFT);
    m->addTransition(states[1], 'a', states[2], RIGHT);
    m->addTransition(states[1], 'b', states[1], RIGHT);
    m->addTransition(states[1], RIGHT_ENDMARKER, states[6], LEFT);
    m->addTransition(states[2], 'a', states[0], RIGHT);
    m->addTransition(states[2], 'b', states[2], RIGHT);
    m->addTransition(states[2], RIGHT_ENDMARKER, states[6], LEFT);
    m->addTransition(states[3], 'a', states[3], LEFT);
    m->addTransition(states[3], 'b', states[4], LEFT);
    m->addTransition(states[3], LEFT_ENDMARKER, states[5], RIGHT);
    m->addTransition(states[4], 'a', states[4], LEFT);
    m->addTransition(states[4], 'b', states[3], LEFT);
    m->addTransition(states[4], LEFT_ENDMARKER, states[6], RIGHT);
    m->addTransition(states[5], 'a', states[5], RIGHT);
    m->addTransition(states[5], 'b', states[5], RIGHT);
    m->addTransition(states[5], RIGHT_ENDMARKER, states[5], RIGHT);
    m->addTransition(states[6], 'a', states[6], LEFT);
    m->addTransition(states[6], 'b', states[6], LEFT);
    m->addTransition(states[6], LEFT_ENDMARKER, states[6], LEFT);

    DFA *n = new DFA("ab");
    deque<STATE> states2 = n->addStates(6);
    n->makeStart(states[0]);
    n->makeAccept(states[0]);
    n->addTransition(states[0], 'a', states[1]);
    n->addTransition(states[1], 'a', states[2]);
    n->addTransition(states[2], 'a', states[0]);
    n->addTransition(states[0], 'b', states[3]);
    n->addTransition(states[1], 'b', states[4]);
    n->addTransition(states[2], 'b', states[5]);
    n->addTransition(states[3], 'a', states[4]);
    n->addTransition(states[4], 'a', states[5]);
    n->addTransition(states[5], 'a', states[3]);
    n->addTransition(states[3], 'b', states[0]);
    n->addTransition(states[4], 'b', states[1]);
    n->addTransition(states[5], 'b', states[2]);

    string w1 = "abbaaabbabababbaa"; //true
    string w2 = "aabababbabbbabbab"; //false
    string w3 = "abbababbabbbabbab"; //false
    string w4 = "aabababbabababbab"; //false
    string w5 = "bbaaabbaaabbaaabbaaa"; //true
    string w6 = ""; //true

    printf((w1 + " : 2DFA - " + bool_to_string(m->run(w1)) + "\n").c_str());
    printf((w2 + " : 2DFA - " + bool_to_string(m->run(w2)) + "\n").c_str());
    printf((w3 + " : 2DFA - " + bool_to_string(m->run(w3)) + "\n").c_str());
    printf((w4 + " : 2DFA - " + bool_to_string(m->run(w4)) + "\n").c_str());
    printf((w5 + " : 2DFA - " + bool_to_string(m->run(w5)) + "\n").c_str());
    printf((w6 + " : 2DFA - " + bool_to_string(m->run(w6)) + "\n").c_str());

    printf((w1 + " : DFA - " + bool_to_string(n->run(w1)) + "\n").c_str());
    printf((w2 + " : DFA - " + bool_to_string(n->run(w2)) + "\n").c_str());
    printf((w3 + " : DFA - " + bool_to_string(n->run(w3)) + "\n").c_str());
    printf((w4 + " : DFA - " + bool_to_string(n->run(w4)) + "\n").c_str());
    printf((w5 + " : DFA - " + bool_to_string(n->run(w5)) + "\n").c_str());
    printf((w6 + " : DFA - " + bool_to_string(n->run(w6)) + "\n").c_str());

}