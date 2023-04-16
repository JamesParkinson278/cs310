#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <memory.h>
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

    void addTransition(STATE start, char ch, STATE end, DIRECTION dir, string out, bool verbose = false){
        tuple<STATE, DIRECTION, string> output = make_tuple(end, dir, out);
        transitions[start].insert({ch, output});
        if (verbose) printf("Added transition: (%d, %c) -> (%d, %d, %s)\n", start, ch, end, dir, out.c_str());
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


// typedef tuple<STATE, //q_a
//               STATE, //q_b
//               tuple<deque<tuple<STATE, STATE>>, STATE>, //q_eba
//               deque<tuple<STATE, STATE>>, //q_ebb
//               string, //w
//               int,    //i
//               int     //k
//             > new_state_type;

struct ELR {
    deque<tuple<STATE, STATE>> paths;
    STATE initial;
    char ch;
};
bool operator ==(const ELR& x, const ELR& y) {
    return std::tie(x.paths, x.initial, x.ch) == std::tie(y.paths, y.initial, y.ch);
}
string to_string(ELR e){
    string str = "(";
    for (auto path : e.paths){
        str += "(" + to_string(get<0>(path)) + "," + to_string(get<1>(path)) + ")";
    }
    str += ", " + to_string(e.initial) + ", " + to_string(e.ch) + ")";
    return str;
}

struct ERL {
    deque<tuple<STATE, STATE>> paths;
    char ch;
};
bool operator ==(const ERL& x, const ERL& y) {
    return std::tie(x.paths, x.ch) == std::tie(y.paths, y.ch);
}
string to_string(ERL e){
    string str = "(";
    for (auto path : e.paths){
        str += "(" + to_string(get<0>(path)) + "," + to_string(get<1>(path)) + ")";
    }
    str += ", " + to_string(e.ch) + ")";
    return str;
}

struct new_state_struct {
    STATE q_A;
    STATE q_B;
    ELR q_LR;
    ERL q_RL;
    string w;
    int i;
    int k;
    STATE id;
    deque<tuple<STATE, STATE>> candidates;
    STATE initial;
};
bool operator ==(const new_state_struct& x, const new_state_struct& y) {
    return std::tie(x.q_A, x.q_B, x.q_LR, x.q_RL, x.w, x.i, x.k, x.candidates, x.initial) == std::tie(y.q_A, y.q_B, y.q_LR, y.q_RL, y.w, y.i, y.k, y.candidates, y.initial);
}
string to_string(deque<tuple<STATE, STATE>> candidates){
    string str = "(";
    for (auto c : candidates){
        str += "(" + to_string(get<0>(c)) + "," + to_string(get<1>(c)) + ")";
    }
    str += ")";
    return str;
}
string to_string(new_state_struct s){
    string str = "(" + to_string(s.q_A) + ", " + to_string(s.q_B) + ", " + to_string(s.q_LR) + ", " + to_string(s.q_RL) + ", " + s.w + ", " + to_string(s.i) + ", " + to_string(s.k) + ", " + to_string(s.id) + ", " + to_string(s.candidates) + ", " + to_string(s.initial) + ")";
    return str;
}

new_state_struct createNewState(Two_DFT *C, deque<new_state_struct>*** new_states, deque<new_state_struct>* new_state_queue, STATE q_A, STATE q_B, ELR q_LR, ERL q_RL, string u, int i, int k){
    new_state_struct new_state;
    new_state.q_A = q_A;
    new_state.q_B = q_B;
    new_state.q_LR = q_LR;
    new_state.q_RL = q_RL;
    new_state.w = u;
    new_state.i = i;
    new_state.k = k;

    deque<tuple<STATE, STATE>> empty;
    new_state.candidates = empty;
    new_state.initial = -1;

    deque<new_state_struct> arr = *new_states[q_A][q_B];
    int j;
    for (j = 0; j < arr.size(); j++){
        if (arr[j] == new_state){
            break;
        }
    }

    if (j == arr.size()){
        new_state.id = C->addState();
        new_states[q_A][q_B]->push_back(new_state);
        new_state_queue->push_back(new_state);
        return new_state;
    }

    return arr[j];
}

new_state_struct createNewState(Two_DFT *C, deque<new_state_struct>*** new_states, deque<new_state_struct>* new_state_queue, STATE q_A, STATE q_B, ELR q_LR, ERL q_RL, string u, int i, int k, deque<tuple<STATE, STATE>> candidates, STATE initial){
    new_state_struct new_state;
    new_state.q_A = q_A;
    new_state.q_B = q_B;
    new_state.q_LR = q_LR;
    new_state.q_RL = q_RL;
    new_state.w = u;
    new_state.i = i;
    new_state.k = k;
    new_state.candidates = candidates;
    new_state.initial = initial;

    deque<new_state_struct> arr = *new_states[q_A][q_B];
    int j;
    for (j = 0; j < arr.size(); j++){
        if (arr[j] == new_state){
            break;
        }
    }

    if (j == arr.size()){
        new_state.id = C->addState();
        new_states[q_A][q_B]->push_back(new_state);
        new_state_queue->push_back(new_state);
        return new_state;
    }

    return arr[j];
}


Two_DFT *compose(Two_DFT *B, Two_DFT *A, bool v = false){
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
    int initial_state_A = A->getStartState();
    int initial_state_B = B->getStartState();
    int accept_state_A = A->get_accept_state();
    int accept_state_B = B->get_accept_state();
    auto transitions_A = A->getTransitions();
    auto transitions_B = B->getTransitions();
    if (v) printf("variables initialised\n");

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
    if (v) printf("new dft created\n");
    STATE accept_state_C = C->addState();
    C->makeAccept(accept_state_C);
    
    //generate the alphabet that represents the configuration graph
    map<char, set<tuple<STATE, STATE, DIRECTION, string>>> graph_alphabet;
    for (auto ch : input_alphabet){
        set<tuple<STATE, STATE, DIRECTION, string>> alpha;
        for (STATE s = 0; s < num_states_A; s++){
            tuple<STATE, DIRECTION, string> transition = transitions_A[s][ch];
            string w = get<2>(transition);
            if (s == initial_state_A && ch == LEFT_ENDMARKER) w.insert(0, 1, LEFT_ENDMARKER);
            tuple<STATE, STATE, DIRECTION, string> t = make_tuple(s, get<0>(transition), get<1>(transition), w);
            alpha.insert(t);
        }
        graph_alphabet.insert({ch, alpha});
    }
    if (v) printf("graph alphabet generated\n");

    // map<new_state_type,STATE> new_states;
    deque<new_state_struct> ***new_states = (deque<new_state_struct> ***) malloc(num_states_A * sizeof(deque<new_state_struct> **));
    for (int i = 0; i < num_states_A; i++){
        new_states[i] = (deque<new_state_struct> **) malloc(num_states_B * sizeof(deque<new_state_struct> *));
        for (int j = 0; j < num_states_B; j++){
            new_states[i][j] = new deque<new_state_struct>();
        }
    }
    deque<new_state_struct> *new_state_queue = new deque<new_state_struct>();

    deque<tuple<STATE, STATE>> empty;
    // tuple<deque<tuple<STATE, STATE>>, STATE> q_EBA0 = make_tuple(empty, -1);
    // deque<tuple<STATE, STATE>> q_EBB0;
    ELR q_LR0;
    q_LR0.ch = -1;
    q_LR0.initial = -1;
    q_LR0.paths = empty;
    ERL q_RL0;
    q_RL0.ch = -1;
    q_RL0.paths = empty;

    // new_state_type initial_state_C = make_tuple(initial_state_A, initial_state_B, q_LR0, q_RL0, "", 0, 1);
    // new_states.insert({initial_state_C, C->addState()});
    // new_state_queue.push_back(initial_state_C);
    new_state_struct initial_state_C = createNewState(C, new_states, new_state_queue, initial_state_A, initial_state_B, q_LR0, q_RL0, "", 0, 1);
    C->makeStart(initial_state_C.id);

    //main bulk of transition function
    while (!new_state_queue->empty()){
        printf("%d\n", new_state_queue->size());
        //get all the info about the current state
        new_state_struct current_state = new_state_queue->front();
        new_state_queue->pop_front();

        if (current_state.q_B == accept_state_B && current_state.k == 1){
            for (auto ch : input_alphabet){
                C->addTransition(current_state.id, ch, accept_state_C, STAY, "", true);
                // fprintf(stdout, (to_string(current_state) + "\n" + to_string(accept_state_C) + "\n").c_str());
            }
            continue;
        }

        for (auto ch : input_alphabet){
            if (current_state.k == 1){
                if (current_state.i == current_state.w.length()){
                    tuple<STATE, DIRECTION, string> transition = transitions_A[current_state.q_A][ch];
                    STATE q_Ap = get<0>(transition);
                    DIRECTION d = get<1>(transition);
                    string u = get<2>(transition);
                    if (q_Ap == accept_state_A){
                        u.push_back(RIGHT_ENDMARKER);
                        new_state_struct new_state = createNewState(C, new_states, new_state_queue, q_Ap, current_state.q_B, current_state.q_LR, current_state.q_RL, u, 0, 1);
                        C->addTransition(current_state.id, ch, new_state.id, STAY, "", true);
                        // fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                    }
                    else if (d == STAY){
                        new_state_struct new_state = createNewState(C, new_states, new_state_queue, q_Ap, current_state.q_B, current_state.q_LR, current_state.q_RL, u, 0, 1);
                        C->addTransition(current_state.id, ch, new_state.id, STAY, "", true);
                        // fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());

                        // C->addTransition(new_states[current_state], ch, new_states[new_state], STAY, "");
                    }
                    else if (d == RIGHT && ch != RIGHT_ENDMARKER){
                        ELR q_LRp;
                        q_LRp.ch = ch;
                        STATE initial = current_state.q_LR.initial;
                        char ch2 = current_state.q_LR.ch;
                        deque<tuple<STATE, STATE>> paths = current_state.q_LR.paths;
                        deque<tuple<STATE, STATE>> new_paths;
                        for (STATE s = 0; s < num_states_A; s++){
                            STATE cur = s;
                            new_paths.push_back(make_tuple(s,cur));
                            while (get<1>(transitions_A[cur][ch]) != RIGHT){
                                if (get<1>(transitions_A[cur][ch]) == STAY){
                                    cur = get<0>(transitions_A[cur][ch]);
                                    // new_paths.push_back(make_tuple(s, cur));
                                }
                                else if (get<1>(transitions_A[cur][ch]) == LEFT){
                                    STATE sp = get<0>(transitions_A[cur][ch]);
                                    for (auto path : paths){
                                        if (ch2 != -1 && get<0>(path) == sp && get<1>(transitions_A[get<1>(path)][ch2]) == RIGHT){
                                            cur = get<0>(transitions_A[get<1>(path)][ch2]);
                                            // new_paths.push_back(make_tuple(s,cur));
                                        }
                                    }
                                }
                            }
                        }
                        q_LRp.paths = new_paths;
                        STATE prev;
                        STATE cur;
                        if (ch2 == -1){
                            cur = current_state.q_A;
                        }
                        else{
                            cur = get<0>(transitions_A[initial][ch2]);
                        }
                        do {
                            prev = cur;
                            for (auto path : new_paths){
                                if (get<0>(path) != get<1>(path) && get<0>(path) == cur) {
                                    cur = get<1>(path);
                                    break;
                                }
                            }
                        } while (cur != prev);
                        q_LRp.initial = cur;

                        new_state_struct new_state = createNewState(C, new_states, new_state_queue, q_Ap, current_state.q_B, q_LRp, current_state.q_RL, u, 0, 2);
                        C->addTransition(current_state.id, ch, new_state.id, RIGHT, "", true);
                        // fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                    }
                    else if (d == LEFT && ch != LEFT_ENDMARKER){
                        ERL q_RLp;
                        q_RLp.ch = ch;
                        char ch2 = current_state.q_RL.ch;
                        deque<tuple<STATE, STATE>> paths = current_state.q_RL.paths;
                        deque<tuple<STATE, STATE>> new_paths;
                        for (STATE s = 0; s < num_states_A; s++){
                            STATE cur = s;
                            new_paths.push_back(make_tuple(s,cur));
                            while (get<1>(transitions_A[cur][ch]) != LEFT){
                                if (get<1>(transitions_A[cur][ch]) == STAY){
                                    cur = get<0>(transitions_A[cur][ch]);
                                    // new_paths.push_back(make_tuple(s, cur));
                                }
                                else if (get<1>(transitions_A[cur][ch]) == RIGHT){
                                    STATE sp = get<0>(transitions_A[cur][ch]);
                                    for (auto path : paths){
                                        if (ch2 != -1 && get<0>(path) == sp && get<1>(transitions_A[get<1>(path)][ch2]) == LEFT){
                                            cur = get<0>(transitions_A[get<1>(path)][ch2]);
                                            // new_paths.push_back(make_tuple(s,cur));
                                        }
                                    }
                                }
                            }
                        }
                        q_RLp.paths = new_paths;

                        new_state_struct new_state = createNewState(C, new_states, new_state_queue, q_Ap, current_state.q_B, current_state.q_LR, q_RLp, u, 0, 4);
                        C->addTransition(current_state.id, ch, new_state.id, LEFT, "", true);
                        // fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                    }
                }
                else if (current_state.i >= 0){
                    tuple<STATE, DIRECTION, string> transition = transitions_B[current_state.q_B][current_state.w[current_state.i]];
                    STATE q_Bp = get<0>(transition);
                    DIRECTION d = get<1>(transition);
                    string u = get<2>(transition);
                    new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, q_Bp, current_state.q_LR, current_state.q_RL, current_state.w, current_state.i + d, 1);
                    C->addTransition(current_state.id, ch, new_state.id, STAY, u, true);
                    // fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                }
            }
            else if (current_state.k == 2){
                if (current_state.q_RL.ch == -1 || current_state.q_RL.ch == ch){
                    if (ch == RIGHT_ENDMARKER){
                        new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, current_state.q_B, current_state.q_LR, q_RL0, current_state.w, current_state.i, 1);
                        C->addTransition(current_state.id, ch, new_state.id, STAY, "", true);
                        // fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                    }
                    else if (ch != LEFT_ENDMARKER) {
                        ERL q_RLp;
                        q_RLp.ch = ch;
                        q_RLp.paths = current_state.q_RL.paths;
                        new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, current_state.q_B, current_state.q_LR, q_RLp, current_state.w, current_state.i, 3);
                        C->addTransition(current_state.id, ch, new_state.id, RIGHT, "", true);
                        // fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                    }
                }
            }
            else if (current_state.k == 3 && ch != LEFT_ENDMARKER){
                ERL q_RLp;
                q_RLp.ch = ch;
                deque<tuple<STATE, STATE>> paths = current_state.q_RL.paths;
                deque<tuple<STATE, STATE>> new_paths;
                for (STATE s = 0; s < num_states_A; s++){
                    new_paths.push_back(make_tuple(s,s));
                }
                for (auto path : paths){
                    STATE cur = get<0>(path);
                    while (get<1>(transitions_A[cur][current_state.q_RL.ch]) == STAY){
                        cur = get<0>(transitions_A[cur][current_state.q_RL.ch]);
                    }
                    if (get<1>(transitions_A[cur][current_state.q_RL.ch]) == RIGHT){
                        STATE s1 = get<0>(transitions_A[cur][current_state.q_RL.ch]);
                        for (STATE s2 = 0; s2 < num_states_A; s2++){ //THIS WONT WORK COMPLETELY COS THERE'S MULTIPLE OPTIONS
                            if (get<0>(transitions_A[s2][ch]) == get<1>(path) && get<1>(transitions_A[s2][ch]) == LEFT && s1 != s2){
                                new_paths.push_back(make_tuple(s1,s2));
                                break;
                            }
                        }
                    }
                }
                q_RLp.paths = new_paths;
                new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, current_state.q_B, current_state.q_LR, q_RLp, current_state.w, 0, 1);
                C->addTransition(current_state.id, ch, new_state.id, LEFT, "", true);
                // fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
            }   
            else if (current_state.k == 4){
                if (current_state.q_LR.ch == -1 || current_state.q_LR.ch == ch){
                    if (ch == LEFT_ENDMARKER){
                        new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, current_state.q_B, q_LR0, current_state.q_RL, current_state.w, current_state.i, 1);
                        C->addTransition(current_state.id, ch, new_state.id, STAY, "", true);
                        // fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                    }
                    else if (ch != RIGHT_ENDMARKER) {
                        ELR q_LRp;
                        q_LRp.ch = ch;
                        q_LRp.paths = current_state.q_LR.paths;
                        q_LRp.initial = current_state.q_LR.initial;

                        ERL q_RLp;
                        q_RLp.ch = ch;
                        char ch2 = current_state.q_RL.ch;
                        deque<tuple<STATE, STATE>> paths = current_state.q_RL.paths;
                        deque<tuple<STATE, STATE>> new_paths;
                        for (STATE s = 0; s < num_states_A; s++){
                            STATE cur = s;
                            new_paths.push_back(make_tuple(s,cur));
                            while (get<1>(transitions_A[cur][ch]) != LEFT){
                                if (get<1>(transitions_A[cur][ch]) == STAY){
                                    cur = get<0>(transitions_A[cur][ch]);
                                    // new_paths.push_back(make_tuple(s, cur));
                                }
                                else if (get<1>(transitions_A[cur][ch]) == RIGHT){
                                    STATE sp = get<0>(transitions_A[cur][ch]);
                                    for (auto path : paths){
                                        if (ch2 != -1 && get<0>(path) == sp && get<1>(transitions_A[get<1>(path)][ch2]) == LEFT){
                                            cur = get<0>(transitions_A[get<1>(path)][ch2]);
                                            // new_paths.push_back(make_tuple(s,cur));
                                        }
                                    }
                                }
                            }
                        }
                        q_RLp.paths = new_paths;

                        deque<tuple<STATE, STATE>> candidates;
                        new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, current_state.q_B, q_LRp, q_RLp, current_state.w, 0, 5, candidates, current_state.q_LR.initial);
                        C->addTransition(current_state.id, ch, new_state.id, LEFT, "", true);    
                        // fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());                
                    }
                }
            }
            else if (current_state.k == 5 && ch != RIGHT_ENDMARKER){
                ELR q_LRp;
                q_LRp.ch = ch;
                deque<tuple<STATE, STATE>> paths = current_state.q_LR.paths;
                deque<tuple<STATE, STATE>> new_paths;
                for (STATE s = 0; s < num_states_A; s++){
                    new_paths.push_back(make_tuple(s,s));
                }
                for (auto path : paths){
                    STATE cur = get<0>(path);
                    while (get<1>(transitions_A[cur][current_state.q_LR.ch]) == STAY){
                        cur = get<0>(transitions_A[cur][current_state.q_LR.ch]);
                    }
                    if (get<1>(transitions_A[cur][current_state.q_LR.ch]) == LEFT){
                        deque<STATE> starts;
                        STATE s1 = get<0>(transitions_A[cur][current_state.q_LR.ch]);
                        do {
                            starts.push_back(s1);
                            s1 = get<0>(transitions_A[s1][ch]);
                        } while (get<1>(transitions_A[s1][ch]) == STAY);
                        for (STATE s2 = 0; s2 < num_states_A; s2++){ //THIS WONT WORK COMPLETELY COS THERE COULD BE MULTIPLE OPTIONS
                            if (get<0>(transitions_A[s2][ch]) == get<1>(path) && get<1>(transitions_A[s2][ch]) == RIGHT){
                                for (STATE s3 : starts){
                                    new_paths.push_back(make_tuple(s3,s2));
                                }
                                break;
                            } CHANGE ALL OF THIS 
                        }
                    }
                }
                q_LRp.paths = new_paths;
                // new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, current_state.q_B, q_LRp, current_state.q_LR, current_state.w, 0, 1);
                // C->addTransition(current_state.id, ch, new_state.id, RIGHT, "", true);
                // fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());


                if (current_state.candidates.size() == 0){
                     for (STATE s = 0; s < num_states_A; s++){
                         tuple<STATE, DIRECTION, string> t1 = transitions_A[s][ch];
                         if (get<0>(t1) == current_state.initial && get<1>(t1) == RIGHT){
                             current_state.candidates.push_back(make_tuple(s,s));
                         }
                     }
                }

                if (current_state.candidates.size() == 1){
                    q_LRp.initial = get<0>(current_state.candidates.front());
                }
            }
        }
    }

    return C;
}



void test(Two_DFT *B, Two_DFT *A, int numTests){
    srand(time(0));
    set<char> alpha = A->getInputAlpha();
    Two_DFT *C = compose(B, A, true);
    int states = C->getStates();
    printf("%d\n", states);
    int numRight = 0;

    for (int i = 0; i < numTests; i++){
        int len = rand() % states*2;
        // int len = rand() % 100;
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

        printf("Input w = %s\nB(A(w)) = %s\nC(w) = %s\n\n", input.c_str(), output1.c_str(), output2.c_str());
    }

    printf("# Correct Outputs: %d / %d\n", numRight, numTests);
    printf("%d\n", states);
}

void test1(){ //swap a and b, then replace ab with cd
    Two_DFT *m = new Two_DFT("ab", "ab");
    STATE q = m->addState();
    STATE accept = m->addState();
    m->makeAccept(accept);

    m->addTransition(q, LEFT_ENDMARKER, q, RIGHT, "");
    m->addTransition(q, RIGHT_ENDMARKER, accept, RIGHT, "");
    m->addTransition(q, 'a', q, RIGHT, "b");
    m->addTransition(q, 'b', q, RIGHT, "a");

    Two_DFT *n = new Two_DFT("ab", "cd");
    STATE p = n->addState();
    STATE accept2 = n->addState();
    n->makeAccept(accept);

    n->addTransition(p, LEFT_ENDMARKER, p, RIGHT, "");
    n->addTransition(p, RIGHT_ENDMARKER, accept2, RIGHT, "");
    n->addTransition(p, 'a', p, RIGHT, "c");
    n->addTransition(p, 'b', p, RIGHT, "d");

    printf("Starting composition\n");
    test(n,m,10000);
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

void test3(){ //swap every third letter then replace every other letter with cd
    Two_DFT *m = new Two_DFT("ab", "ab");
    deque<STATE> q = m->addStates(3);
    STATE accept = m->addState();
    m->makeAccept(accept);

    m->addTransition(q[0], LEFT_ENDMARKER, q[0], RIGHT, "");
    m->addTransition(q[0], RIGHT_ENDMARKER, accept, RIGHT, "");
    m->addTransition(q[0], 'a', q[1], RIGHT, "a");
    m->addTransition(q[0], 'b', q[1], RIGHT, "b");

    m->addTransition(q[1], LEFT_ENDMARKER, q[1], RIGHT, "");
    m->addTransition(q[1], RIGHT_ENDMARKER, accept, RIGHT, "");
    m->addTransition(q[1], 'a', q[2], RIGHT, "a");
    m->addTransition(q[1], 'b', q[2], RIGHT, "b");

    m->addTransition(q[2], LEFT_ENDMARKER, q[0], RIGHT, "");
    m->addTransition(q[2], RIGHT_ENDMARKER, accept, RIGHT, "");
    m->addTransition(q[2], 'a', q[0], RIGHT, "b");
    m->addTransition(q[2], 'b', q[0], RIGHT, "a");

    Two_DFT *n = new Two_DFT("ab", "abcd");
    deque<STATE> p = n->addStates(2);
    STATE accept2 = n->addState();
    n->makeAccept(accept2);

    n->addTransition(p[0], LEFT_ENDMARKER, p[0], RIGHT, "");
    n->addTransition(p[0], RIGHT_ENDMARKER, accept2, RIGHT, "");
    n->addTransition(p[0], 'a', p[1], RIGHT, "a");
    n->addTransition(p[0], 'b', p[1], RIGHT, "b");

    n->addTransition(p[1], LEFT_ENDMARKER, p[1], RIGHT, "");
    n->addTransition(p[1], RIGHT_ENDMARKER, accept2, RIGHT, "");
    n->addTransition(p[1], 'a', p[0], RIGHT, "c");
    n->addTransition(p[1], 'b', p[0], RIGHT, "d");

    printf("Starting composition\n");
    test(n,m,10000);
}

int main(){
    
    // test1();
    // test2();
    test3();
}