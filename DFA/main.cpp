#include <iostream>
#include <list>
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

    while (true) {
        //if maneList_for_thisState is empty checked
        if (maneList_for_thisState.empty()) {
            break;
        }
        string maneinput;

        cout << "which state should we connect" << endl;

        for (string mane: maneList_for_thisState) {
            cout << mane << endl;
        }
        // if maneinput is on the list
        cin >> maneinput;
        for (string mane: maneList_for_thisState) {

            if (maneinput == mane) {
                maneList_for_thisState.remove(maneinput);
                break;
            }
            cout << "value Invalid" << endl;
        }
    }
}
