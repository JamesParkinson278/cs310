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
#pragma GCC diagnostic ignored "-Wformat="


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
        transitions.push_back(t);
        return newState;
    }

    deque<STATE> addStates(int n){
        deque<STATE> states;
        for (int i = 0; i < n; i++){
            states.push_back(addState());
        }
        return states;
    }

    void addTransition(STATE start, char ch, STATE end, DIRECTION dir, string out, bool verbose = true){
        tuple<STATE, DIRECTION, string> output = make_tuple(end, dir, out);
        transitions[start].insert({ch, output});
        if (verbose) printf("Added transition: (%d, %c) -> (%d, %d, %s)\n", start, ch, end, dir, out.c_str());
    }

    void makeAccept(STATE q){
        accept_state = q;
    }

    void makeStart(STATE q){
        start_state = q;
    }

    string run(string w, bool verbose = false){
        string output = "";
        STATE currentState = start_state;
        int pos = 0;     
        w.insert(0, 1, LEFT_ENDMARKER);
        w.push_back(RIGHT_ENDMARKER);
        const char* word = w.c_str();
        // set<tuple<STATE, int>> history;
        while (true){
            // if (history.find(make_tuple(currentState, pos)) != history.end()) {
            //     printf("Loop starts at config %d %d\n", currentState, pos);
            //     return "";
            // }
            // history.insert(make_tuple(currentState, pos));
            if (verbose) printf("(%d,%c,%d) ", currentState, word[pos], pos);
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
    set<tuple<STATE, STATE>> paths;
    STATE initial;
    char ch;
};
bool operator ==(const ELR& x, const ELR& y) {
    return std::tie(x.paths, x.initial, x.ch) == std::tie(y.paths, y.initial, y.ch);
}
string to_string(ELR e){
    string s = "(";
    for (auto path : e.paths){
        s += "(" + to_string(get<0>(path)) + "," + to_string(get<1>(path)) + ")";
    }
    s += ", " + to_string(e.initial) + ", " + string(1,e.ch) + ")";
    return s;
}

struct ERL {
    set<tuple<STATE, STATE>> paths;
    char ch;
};
bool operator ==(const ERL& x, const ERL& y) {
    return std::tie(x.paths, x.ch) == std::tie(y.paths, y.ch);
}
string to_string(ERL e){
    string s = "(";
    for (auto path : e.paths){
        s += "(" + to_string(get<0>(path)) + "," + to_string(get<1>(path)) + ")";
    }
    s += ", " + string(1,e.ch) + ")";
    return s;
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

ELR moveLRforward(char ch, new_state_struct current_state, int num_states_A, auto transitions_A, STATE initial_state_A){
    ELR q_LRp;
    q_LRp.ch = ch;
    STATE initial = current_state.q_LR.initial;
    char ch2 = current_state.q_LR.ch;
    set<tuple<STATE, STATE>> paths = current_state.q_LR.paths;
    set<tuple<STATE, STATE>> new_paths;
    for (STATE s = 0; s < num_states_A; s++){
        STATE cur = s;
        STATE prev = cur;
        new_paths.insert(make_tuple(s,cur));
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
            if (prev == cur) break;
            prev = cur;
        }
        if (cur != s) new_paths.insert(make_tuple(s,cur));
    }
    q_LRp.paths = new_paths;

    STATE new_initial;
    if (ch2 == -1){
        new_initial = initial_state_A;
    }
    else{
        STATE prev;
        STATE cur = initial;
        do {
            prev = cur;
            for (auto path : current_state.q_LR.paths){
                if (get<0>(path) != get<1>(path) && get<0>(path) == cur) {
                    cur = get<1>(path);
                    break;
                }
            }
        } while (cur != prev);
        new_initial = get<0>(transitions_A[cur][ch2]);
    }
    
    q_LRp.initial = new_initial;

    return q_LRp;
}

ERL moveRLforward(char ch, new_state_struct current_state, int num_states_A, auto transitions_A){
    ERL q_RLp;
    q_RLp.ch = ch;
    char ch2 = current_state.q_RL.ch;
    set<tuple<STATE, STATE>> paths = current_state.q_RL.paths;
    set<tuple<STATE, STATE>> new_paths;
    for (STATE s = 0; s < num_states_A; s++){
        STATE cur = s;
        STATE prev = cur;
        new_paths.insert(make_tuple(s,cur));
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
            if (prev == cur) break;
            prev = cur;
        }
        if (cur != s) new_paths.insert(make_tuple(s,cur));
    }
    q_RLp.paths = new_paths;
    return q_RLp;
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
    map<char, map<STATE, tuple<STATE, DIRECTION, string>>> graph_alphabet;
    for (auto ch : input_alphabet){
        map<STATE, tuple<STATE, DIRECTION, string>> alpha;
        for (STATE s = 0; s < num_states_A; s++){
            tuple<STATE, DIRECTION, string> transition = transitions_A[s][ch];
            string w = get<2>(transition);
            if (s == initial_state_A && ch == LEFT_ENDMARKER) w.insert(0, 1, LEFT_ENDMARKER);
            tuple<STATE, DIRECTION, string> t = make_tuple(get<0>(transition), get<1>(transition), w);
            alpha.insert({s,t});
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

    set<tuple<STATE, STATE>> empty;
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
    string w = get<2>(graph_alphabet[LEFT_ENDMARKER][initial_state_A]);
    new_state_struct initial_state_C = createNewState(C, new_states, new_state_queue, initial_state_A, initial_state_B, q_LR0, q_RL0, w, 0, 1);
    C->makeStart(initial_state_C.id);

    //main bulk of transition function
    while (!new_state_queue->empty()){
        // printf("%d\n", new_state_queue->size());
        //get all the info about the current state
        new_state_struct current_state = new_state_queue->front();
        new_state_queue->pop_front();

        if (current_state.q_B == accept_state_B && current_state.k == 1){
            for (auto ch : input_alphabet){
                C->addTransition(current_state.id, ch, accept_state_C, STAY, "");
                if (v) fprintf(stdout, (to_string(current_state) + "\n" + to_string(accept_state_C) + "\n").c_str());
            }
            continue;
        }

        for (auto ch : input_alphabet){
            if (current_state.k == 1){
                if (current_state.q_LR == q_LR0 && ch != LEFT_ENDMARKER || !(current_state.q_LR == q_LR0 || ch != LEFT_ENDMARKER)) continue;
                if (!(current_state.q_RL == q_RL0 || ch != RIGHT_ENDMARKER)) continue;
                if (current_state.i == current_state.w.length()){
                    tuple<STATE, DIRECTION, string> transition = transitions_A[current_state.q_A][ch];
                    STATE q_Ap = get<0>(transition);
                    DIRECTION d = get<1>(transition);
                    
                    if (d == STAY){
                        string u = get<2>(graph_alphabet[ch][q_Ap]);
                        if (get<0>(graph_alphabet[ch][q_Ap]) == accept_state_A){
                            u.push_back(RIGHT_ENDMARKER);
                        }
                        new_state_struct new_state = createNewState(C, new_states, new_state_queue, q_Ap, current_state.q_B, current_state.q_LR, current_state.q_RL, u, 0, 1);
                        C->addTransition(current_state.id, ch, new_state.id, STAY, "");
                        if (v) fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                    }
                    else if (d == RIGHT && ch != RIGHT_ENDMARKER){
                        ELR q_LRp = moveLRforward(ch, current_state, num_states_A, transitions_A, initial_state_A);
                        new_state_struct new_state = createNewState(C, new_states, new_state_queue, q_Ap, current_state.q_B, q_LRp, current_state.q_RL, "", 0, 2);
                        C->addTransition(current_state.id, ch, new_state.id, RIGHT, "");
                        if (v) fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                    }
                    else if (d == LEFT && ch != LEFT_ENDMARKER){
                        ERL q_RLp = moveRLforward(ch, current_state, num_states_A, transitions_A);
                        new_state_struct new_state = createNewState(C, new_states, new_state_queue, q_Ap, current_state.q_B, current_state.q_LR, q_RLp, "", 0, 4);
                        C->addTransition(current_state.id, ch, new_state.id, LEFT, "");
                        if (v) fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                    }
                }
                else if (current_state.i >= 0){
                    tuple<STATE, DIRECTION, string> transition = transitions_B[current_state.q_B][current_state.w[current_state.i]];
                    STATE q_Bp = get<0>(transition);
                    DIRECTION d = get<1>(transition);
                    string u = get<2>(transition);
                    new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, q_Bp, current_state.q_LR, current_state.q_RL, current_state.w, current_state.i + d, 1);
                    C->addTransition(current_state.id, ch, new_state.id, STAY, u);
                    if (v) fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                }
                else {
                    map<STATE, tuple<STATE, DIRECTION, string>> graph_col_left;
                    map<STATE, tuple<STATE, DIRECTION, string>> graph_col = graph_alphabet[ch];
                    map<STATE, tuple<STATE, DIRECTION, string>> graph_col_right;
                    if (current_state.q_LR.ch != -1){
                        graph_col_left = graph_alphabet[current_state.q_LR.ch];
                    }
                    if (current_state.q_RL.ch != -1){
                        graph_col_right = graph_alphabet[current_state.q_RL.ch];
                    }
                    deque<tuple<STATE, int, STATE, int>> candidates;
                    for (STATE s = 0; s < num_states_A; s++){
                        if (current_state.q_LR.ch != -1 && get<0>(graph_col_left[s]) == current_state.q_A && get<1>(graph_col_left[s]) == RIGHT){
                            candidates.push_back(make_tuple(s, -1, s, -1));
                        }
                        if (current_state.q_RL.ch != -1 && get<0>(graph_col_right[s]) == current_state.q_A && get<1>(graph_col_right[s]) == LEFT){
                            candidates.push_back(make_tuple(s, 1, s, 1));
                        }
                        if (get<0>(graph_col[s]) == current_state.q_A && get<1>(graph_col[s]) == STAY){
                            candidates.push_back(make_tuple(s, 0, s, 0));
                        }
                    }
                    // printf("%d %d\n",current_state.id, candidates.size());
                    while (candidates.empty() == false) {
                        STATE state = get<0>(candidates.front());
                        int pos = get<1>(candidates.front());
                        STATE origin_state = get<2>(candidates.front());
                        int origin_pos = get<3>(candidates.front());
                        candidates.pop_front();

                        if ((pos == -1 && state == current_state.q_LR.initial) || candidates.empty()){
                            int k;
                            string u = "";
                            ELR q_LRp = current_state.q_LR;
                            ERL q_RLp = current_state.q_RL;

                            if (origin_pos == -1){
                                k = 8;
                                q_RLp = moveRLforward(ch, current_state, num_states_A, transitions_A);
                            }
                            else if (origin_pos == 0){
                                k = 1;
                                u = get<2>(graph_alphabet[ch][origin_state]);
                                if (get<0>(graph_alphabet[ch][origin_state]) == accept_state_A){
                                    u.push_back(RIGHT_ENDMARKER);
                                }
                            }
                            else if (origin_pos == 1){
                                k = 7;
                                q_LRp = moveLRforward(ch, current_state, num_states_A, transitions_A, initial_state_A);
                            }
                            
                            new_state_struct new_state = createNewState(C, new_states, new_state_queue, origin_state, current_state.q_B, q_LRp, q_RLp, u, u.length() - 1, k);
                            C->addTransition(current_state.id, ch, new_state.id, (DIRECTION)origin_pos, "");
                            if (v) fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                            break;
                        }

                        if (pos == 0){
                            for (STATE s = 0; s < num_states_A; s++){
                                if (current_state.q_LR.ch != -1 && get<0>(graph_col_left[s]) == state && get<1>(graph_col_left[s]) == RIGHT){
                                    candidates.push_back(make_tuple(s,-1,origin_state,origin_pos));
                                }
                                if (current_state.q_RL.ch != -1 && get<0>(graph_col_right[s]) == state && get<1>(graph_col_right[s]) == LEFT){
                                    candidates.push_back(make_tuple(s,1,origin_state,origin_pos));
                                }
                                if (get<0>(graph_col[s]) == state && get<1>(graph_col[s]) == STAY){
                                    candidates.push_back(make_tuple(s,0,origin_state,origin_pos));
                                }
                            }
                        }
                        else if (pos == -1){
                            bool found_path = false;
                            for (auto path : current_state.q_LR.paths){
                                if (get<0>(path) != get<1>(path) && get<1>(path) == state){
                                    candidates.push_back(make_tuple(get<0>(path), pos, origin_state, origin_pos)); 
                                    found_path = true;
                                }
                            }
                            if (!found_path){
                                for (STATE s = 0; s < num_states_A; s++){
                                    if (get<0>(graph_col[s]) == state && get<1>(graph_col[s]) == LEFT){
                                        candidates.push_back(make_tuple(s,0,origin_state,origin_pos));
                                    }
                                }
                            }
                        }
                        else if (pos == 1){
                            bool found_path = false;
                            for (auto path : current_state.q_RL.paths){
                                if (get<0>(path) != get<1>(path) && get<1>(path) == state){
                                    candidates.push_back(make_tuple(get<0>(path), pos, origin_state, origin_pos)); 
                                    found_path = true;
                                }
                            }
                            if (!found_path){
                                for (STATE s = 0; s < num_states_A; s++){
                                    if (get<0>(graph_col[s]) == state && get<1>(graph_col[s]) == RIGHT){
                                        candidates.push_back(make_tuple(s,0,origin_state,origin_pos));
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else if (current_state.k == 2){
                if (current_state.q_RL.ch == -1 || current_state.q_RL.ch == ch){
                    string u = get<2>(graph_alphabet[ch][current_state.q_A]);
                    if (get<0>(graph_alphabet[ch][current_state.q_A]) == accept_state_A){
                        u.push_back(RIGHT_ENDMARKER);
                    }

                    if (ch == RIGHT_ENDMARKER){
                        new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, current_state.q_B, current_state.q_LR, q_RL0, u, 0, 1);
                        C->addTransition(current_state.id, ch, new_state.id, STAY, "");
                        if (v) fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                    }
                    else if (ch != LEFT_ENDMARKER) {
                        ERL q_RLp;
                        q_RLp.ch = ch;
                        q_RLp.paths = current_state.q_RL.paths;
                        new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, current_state.q_B, current_state.q_LR, q_RLp, u, 0, 3);
                        C->addTransition(current_state.id, ch, new_state.id, RIGHT, "");
                        if (v) fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                    }
                }
            }
            else if (current_state.k == 3 && ch != LEFT_ENDMARKER){
                ERL q_RLp;
                q_RLp.ch = ch;
                set<tuple<STATE, STATE>> paths = current_state.q_RL.paths;
                set<tuple<STATE, STATE>> new_paths;
                for (STATE s = 0; s < num_states_A; s++){
                    new_paths.insert(make_tuple(s,s));
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
                                new_paths.insert(make_tuple(s1,s2));
                                break;
                            }
                        }
                    }
                }

                q_RLp.paths = new_paths;
                new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, current_state.q_B, current_state.q_LR, q_RLp, current_state.w, current_state.i, 1);
                C->addTransition(current_state.id, ch, new_state.id, LEFT, "");
                if (v) fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
            }   
            else if (current_state.k == 4){
                if (current_state.q_LR.ch == -1 || current_state.q_LR.ch == ch){
                    string u = get<2>(graph_alphabet[ch][current_state.q_A]);
                    if (get<0>(graph_alphabet[ch][current_state.q_A]) == accept_state_A){
                        u.push_back(RIGHT_ENDMARKER);
                    }
                    if (ch == LEFT_ENDMARKER){
                        new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, current_state.q_B, q_LR0, current_state.q_RL, u, 0, 1);
                        C->addTransition(current_state.id, ch, new_state.id, STAY, "");
                        if (v) fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                    }
                    else if (ch != RIGHT_ENDMARKER) {
                        ELR q_LRp;
                        q_LRp.ch = ch;
                        q_LRp.paths = current_state.q_LR.paths;
                        q_LRp.initial = current_state.q_LR.initial;

                        ERL q_RLp = moveRLforward(ch, current_state, num_states_A, transitions_A);

                        deque<tuple<STATE, STATE>> candidates;
                        new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, current_state.q_B, q_LRp, q_RLp, u, 0, 5, candidates, current_state.q_LR.initial);
                        C->addTransition(current_state.id, ch, new_state.id, LEFT, "");    
                        if (v) fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                    }
                }
            }
            else if (current_state.k == 5 && ch != RIGHT_ENDMARKER){
                ELR q_LRp;
                q_LRp.ch = ch;
                set<tuple<STATE, STATE>> paths = current_state.q_LR.paths;
                set<tuple<STATE, STATE>> new_paths;
                for (STATE s = 0; s < num_states_A; s++){
                    new_paths.insert(make_tuple(s,s));
                }
                map<STATE, tuple<STATE, DIRECTION, string>> cur_graph_col = graph_alphabet[current_state.q_LR.ch];
                map<STATE, tuple<STATE, DIRECTION, string>> next_graph_col = graph_alphabet[ch];
                for (auto path : paths){
                    tuple<STATE, DIRECTION, string> t = cur_graph_col[get<0>(path)];
                    
                    if (get<1>(t) == LEFT){
                        deque<STATE> starts;
                        STATE end;
                        STATE s1 = get<0>(t);
                        do {
                            starts.push_back(s1);
                            s1 = get<0>(transitions_A[s1][ch]);
                        } while (get<1>(transitions_A[s1][ch]) == STAY);

                        if (get<1>(transitions_A[s1][ch]) == LEFT){
                            for (STATE s2 = 0; s2 < num_states_A; s2++){
                                auto t2 = transitions_A[s2][ch];
                                if (get<1>(t2) == RIGHT && get<0>(t2) == get<1>(path)){
                                    end = s2;
                                    break;
                                }
                            }
                            for (STATE s : starts){
                                new_paths.insert(make_tuple(s, end));
                            }
                        }
                    }
                }
                q_LRp.paths = new_paths;

                deque<tuple<STATE, STATE>> new_candidates;
                if (current_state.candidates.size() == 0){
                    for (STATE s = 0; s < num_states_A; s++){
                        tuple<STATE, DIRECTION, string> t1 = transitions_A[s][ch];
                        if (get<0>(t1) == current_state.initial && get<1>(t1) == RIGHT){
                            new_candidates.push_back(make_tuple(s,s));
                        }
                    }
                }
                else {
                    for (tuple<STATE,STATE> candidate : current_state.candidates){
                        set<STATE> states;
                        deque<STATE> curs;
                        curs.push_back(get<0>(candidate));
                        while (curs.empty() != true){
                            STATE cur = curs.front();
                            curs.pop_front();
                            states.insert(cur);
                            for (auto path : current_state.q_RL.paths){
                                if (get<0>(path) != get<1>(path) && get<1>(path) == cur) {
                                    curs.push_back(get<0>(path));
                                }
                            }
                        }
                        for (STATE s = 0; s < num_states_A; s++){
                            tuple<STATE, DIRECTION, string> t = next_graph_col[s];
                            if (states.count(get<0>(t)) != 0 && get<1>(t) == RIGHT){
                                new_candidates.push_back(make_tuple(s,get<1>(candidate)));
                            }
                        }
                    }
                }

                if (new_candidates.size() == 1){
                    q_LRp.initial = get<1>(new_candidates.front());
                    new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, current_state.q_B, q_LRp, current_state.q_RL, current_state.w, current_state.i, 6, current_state.candidates, current_state.initial);
                    C->addTransition(current_state.id, ch, new_state.id, RIGHT, "");
                    if (v) fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                }
                else if (ch == LEFT_ENDMARKER && new_candidates.size() != 0){
                    STATE s = initial_state_A;
                    bool isCandidate = false;
                    for (auto c : new_candidates){
                        isCandidate = isCandidate || get<0>(c) == s;
                    }
                    while (!isCandidate){
                        for (auto path : q_LRp.paths){
                            if (get<0>(path) != get<1>(path) && get<0>(path) == s){
                                s = get<1>(path);
                                break;
                            }
                        }
                        for (auto c : new_candidates){
                            isCandidate = isCandidate || get<0>(c) == s;
                        }
                    }
                    STATE connected;
                    for (auto c : new_candidates){
                        if (get<0>(c) == s){
                            q_LRp.initial = get<1>(c);
                            break;
                        }
                    }
                    new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, current_state.q_B, q_LRp, current_state.q_RL, current_state.w, current_state.i, 6, current_state.candidates, current_state.initial);
                    C->addTransition(current_state.id, ch, new_state.id, RIGHT, "");
                    if (v) fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());

                }
                else if (new_candidates.size() != 0){
                    ERL q_RLp = moveRLforward(ch, current_state, num_states_A, transitions_A);
                    new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, current_state.q_B, q_LRp, q_RLp, current_state.w, current_state.i, 5, new_candidates, current_state.initial);
                    C->addTransition(current_state.id, ch, new_state.id, LEFT, "");
                    if (v) fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                }
            }
            else if (current_state.k == 6 && ch == current_state.q_RL.ch){
                if (current_state.candidates.size() == 0 || current_state.candidates.size() == 1){
                    int k = 2;
                    if (current_state.i == -1){
                        k = 7;
                    }
                    new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, current_state.q_B, current_state.q_LR, current_state.q_RL, current_state.w, current_state.i, k);
                    C->addTransition(current_state.id, ch, new_state.id, STAY, "");
                    if (v) fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                }
            }
            else if (current_state.k == 7){
                if (current_state.q_RL.ch == -1 || current_state.q_RL.ch == ch){
                    string u = get<2>(graph_alphabet[ch][current_state.q_A]);
                    if (get<0>(graph_alphabet[ch][current_state.q_A]) == accept_state_A){
                        u.push_back(RIGHT_ENDMARKER);
                    }

                    if (ch == RIGHT_ENDMARKER){
                        new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, current_state.q_B, current_state.q_LR, q_RL0, u, u.length() - 1, 1);
                        C->addTransition(current_state.id, ch, new_state.id, STAY, "");
                        if (v) fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                    }
                    else if (ch != LEFT_ENDMARKER) {
                        ERL q_RLp;
                        q_RLp.ch = ch;
                        q_RLp.paths = current_state.q_RL.paths;
                        new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, current_state.q_B, current_state.q_LR, q_RLp, u, u.length() - 1, 3);
                        C->addTransition(current_state.id, ch, new_state.id, RIGHT, "");
                        if (v) fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                    }
                }
            }
            else if (current_state.k == 8){
                if (current_state.q_LR.ch == -1 || current_state.q_LR.ch == ch){
                    string u = get<2>(graph_alphabet[ch][current_state.q_A]);
                    if (get<0>(graph_alphabet[ch][current_state.q_A]) == accept_state_A){
                        u.push_back(RIGHT_ENDMARKER);
                    }
                    if (ch == LEFT_ENDMARKER){
                        new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, current_state.q_B, q_LR0, current_state.q_RL, u, u.length() - 1, 1);
                        C->addTransition(current_state.id, ch, new_state.id, STAY, "");
                        if (v) fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                    }
                    else if (ch != RIGHT_ENDMARKER) {
                        ELR q_LRp;
                        q_LRp.ch = ch;
                        q_LRp.paths = current_state.q_LR.paths;
                        q_LRp.initial = current_state.q_LR.initial;

                        ERL q_RLp = moveRLforward(ch, current_state, num_states_A, transitions_A);
                        deque<tuple<STATE, STATE>> candidates;
                        new_state_struct new_state = createNewState(C, new_states, new_state_queue, current_state.q_A, current_state.q_B, q_LRp, q_RLp, u, u.length() - 1, 5, candidates, current_state.q_LR.initial);
                        C->addTransition(current_state.id, ch, new_state.id, LEFT, "");    
                        if (v) fprintf(stdout, (to_string(current_state) + "\n" + to_string(new_state) + "\n").c_str());
                    }
                }
            }
        }
    }

    free(new_state_queue);
    for (int i = 0; i < num_states_A; i++){
        deque<new_state_struct> ** tmp = new_states[i];
        for (int j = 0; j < num_states_B; j++){
            free(tmp[j]);
        }
        free(tmp);
    }

    return C;
}



