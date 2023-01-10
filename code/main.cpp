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
        return alphabet;
    }

    int getStates(){
        return numStates;
    }

    STATE getStartState(){
        return start_state;
    }

    set<STATE> get_accept_states(){
        return accept_states;
    }

    deque<map<char, tuple<STATE, DIRECTION>>> getTransitions(){
        return transitions;
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
    STATE start_state;
    set<STATE> accept_states;
    deque<map<char, STATE>> transitions;
public:
    int numStates = 0;
    DFA(string _alphabet) {
        for (char ch : _alphabet) {
            alphabet.insert(ch);
        }
    }
    DFA(set<char> _alphabet) {
        alphabet = _alphabet;
    }

    void setAlpha(set<char> alpha){
        alphabet = alpha;
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
        if (alphabet.find(ch) != alphabet.end() || ch == LEFT_ENDMARKER || ch == RIGHT_ENDMARKER){
            transitions[start].insert_or_assign(ch, end);
            return true;
        }
        return false;
    }

    void removeEndmarkers(){
        deque<map<char, STATE>> new_transitions;
        set<STATE> new_accepts;
        makeStart(transitions[start_state][LEFT_ENDMARKER]);
        for (int i = 0; i < numStates; i++){
            map<char, STATE> t;
            for (auto kv : transitions[i]){
                if (kv.first == RIGHT_ENDMARKER && accept_states.find(kv.second) != accept_states.end()){
                    new_accepts.insert(i);
                    printf((to_string(i) + " Is an accept state\n").c_str());
                }
                else if (alphabet.find(kv.first) != alphabet.end()){
                    t.insert_or_assign(kv.first, transitions[i][kv.first]);
                    string s = "Added transition: D(" + to_string(i) + "," + kv.first + ") = " + to_string(transitions[i][kv.first]) + "\n";
                    printf(s.c_str());
                }
            }
            transitions.push_back(move(t));
        }
        transitions.clear();
        for (auto t : new_transitions){
            transitions.push_back(t);
        }
        accept_states.clear();
        for (auto s : new_accepts){
            accept_states.insert(s);
        }
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
    map<tuple<STATE, int>, STATE> new_states;

    alphabet.insert(LEFT_ENDMARKER);
    alphabet.insert(RIGHT_ENDMARKER);

    STATE trap_state = D->addState();
    for (auto ch : alphabet){
        D->addTransition(trap_state, ch, trap_state);
    }

    STATE R = -1;
    deque<STATE*> deltas;
    STATE delta0[numStates];
    for (int j = 0; j < numStates; j++){
        delta0[j] = R;
    }
    deltas.push_back(delta0);


    STATE current_state = 0;
    int current_delta = 0;
    new_states.insert({make_tuple(current_state, current_delta), D->addState()});
    deque<tuple<STATE, int>> new_state_queue;
    new_state_queue.push_back(make_tuple(current_state, current_delta));
    while (!new_state_queue.empty()){
        tuple<STATE, int> tmp = new_state_queue.front();
        new_state_queue.pop_front();
        current_state = get<0>(tmp);
        current_delta = get<1>(tmp);

        for (auto ch : alphabet){
            STATE *new_delta = (STATE*) malloc(sizeof(STATE) * numStates);
            for (STATE i = 0; i < numStates; i++){
                deque<STATE> sequence;
                sequence.push_back(i);
                while (sequence.size() < numStates + 1){
                    tuple<STATE, DIRECTION> next_tuple = transitions[sequence.back()][ch];
                    STATE next_state = get<0>(next_tuple);
                    DIRECTION dir = get<1>(next_tuple);

                    if (dir == RIGHT){
                        new_delta[i] = next_state;
                        break;
                    }
                    else if (dir == STAY){
                        sequence.push_back(next_state);
                    }
                    else if (dir == LEFT){
                        if (deltas[current_delta][next_state] == R){
                            new_delta[i] = R;
                            break;
                        }
                        else {
                            sequence.push_back(deltas[current_delta][next_state]);

                        }
                    }
                }
                if (sequence.size() == numStates + 1){
                    new_delta[i] = R;
                }
            }
            int d;
            bool is_delta;
            for (d = 0; d < deltas.size(); d++){
                is_delta = true;
                for (int s = 0; s < numStates; s++){
                    is_delta = is_delta && deltas[d][s] == new_delta[s];
                }
                if (is_delta) break;
            }

            if (!is_delta){
                deltas.push_back(new_delta);
            }

            STATE new_state = new_delta[current_state];
            if (new_state == R){
                D->addTransition(new_states[make_tuple(current_state,current_delta)], ch, trap_state);
            }
            else{
                if (new_states.find(make_tuple(new_state,d)) == new_states.end()){
                    new_states.insert({make_tuple(new_state,d), D->addState()});
                    new_state_queue.push_back(make_tuple(new_state, d));
                }
                D->addTransition(new_states[make_tuple(current_state,current_delta)], ch, new_states[make_tuple(new_state,d)]);
            }
        }
    }

    D->makeStart(new_states[make_tuple(0,0)]);
    for (auto itr : accept_states){
        for (int j = 0; j < numDeltas; j++){
            if (new_states.find(make_tuple(itr, j)) != new_states.end()){
                D->makeAccept(new_states[make_tuple(itr,j)]);
            }
        }
    }

    D->removeEndmarkers();

    return D;
}







void test(){
    Two_DFA *m = new Two_DFA("ab");
    deque<STATE> states = m->addStates(6);
    m->makeStart(states[0]);
    m->addTransition(states[0], LEFT_ENDMARKER, states[0], RIGHT);
    m->addTransition(states[0], RIGHT_ENDMARKER, states[2], STAY);
    m->addTransition(states[0], 'a', states[1], RIGHT);
    m->addTransition(states[0], 'b', states[0], RIGHT);

    m->addTransition(states[1], LEFT_ENDMARKER, states[1], RIGHT);
    m->addTransition(states[1], RIGHT_ENDMARKER, states[3], RIGHT);
    m->addTransition(states[1], 'a', states[0], RIGHT);
    m->addTransition(states[1], 'b', states[1], RIGHT);

    m->addTransition(states[2], LEFT_ENDMARKER, states[5], RIGHT);
    m->addTransition(states[2], RIGHT_ENDMARKER, states[2], LEFT);
    m->addTransition(states[2], 'a', states[2], LEFT);
    m->addTransition(states[2], 'b', states[4], LEFT);

    m->addTransition(states[3], LEFT_ENDMARKER, states[3], RIGHT);
    m->addTransition(states[3], RIGHT_ENDMARKER, states[3], RIGHT);
    m->addTransition(states[3], 'a', states[3], RIGHT);
    m->addTransition(states[3], 'b', states[3], RIGHT);

    m->addTransition(states[4], LEFT_ENDMARKER, states[3], LEFT);
    m->addTransition(states[4], RIGHT_ENDMARKER, states[4], LEFT);
    m->addTransition(states[4], 'a', states[4], LEFT);
    m->addTransition(states[4], 'b', states[2], LEFT);

    m->addTransition(states[5], LEFT_ENDMARKER, states[5], RIGHT);
    m->addTransition(states[5], RIGHT_ENDMARKER, states[5], RIGHT);
    m->addTransition(states[5], 'a', states[5], RIGHT);
    m->addTransition(states[5], 'b', states[5], RIGHT);

    m->makeAccept(states[5]);

    DFA* n = convert_to_DFA(m);
    printf("%d\n", n->numStates);

    string w1 = "aabb"; //true
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
    printf((w6 + " : 2DFA - " + bool_to_string(m->run(w6)) + "\n\n").c_str());

    printf((w1 + " : DFA - " + bool_to_string(n->run(w1)) + "\n").c_str());
    printf((w2 + " : DFA - " + bool_to_string(n->run(w2)) + "\n").c_str());
    printf((w3 + " : DFA - " + bool_to_string(n->run(w3)) + "\n").c_str());
    printf((w4 + " : DFA - " + bool_to_string(n->run(w4)) + "\n").c_str());
    printf((w5 + " : DFA - " + bool_to_string(n->run(w5)) + "\n").c_str());
    printf((w6 + " : DFA - " + bool_to_string(n->run(w6)) + "\n").c_str());
}

int main(){
    
    // test();

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

    DFA *n = convert_to_DFA(m);
    printf("%d\n", n->numStates);
    // deque<STATE> states2 = n->addStates(6);
    // n->makeStart(states[0]);
    // n->makeAccept(states[0]);
    // n->addTransition(states[0], 'a', states[1]);
    // n->addTransition(states[1], 'a', states[2]);
    // n->addTransition(states[2], 'a', states[0]);
    // n->addTransition(states[0], 'b', states[3]);
    // n->addTransition(states[1], 'b', states[4]);
    // n->addTransition(states[2], 'b', states[5]);
    // n->addTransition(states[3], 'a', states[4]);
    // n->addTransition(states[4], 'a', states[5]);
    // n->addTransition(states[5], 'a', states[3]);
    // n->addTransition(states[3], 'b', states[0]);
    // n->addTransition(states[4], 'b', states[1]);
    // n->addTransition(states[5], 'b', states[2]);

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

    printf((w1 + " : DFA - " + bool_to_string(n->run("<" + w1 + ">")) + "\n").c_str());
    printf((w2 + " : DFA - " + bool_to_string(n->run("<" + w2 + ">")) + "\n").c_str());
    printf((w3 + " : DFA - " + bool_to_string(n->run("<" + w3 + ">")) + "\n").c_str());
    printf((w4 + " : DFA - " + bool_to_string(n->run("<" + w4 + ">")) + "\n").c_str());
    printf((w5 + " : DFA - " + bool_to_string(n->run("<" + w5 + ">")) + "\n").c_str());
    printf((w6 + " : DFA - " + bool_to_string(n->run("<" + w6 + ">")) + "\n").c_str());

}