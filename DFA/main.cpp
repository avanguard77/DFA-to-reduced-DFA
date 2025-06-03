#include <iostream>
#include <list>
#include <algorithm>
#include <ranges>
struct Mane;
using namespace std;

struct State {
    string statename;

    bool startState = false;
    bool finalState = false;

    list<Mane> exit_Manes_To_Other_states;
};

struct Mane {
    string name;
    State enteredstate;
    State exitstate;
};

list<State> stateList;
list<string> maneList;


void Connect_State_Mane();

void R(State);

void InputGetter_State_Mane();


int main() {
    InputGetter_State_Mane();
    Connect_State_Mane();


    return 0;
}

void InputGetter_State_Mane() {
    cout << "Hello DFA to redused DFA Project ( Hamed Mohammadi )" << endl << endl <<
            "Please Enter the number of your State :";


    //state input

    int numberOfState;
    cin >> numberOfState;

    for (int i = 0; i < numberOfState; i++) {
        string stateinput;

        if (i == 0) {
            //start state

            cout << "Enter startState name :" << endl;
            cin >> stateinput;
            State state;
            state.statename = stateinput;
            state.startState = true;

            stateList.push_back(state);
        } else {
            // the other state check is that final or not

            cout << "Enter Other state name :" << endl;
            cin >> stateinput;

            State state;
            cout << "Is this state finialState ? ((yes/1)(no/0))   ";
            int finialStateChecked;
            while (true) {
                cin >> finialStateChecked;
                if (finialStateChecked == 1) {
                    //final state

                    State state;
                    state.statename = stateinput;
                    state.finalState = true;

                    stateList.push_back(state);
                    cout << "non final state is added" << endl;
                    break;
                }
                if (finialStateChecked == 0) {
                    //nonfinal state

                    State state;
                    state.statename = stateinput;

                    stateList.push_back(state);
                    cout << "non final state is added" << endl;
                    break;
                }
                cout << "invalid number please enter ((yes/1)(no/0))" << endl;
            }
        }
    }

    cout << endl << endl << "Please Enter the number of your Mane :";

    int numberOfMane;
    cin >> numberOfMane;
    cout << endl << endl;

    for (int i = 0; i < numberOfMane; i++) {
        cout << endl << "Enter " << (i + 1) << " Mane name :";
        string maneinput;
        cin >> maneinput;
        maneList.push_back(maneinput);
        cout << endl << "Mane are added " << endl << endl;
    }
}

void Connect_State_Mane() {
    cout << endl << endl;
    State this_state;
    for (State state: stateList) {
        if (state.startState) {
            this_state = state;
            cout << "startState is : " << this_state.statename << endl << endl;
            R(this_state);
        }
    }

    for (State thisState: stateList) {
        if (!this_state.startState) {
        }
    }
}

void R(State state) {
    list<string> maneList_for_thisState = maneList;
    list<State> state_for_checking = stateList;

    while (true) {
        //empty checked
        if (maneList_for_thisState.empty()) {
            return;
        }

        string maneinput;
        cout << "Which Mane do you choose?" << endl;

        for (string mane : maneList_for_thisState) {
            cout << mane << endl;
        }

        cin >> maneinput;
        //checking invalid
        auto maneIt = find(maneList_for_thisState.begin(), maneList_for_thisState.end(), maneinput);
        if (maneIt == maneList_for_thisState.end()) {
            cout << "Invalid Mane selection!" << endl;
            continue;
        }
        //setup Mane
        Mane this_mane;
        this_mane.name = maneinput;
        this_mane.enteredstate = state;

        cout << "Mane is selected: " << maneinput << endl;
        cout << "Which State should we select by this Mane " << maneinput << " ?" << endl;

        for (State this_state : state_for_checking) {
            if (this_state.statename != state.statename) {
                cout << this_state.statename << endl;
            }
        }

        string stateinput;
        cin >> stateinput;

        auto stateIt = ranges::find_if(state_for_checking, [&stateinput](const State& this_state) {
            return this_state.statename == stateinput;
        });

        if (stateIt == state_for_checking.end()) {
            cout << "Invalid State selection!" << endl;
            continue;  // Prevents proceeding with invalid state selection
        }


        this_mane.exitstate = *stateIt;
        state.exit_Manes_To_Other_states.push_back(this_mane);
        maneList_for_thisState.erase(maneIt);
    }
}