void test(Two_DFT *B, Two_DFT *A, int numTests){
    srand(time(0));
    set<char> alpha = A->getInputAlpha();
    Two_DFT *C = compose(B, A, true);
    printf("Composition done\n");
    int states = C->getStates();
    int numRight = 0;

    for (int i = 0; i < numTests; i++){
        int len = rand() % states*2;
        // int len = rand() % 6;
        string input = "";
        for (int j = 0; j < len; j++){
            int k = rand() % alpha.size();
            set<char>::iterator it = alpha.begin();
            for (;k != 0; k--) it++;
            char letter = *it;
            input += letter;
        }

        string output1 = B->run(A->run(input));
        // string output1 = "a";
        string output2 = C->run(input);

        if (output1 == output2){
            numRight += 1;
        }

        printf("Input w = %s\nB(A(w)) = %s\nC(w) = %s\n\n", input.c_str(), output1.c_str(), output2.c_str());
    }

    printf("# Correct Outputs: %d / %d\n", numRight, numTests);
    printf("%d\n", states);
    free(C);
}

void test1(){ //swap a and b, then replace ab with cd
    Two_DFT *m = new Two_DFT("ab", "ab");
    STATE q = m->addState();
    STATE accept = m->addState();
    m->makeAccept(accept);
    m->addTransition(accept, 'a', accept, RIGHT, "");
    m->addTransition(accept, 'b', accept, RIGHT, "");
    m->addTransition(accept, LEFT_ENDMARKER, accept, RIGHT, "");
    m->addTransition(accept, RIGHT_ENDMARKER, accept, RIGHT, "");
    m->makeStart(q);

    m->addTransition(q, LEFT_ENDMARKER, q, RIGHT, "");
    m->addTransition(q, RIGHT_ENDMARKER, accept, RIGHT, "");
    m->addTransition(q, 'a', q, RIGHT, "b");
    m->addTransition(q, 'b', q, RIGHT, "a");

    Two_DFT *n = new Two_DFT("ab", "cd");
    STATE p = n->addState();
    STATE accept2 = n->addState();
    n->makeAccept(accept);
    n->addTransition(accept2, 'a', accept2, RIGHT, "");
    n->addTransition(accept2, 'b', accept2, RIGHT, "");
    n->addTransition(accept2, LEFT_ENDMARKER, accept2, RIGHT, "");
    n->addTransition(accept2, RIGHT_ENDMARKER, accept2, RIGHT, "");
    n->makeStart(p);

    n->addTransition(p, LEFT_ENDMARKER, p, RIGHT, "");
    n->addTransition(p, RIGHT_ENDMARKER, accept2, RIGHT, "");
    n->addTransition(p, 'a', p, RIGHT, "c");
    n->addTransition(p, 'b', p, RIGHT, "d");

    printf("Starting composition\n");
    test(n,m,1000);
    free(m);
    free(n);
}

