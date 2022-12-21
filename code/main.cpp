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
    int numStates = 0;
    STATE start_state;
    // set<STATE> final_states'
    STATE accept_state;
    STATE reject_state;
    deque<map<char, tuple<STATE, DIRECTION>>> transitions;
public:
    Two_DFA() {}

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
        accept_state = q;
    }

    void makeReject(STATE q){
        reject_state = q;
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
        while (accept_state != currentState && reject_state != currentState){
            map<char, tuple<STATE, DIRECTION>> ts = transitions[currentState];
            if (ts.find(word[pos]) == ts.end()) return false;

            tuple<STATE, DIRECTION> transition = ts[word[pos]];
            STATE newState = get<0>(transition);
            DIRECTION dir = get<1>(transition);

            currentState = newState;
            pos += dir;
        }
        return accept_state == currentState;
    }
};

string bool_to_string(bool b){
    return b? "True" : "False";
}

int main(){
    unique_ptr<Two_DFA> m = make_unique<Two_DFA>();
    deque<STATE> states = m->addStates(7);
    m->makeStart(states[0]);
    m->makeAccept(states[5]);
    m->makeReject(states[6]);
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

    string w1 = "abbaaabbabababbaa"; //true
    string w2 = "aabababbabbbabbab"; //false
    string w3 = "abbababbabbbabbab"; //false
    string w4 = "aabababbabababbab"; //false

    printf((w1 + " : " + bool_to_string(m->run(w1)) + "\n").c_str());
    printf((w2 + " : " + bool_to_string(m->run(w2)) + "\n").c_str());
    printf((w3 + " : " + bool_to_string(m->run(w3)) + "\n").c_str());
    printf((w4 + " : " + bool_to_string(m->run(w4)) + "\n").c_str());

}