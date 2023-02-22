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
        transitions[start].insert_or_assign(ch, output);
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


typedef tuple<STATE, //q_a
              STATE, //q_b
              tuple<set<tuple<STATE, STATE, DIRECTION, string>>, char> *,
              string, //w
              int,    //i
              int,    //j
              int     //k
            > new_state_type;

tuple<set<tuple<STATE, STATE, DIRECTION, string>>, char> *restrict_alphas(tuple<set<tuple<STATE, STATE, DIRECTION, string>>, char> *alphas, STATE initial_state, int initial_alpha){ 
    set<tuple<STATE, STATE, DIRECTION, string>> restricted_alpha0;
    set<tuple<STATE, STATE, DIRECTION, string>> restricted_alpha1;
    set<tuple<STATE, STATE, DIRECTION, string>> restricted_alpha2;
    set<tuple<STATE, STATE, DIRECTION, string>> restricted_alphas[3];
    restricted_alphas[0] = restricted_alpha0;
    restricted_alphas[1] = restricted_alpha1;
    restricted_alphas[2] = restricted_alpha2;

    tuple<STATE, STATE, DIRECTION, string> current_transition;
    int current_alpha;
    for (auto t : get<0>(alphas[initial_alpha])){
        if (get<0>(t) == initial_state){
            current_transition = t;
            restricted_alphas[initial_alpha].insert(t);
            current_alpha = initial_alpha + get<2>(t);
            break;
        }
    }

    while (current_alpha >= 0 && current_alpha <= 2){
        for (auto t : get<0>(alphas[current_alpha])){
            if (get<0>(t) == get<1>(current_transition)){
                restricted_alphas[current_alpha].insert(t);
                current_transition = t;
                current_alpha += get<2>(t);
                break;
            }
        }
    }

    tuple<set<tuple<STATE, STATE, DIRECTION, string>>, char> *output = (tuple<set<tuple<STATE, STATE, DIRECTION, string>>, char> *) malloc(3 * sizeof(make_tuple(restricted_alphas[0], get<1>(alphas[0]))));
    output[0] = make_tuple(restricted_alphas[0], get<1>(alphas[0]));
    output[1] = make_tuple(restricted_alphas[1], get<1>(alphas[1]));
    output[2] = make_tuple(restricted_alphas[2], get<1>(alphas[2]));
    return output;
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
    STATE accept_state = C->addState();
    C->makeAccept(accept_state);
    
    //generate the alphabet that represents the configuration graph
    map<char, tuple<set<tuple<STATE, STATE, DIRECTION, string>>, char>> graph_alphabet;
    for (auto ch : input_alphabet){
        set<tuple<STATE, STATE, DIRECTION, string>> alpha;
        for (STATE s = 0; s < num_states_A; s++){
            tuple<STATE, DIRECTION, string> transition = transitions_A[s][ch];
            tuple<STATE, STATE, DIRECTION, string> t = make_tuple(s, get<0>(transition), get<1>(transition), get<2>(transition));
            alpha.insert(t);
        }
        graph_alphabet.insert({ch, make_tuple(alpha, ch)});
    }
    if (v) printf("graph alphabet generated\n");

    map<new_state_type,STATE> new_states;
    map<new_state_type, char> letter_at_state;
    deque<new_state_type> new_state_queue;

    //set up the first few transitions to create the restricted configuration graph
    tuple<set<tuple<STATE, STATE, DIRECTION, string>>, char> empty_alpha;
    tuple<set<tuple<STATE, STATE, DIRECTION, string>>, char> initial_alphas0[3] = {empty_alpha, empty_alpha, empty_alpha};
    new_state_type initial_state0 = make_tuple(initial_state_A, initial_state_B, initial_alphas0, "", 0, 0, 0);
    new_states.insert({initial_state0, C->addState()});
    C->makeStart(new_states[initial_state0]);
    if (v) printf("start state created\n");

    tuple<set<tuple<STATE, STATE, DIRECTION, string>>, char> initial_alphas1[3] = {graph_alphabet[LEFT_ENDMARKER], empty_alpha, empty_alpha};
    new_state_type initial_state1 = make_tuple(initial_state_A, initial_state_B, initial_alphas1, "", 0, 0, 0);
    new_states.insert({initial_state1, C->addState()});
    C->addTransition(new_states[initial_state0], LEFT_ENDMARKER, new_states[initial_state1], RIGHT, "", v);

    for (auto ch : input_alphabet){
        if (ch == LEFT_ENDMARKER) continue;
        tuple<set<tuple<STATE, STATE, DIRECTION, string>>, char> initial_alphas2[3] = {graph_alphabet[LEFT_ENDMARKER], graph_alphabet[ch], empty_alpha};
        new_state_type initial_state2 = make_tuple(initial_state_A, initial_state_B, initial_alphas2, "", 0, 0, 0);
        if (new_states.find(initial_state2) == new_states.end()){
            new_states.insert({initial_state2, C->addState()});
        }
        
        // new_states.insert({initial_state2, C->addState()});
        C->addTransition(new_states[initial_state1], ch, new_states[initial_state2], RIGHT, "", v);
        for (auto ch2 : input_alphabet){
            if (ch2 == LEFT_ENDMARKER || ch == RIGHT_ENDMARKER) continue;
            tuple<set<tuple<STATE, STATE, DIRECTION, string>>, char> initial_alphas3[3] = {graph_alphabet[LEFT_ENDMARKER], graph_alphabet[ch], graph_alphabet[ch2]};
            tuple<set<tuple<STATE, STATE, DIRECTION, string>>, char> *restricted_alphas = restrict_alphas(initial_alphas3, initial_state_A, 0);
            new_state_type initial_state3 = make_tuple(initial_state_A, initial_state_B, restricted_alphas, "", 0, 0, 1);
            if (new_states.find(initial_state3) == new_states.end()){
                new_states.insert({initial_state3, C->addState()});
                new_state_queue.push_back(initial_state3);
            }
            // new_states.insert({initial_state3, C->addState()});
            // new_state_queue.push_back(initial_state3);
            letter_at_state.insert({initial_state3, ch});
            C->addTransition(new_states[initial_state2], ch2, new_states[initial_state3], LEFT, "", v);
        }
    }
    if (v) printf("initial states created\n");

    //main bulk of transition function
    while (!new_state_queue.empty()){
        printf("%d\n", new_state_queue.size());
        //get all the info about the current state
        new_state_type current_state = new_state_queue.front();
        new_state_queue.pop_front();
        STATE q_A = get<0>(current_state);
        STATE q_B = get<1>(current_state);
        tuple<set<tuple<STATE, STATE, DIRECTION, string>>, char>* alphas = get<2>(current_state);
        string w = get<3>(current_state);
        int position_in_w = get<4>(current_state);
        int current_alpha = get<5>(current_state);
        int current_sim = get<6>(current_state);

        //\delta_C([q_A,q_B,alphas,w,|w|,j,1],\sigma) =
        if (current_sim == 1){
            if (position_in_w == w.length()){
                tuple<set<tuple<STATE, STATE, DIRECTION, string>>, char> alpha = alphas[current_alpha];
                tuple<STATE, STATE, DIRECTION, string> transition;
                for (auto t : get<0>(alpha)){
                    if (get<0>(t) == q_A){
                        transition = t;
                        break;
                    }
                }
                STATE q_Ap = get<1>(transition);
                DIRECTION d = get<2>(transition);
                string u = get<3>(transition);
                if (d == STAY){
                    new_state_type new_state = make_tuple(q_Ap, q_B, alphas, u, 0, current_alpha, 1);
                    if (new_states.find(new_state) == new_states.end()){
                        new_states.insert({new_state, C->addState()});
                        new_state_queue.push_back(new_state);
                    }
                    C->addTransition(new_states[current_state], get<1>(alphas[1]), new_states[new_state], STAY, "", v);
                }
                else if (d == RIGHT){
                    if (current_alpha == 2){
                        if (get<1>(alphas[2]) == RIGHT_ENDMARKER){
                            C->addTransition(new_states[current_state], get<1>(alphas[1]), accept_state, RIGHT, "", v);
                            continue;
                        }
                        new_state_type new_state = make_tuple(q_Ap, q_B, alphas, u, 0, 2, 2);
                        if (new_states.find(new_state) == new_states.end()){
                            new_states.insert({new_state, C->addState()});
                            new_state_queue.push_back(new_state);
                        }
                        C->addTransition(new_states[current_state], get<1>(alphas[1]), new_states[new_state], RIGHT, "", v);
                    }
                    else {
                        new_state_type new_state = make_tuple(q_Ap, q_B, alphas, u, 0, current_alpha + 1, 1);
                        if (new_states.find(new_state) == new_states.end()){
                            new_states.insert({new_state, C->addState()});
                            new_state_queue.push_back(new_state);
                        }
                        C->addTransition(new_states[current_state], get<1>(alphas[1]), new_states[new_state], STAY, "", v);
                    }
                }
                else if (d == LEFT){
                    if (current_alpha == 0){
                        if (get<1>(alphas[0]) == LEFT_ENDMARKER){
                            C->addTransition(new_states[current_state], get<1>(alphas[1]), accept_state, RIGHT, "", v);
                            continue;
                        }
                        new_state_type new_state = make_tuple(q_Ap, q_B, alphas, u, 0, 0, 4);
                        if (new_states.find(new_state) == new_states.end()){
                            new_states.insert({new_state, C->addState()});
                            new_state_queue.push_back(new_state);
                        }
                        C->addTransition(new_states[current_state], get<1>(alphas[1]), new_states[new_state], LEFT, "", v);
                    }
                    else {
                        new_state_type new_state = make_tuple(q_Ap, q_B, alphas, u, 0, current_alpha - 1, 1);
                        if (new_states.find(new_state) == new_states.end()){
                            new_states.insert({new_state, C->addState()});
                            new_state_queue.push_back(new_state);
                        }
                        C->addTransition(new_states[current_state], get<1>(alphas[1]), new_states[new_state], STAY, "", v);
                    }
                }
            }
            else if (position_in_w >= 0){
                tuple<STATE, DIRECTION, string> transition = transitions_B[q_B][w[position_in_w]];
                STATE q_Bp = get<0>(transition);
                DIRECTION d = get<1>(transition);
                string output = get<2>(transition);
                new_state_type new_state = make_tuple(q_A, q_Bp, alphas, w, position_in_w + d, current_alpha, 1);
                if (new_states.find(new_state) == new_states.end()){
                    new_states.insert({new_state, C->addState()});
                    new_state_queue.push_back(new_state);
                }
                C->addTransition(new_states[current_state], get<1>(alphas[1]), new_states[new_state], STAY, output, v);
            }
            
        }
        else if (current_sim == 2){
            new_state_type new_state = make_tuple(q_A, q_B, alphas, w, 0, 2, 3);
            if (new_states.find(new_state) == new_states.end()){
                new_states.insert({new_state, C->addState()});
                new_state_queue.push_back(new_state);
            }
            C->addTransition(new_states[current_state], get<1>(alphas[2]), new_states[new_state], RIGHT, "", v);
        }
        else if (current_sim == 3){
            for (auto ch : input_alphabet){
                if (ch == LEFT_ENDMARKER) continue;
                tuple<set<tuple<STATE, STATE, DIRECTION, string>>, char> alphasP[3];
                alphasP[0] = alphas[1];
                alphasP[1] = alphas[2];
                alphasP[2] = graph_alphabet[ch];
                new_state_type new_state = make_tuple(q_A, q_B, restrict_alphas(alphasP, q_A, 2), w, 0, 2, 1);
                if (new_states.find(new_state) == new_states.end()){
                    new_states.insert({new_state, C->addState()});
                    new_state_queue.push_back(new_state);
                }
                C->addTransition(new_states[current_state], ch, new_states[new_state], LEFT, "", v);  
            }
        }

        else if (current_sim == 4){
            new_state_type new_state = make_tuple(q_A, q_B, alphas, w, 0, 0, 5);
            if (new_states.find(new_state) == new_states.end()){
                new_states.insert({new_state, C->addState()});
                new_state_queue.push_back(new_state);
            }
            C->addTransition(new_states[current_state], get<1>(alphas[0]), new_states[new_state], LEFT, "", v);
        }
        else if (current_sim == 5){
            for (auto ch : input_alphabet){
                if (ch == RIGHT_ENDMARKER) continue;
                tuple<set<tuple<STATE, STATE, DIRECTION, string>>, char> alphasP[3];
                alphasP[2] = alphas[1];
                alphasP[1] = alphas[0];
                alphasP[0] = graph_alphabet[ch];
                new_state_type new_state = make_tuple(q_A, q_B, restrict_alphas(alphasP, q_A, 0), w, 0, 0, 1);
                if (new_states.find(new_state) == new_states.end()){
                    new_states.insert({new_state, C->addState()});
                    new_state_queue.push_back(new_state);
                }
                C->addTransition(new_states[current_state], ch, new_states[new_state], RIGHT, "", v);  
            }
        }
    }

    return C;
}