void test2(){ //if word ends with b, replace all a with c, otherwise do nothing
    Two_DFT *m = new Two_DFT("ab", "abc");
    deque<STATE> states = m->addStates(6);
    STATE accept = m->addState();
    m->makeAccept(accept);
    m->addTransition(accept, 'a', accept, RIGHT, "");
    m->addTransition(accept, 'b', accept, RIGHT, "");
    m->addTransition(accept, LEFT_ENDMARKER, accept, RIGHT, "");
    m->addTransition(accept, RIGHT_ENDMARKER, accept, RIGHT, "");
    m->makeStart(states[0]);

    m->addTransition(states[0], LEFT_ENDMARKER, states[0], RIGHT, "");
    m->addTransition(states[0], RIGHT_ENDMARKER, states[1], LEFT, "");
    m->addTransition(states[0], 'a', states[0], RIGHT, "");
    m->addTransition(states[0], 'b', states[0], RIGHT, "");

    m->addTransition(states[1], LEFT_ENDMARKER, states[3], STAY, "");
    m->addTransition(states[1], RIGHT_ENDMARKER, states[1], LEFT, "");
    m->addTransition(states[1], 'a', states[3], LEFT, "");
    m->addTransition(states[1], 'b', states[2], LEFT, "");

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

    Two_DFT *n = new Two_DFT("abc", "def");
    STATE p = n->addState();
    STATE accept2 = n->addState();
    n->makeAccept(accept2);
    n->addTransition(accept2, 'a', accept2, RIGHT, "");
    n->addTransition(accept2, 'b', accept2, RIGHT, "");
    n->addTransition(accept2, 'c', accept2, RIGHT, "");
    n->addTransition(accept2, LEFT_ENDMARKER, accept2, RIGHT, "");
    n->addTransition(accept2, RIGHT_ENDMARKER, accept2, RIGHT, "");
    n->makeStart(p);

    n->addTransition(p, LEFT_ENDMARKER, p, RIGHT, "");
    n->addTransition(p, RIGHT_ENDMARKER, accept2, RIGHT, "");
    n->addTransition(p, 'a', p, RIGHT, "d");
    n->addTransition(p, 'b', p, RIGHT, "e");
    n->addTransition(p, 'c', p, RIGHT, "f");
    printf("Start m %d, start n%d\n", states[0], p);
    printf("Accept m %d, accept n%d\n", accept, accept2);

    printf("Starting composition\n");
    test(n,m,1000);
    free(m);
    free(n);
}

void test3(){ //swap every third letter then replace every other letter with cd
    Two_DFT *m = new Two_DFT("ab", "ab");
    deque<STATE> q = m->addStates(3);
    STATE accept = m->addState();
    m->makeAccept(accept);
    m->addTransition(accept, 'a', accept, RIGHT, "");
    m->addTransition(accept, 'b', accept, RIGHT, "");
    m->addTransition(accept, LEFT_ENDMARKER, accept, RIGHT, "");
    m->addTransition(accept, RIGHT_ENDMARKER, accept, RIGHT, "");
    m->makeStart(q[0]);

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

    Two_DFT *n = new Two_DFT("ab", "ab");
    deque<STATE> p = n->addStates(2);
    STATE accept2 = n->addState();
    n->makeAccept(accept2);
    n->addTransition(accept2, 'a', accept2, RIGHT, "");
    n->addTransition(accept2, 'b', accept2, RIGHT, "");
    n->addTransition(accept2, LEFT_ENDMARKER, accept2, RIGHT, "");
    n->addTransition(accept2, RIGHT_ENDMARKER, accept2, RIGHT, "");
    n->makeStart(p[0]);

    n->addTransition(p[0], LEFT_ENDMARKER, p[0], RIGHT, "");
    n->addTransition(p[0], RIGHT_ENDMARKER, p[1], STAY, "");
    n->addTransition(p[0], 'a', q[0], RIGHT, "");
    n->addTransition(p[0], 'b', q[0], RIGHT, "");

    n->addTransition(p[1], LEFT_ENDMARKER, accept2, STAY, "");
    n->addTransition(p[1], RIGHT_ENDMARKER, p[1], LEFT, "");
    n->addTransition(p[1], 'a', p[1], LEFT, "a");
    n->addTransition(p[1], 'b', p[1], LEFT, "b");

    printf("Starting composition\n");
    test(m,n,1000);
    free(m);
    free(n);
}