void test(Two_DFT *B, Two_DFT *A, int numTests){
    srand(time(0));
    set<char> alpha = A->getInputAlpha();
    Two_DFT *C = compose(B, A, true);
    // int states = C->getStates();
    // int numRight = 0;

    // for (int i = 0; i < numTests; i++){
    //     int len = rand() % states*2;
    //     string input = "";
    //     for (int j = 0; j < len; j++){
    //         int k = rand() % alpha.size();
    //         set<char>::iterator it = alpha.begin();
    //         for (;k != 0; k--) it++;
    //         char letter = *it;
    //         input += letter;
    //     }
    //     string output1 = B->run(A->run(input));
    //     string output2 = C->run(input);

    //     if (output1 == output2){
    //         numRight += 1;
    //     }

    //     printf("Input w = %s\nB(A(w)) = %s\nC(w) = %s\n\n", input.c_str(), output1.c_str(), output2.c_str());
    // }

    // printf("# Correct Outputs: %d / %d\n", numRight, numTests);
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
    test(n,m,100);
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
    // test2();

    set<tuple<deque<int>,char>> a;
    set<tuple<deque<int>,char>> b;
    set<tuple<deque<int>,char>> q;
    deque<int> c;
    deque<int> d;
    deque<int> e;

    c.push_back(1);
    c.push_back(2);
    c.push_back(3);
    d.push_back(1);
    d.push_back(2);
    d.push_back(3);
    e.push_back(1);
    e.push_back(2);
    e.push_back(3);

    a.insert({make_tuple(c, 'i')});
    b.insert({make_tuple(d, 'i')});
    q.insert({make_tuple(e, 'j')});

    printf("%s\n", bool_to_string(a.find(make_tuple(c,'i')) == a.end()).c_str());
    printf("%s\n", bool_to_string(a.find(make_tuple(d,'i')) == a.end()).c_str());
    printf("%s\n", bool_to_string(a.find(make_tuple(e,'j')) == a.end()).c_str());

}