void test4(){
    Two_DFT *m = new Two_DFT("ab", "abc");
    deque<STATE> states = m->addStates(6);
    STATE accept = m->addState();
    m->makeAccept(accept);
    m->addTransition(accept, 'a', accept, RIGHT, "");
    m->addTransition(accept, 'b', accept, RIGHT, "");
    m->addTransition(accept, LEFT_ENDMARKER, accept, RIGHT, "");
    m->addTransition(accept, RIGHT_ENDMARKER, accept, RIGHT, "");
    m->makeStart(states[0]);

    m->addTransition(states[0], LEFT_ENDMARKER, states[0], RIGHT, "");
    m->addTransition(states[0], RIGHT_ENDMARKER, states[1], LEFT, "");
    m->addTransition(states[0], 'a', states[0], RIGHT, "");
    m->addTransition(states[0], 'b', states[0], RIGHT, "");

    m->addTransition(states[1], LEFT_ENDMARKER, states[3], STAY, "");
    m->addTransition(states[1], RIGHT_ENDMARKER, states[1], LEFT, "");
    m->addTransition(states[1], 'a', states[3], LEFT, "");
    m->addTransition(states[1], 'b', states[2], LEFT, "");

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

    Two_DFT *n = new Two_DFT("ab", "ab");
    deque<STATE> q = n->addStates(3);
    STATE accept2 = n->addState();
    n->makeAccept(accept2);
    n->addTransition(accept2, 'a', accept2, RIGHT, "");
    n->addTransition(accept2, 'b', accept2, RIGHT, "");
    n->addTransition(accept2, LEFT_ENDMARKER, accept2, RIGHT, "");
    n->addTransition(accept2, RIGHT_ENDMARKER, accept2, RIGHT, "");
    n->makeStart(q[0]);

    n->addTransition(q[0], LEFT_ENDMARKER, q[0], RIGHT, "");
    n->addTransition(q[0], RIGHT_ENDMARKER, accept2, RIGHT, "");
    n->addTransition(q[0], 'a', q[1], RIGHT, "a");
    n->addTransition(q[0], 'b', q[1], RIGHT, "b");

    n->addTransition(q[1], LEFT_ENDMARKER, q[1], RIGHT, "");
    n->addTransition(q[1], RIGHT_ENDMARKER, accept2, RIGHT, "");
    n->addTransition(q[1], 'a', q[2], RIGHT, "a");
    n->addTransition(q[1], 'b', q[2], RIGHT, "b");

    n->addTransition(q[2], LEFT_ENDMARKER, q[0], RIGHT, "");
    n->addTransition(q[2], RIGHT_ENDMARKER, accept2, RIGHT, "");
    n->addTransition(q[2], 'a', q[0], RIGHT, "b");
    n->addTransition(q[2], 'b', q[0], RIGHT, "a");
    printf("Starting composition\n");
    test(m,n,1000);
    free(m);
    free(n);
}

void test5(){
    Two_DFT *m = new Two_DFT("ab", "abc");
    deque<STATE> states = m->addStates(6);
    STATE accept = m->addState();
    m->makeAccept(accept);
    m->addTransition(accept, 'a', accept, RIGHT, "");
    m->addTransition(accept, 'b', accept, RIGHT, "");
    m->addTransition(accept, LEFT_ENDMARKER, accept, RIGHT, "");
    m->addTransition(accept, RIGHT_ENDMARKER, accept, RIGHT, "");
    m->makeStart(states[0]);

    m->addTransition(states[0], LEFT_ENDMARKER, states[0], RIGHT, "");
    m->addTransition(states[0], RIGHT_ENDMARKER, states[1], STAY, "");
    m->addTransition(states[0], 'a', states[0], RIGHT, "");
    m->addTransition(states[0], 'b', states[0], RIGHT, "");

    m->addTransition(states[1], LEFT_ENDMARKER, states[3], STAY, "");
    m->addTransition(states[1], RIGHT_ENDMARKER, states[1], LEFT, "");
    m->addTransition(states[1], 'a', states[3], LEFT, "");
    m->addTransition(states[1], 'b', states[2], LEFT, "");

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

    Two_DFT *n = new Two_DFT("ab", "ab");
    deque<STATE> q = n->addStates(2);
    STATE accept2 = n->addState();
    n->makeAccept(accept2);
    n->addTransition(accept2, 'a', accept2, RIGHT, "");
    n->addTransition(accept2, 'b', accept2, RIGHT, "");
    // n->addTransition(accept2, 'c', accept2, RIGHT, "");
    n->addTransition(accept2, LEFT_ENDMARKER, accept2, RIGHT, "");
    n->addTransition(accept2, RIGHT_ENDMARKER, accept2, RIGHT, "");
    n->makeStart(q[0]);

    n->addTransition(q[0], LEFT_ENDMARKER, q[0], RIGHT, "");
    n->addTransition(q[0], RIGHT_ENDMARKER, q[1], STAY, "");
    n->addTransition(q[0], 'a', q[0], RIGHT, "");
    n->addTransition(q[0], 'b', q[0], RIGHT, "");
    // n->addTransition(q[0], 'c', q[0], RIGHT, "");

    n->addTransition(q[1], LEFT_ENDMARKER, accept2, STAY, "");
    n->addTransition(q[1], RIGHT_ENDMARKER, q[1], LEFT, "");
    n->addTransition(q[1], 'a', q[1], LEFT, "a");
    n->addTransition(q[1], 'b', q[1], LEFT, "b");
    // n->addTransition(q[1], 'c', q[1], LEFT, "c");

    printf("Starting composition\n");
    test(m,n,1000);
    free(m);
    free(n);
}

int main(){
    // test1();
    // test3();
    // test2();
    // test4();
    test5();
}