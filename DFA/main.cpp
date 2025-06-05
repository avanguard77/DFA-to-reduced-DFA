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

    list<Mane> *exit_Manes_To_Other_states;
};

struct Mane {
    string name;
    State *enteredstate;
    State *exitstate;
};

list<State> stateList;
list<string> maneList;

bool State_Deleter(State *state, list<State *> &visited);

void DFA_Delete_Trap_UnuseableState();

void DFA_SHow_Stucture();

void DFA_SHow_Stucture(State *state, list<State *> visited = list<State *>());

void Connect_State_Mane();

void Choosing_Mane_And_NextState(State &state);

void InputGetter_State_Mane();

void DFA_TO_ReducedDFA();

int main() {
    InputGetter_State_Mane();
    Connect_State_Mane();

    DFA_Delete_Trap_UnuseableState();
    State *startState;
    for (State state: stateList) {
        if (state.startState) {
            startState = &state;
            DFA_SHow_Stucture(startState);
        }
    }

    DFA_TO_ReducedDFA();

    return 0;
}

bool State_Deleter(State *state, list<State *> &visited) {
    if (find(visited.begin(), visited.end(), state) != visited.end()) {
        return false;
    }
    visited.push_back(state);

    if (state->finalState) {
        return true;
    }

    if (state->exit_Manes_To_Other_states->empty()) {
        return false;
    }

    for (const Mane &mane: *(state->exit_Manes_To_Other_states)) {
        if (mane.exitstate != state) {
            list<State *> newVisited = visited;
            if (State_Deleter(mane.exitstate, newVisited)) {
                return true;
            }
        }
    }

    return false;
}

void DFA_Delete_Trap_UnuseableState() {
    State *startState = nullptr;
    for (State &state: stateList) {
        if (state.startState) {
            startState = &state;
            break;
        }
    }

    if (!startState) {
        cout << "Error: No start state found!" << endl;
        return;
    }

    if (startState->exit_Manes_To_Other_states->empty()) {
        cout << endl << "DFA is incorrect: Start state has no transitions" << endl << endl;
        return;
    }

    list<State *> trapStates;
    cout << "\n=== Analyzing States for Trap Detection ===\n";

    list<State *> allStates;
    for (State &state: stateList) {
        allStates.push_back(&state);
    }

    for (State *state: allStates) {
        cout << "\nChecking state: " << state->statename;
        if (state->startState) cout << " (Start State)";
        if (state->finalState) cout << " (Final State)";
        cout << endl;

        list<State *> visited;
        if (!State_Deleter(state, visited)) {
            trapStates.push_back(state);
            cout << "✗ State " << state->statename << " identified as trap state" << endl;
            cout << "  Transitions from this state:" << endl;
            if (state->exit_Manes_To_Other_states->empty()) {
                cout << "  - No transitions (Dead end)" << endl;
            } else {
                for (const Mane &mane: *(state->exit_Manes_To_Other_states)) {
                    cout << "  - " << state->statename << " --[" << mane.name << "]--> "
                            << mane.exitstate->statename << endl;
                }
            }
        } else {
            cout << "✓ State " << state->statename << " is valid" << endl;
        }
    }

    cout << "\n=== Removing Transitions to Trap States ===\n";

    for (State *state: allStates) {
        cout << "\nProcessing state: " << state->statename << endl;

        list<Mane> validTransitions;

        for (const Mane &mane: *(state->exit_Manes_To_Other_states)) {
            if (find(trapStates.begin(), trapStates.end(), mane.exitstate) == trapStates.end()) {
                cout << "✓ Keeping transition: " << state->statename << " --[" << mane.name << "]--> "
                        << mane.exitstate->statename << endl;
                validTransitions.push_back(mane);
            } else {
                cout << "- Removing transition: " << state->statename << " --[" << mane.name << "]--> "
                        << mane.exitstate->statename << endl;
            }
        }

        *(state->exit_Manes_To_Other_states) = validTransitions;
    }

    cout << "\n=== Summary of Trap State Removal ===\n";
    if (!trapStates.empty()) {
        cout << "Found " << trapStates.size() << " trap state(s):" << endl;
        for (State *trapState: trapStates) {
            cout << "- " << trapState->statename;
            if (trapState->finalState) cout << " (Was Final State)";
            if (trapState->startState) cout << " (Was Start State)";
            cout << endl;
        }

        // Remove trap states from stateList
        stateList.remove_if([&trapStates](const State& state) {
            return std::find_if(trapStates.begin(), trapStates.end(),
                [&state](const State* trap) { return trap->statename == state.statename; }
            ) != trapStates.end();
        });
        
        cout << "\nTrap states have been removed from the DFA." << endl;
    } else {
        cout << "No trap states found in the DFA." << endl;
    }
    cout << "\n=== Trap State Removal Complete ===\n" << endl;
}

void DFA_SHow_Stucture() {
    cout << endl << "DFA Structure" << endl;

    for (const State &state: stateList) {
        // Print state information
        cout << endl << "State: " << state.statename;
        if (state.finalState) cout << " (Final State)";
        else cout << " (Not Final State)";
        cout << endl;

        // Print transitions
        cout << "Mane :" << endl;
        if (state.exit_Manes_To_Other_states != nullptr) {
            for (const Mane &mane: *state.exit_Manes_To_Other_states) {
                cout << "  " << state.statename << "[" << mane.name << "]> "
                        << mane.exitstate->statename << endl;
            }

            if (state.exit_Manes_To_Other_states->empty()) {
                cout << "  (No Mane)" << endl;
            }
        } else {
            cout << "  (No Mane List)" << endl;
        }
        cout << endl << endl;
    }
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
            state.exit_Manes_To_Other_states = new list<Mane>();

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
                    state.exit_Manes_To_Other_states = new list<Mane>();

                    stateList.push_back(state);
                    cout << "final state is added" << endl;
                    break;
                }
                if (finialStateChecked == 0) {
                    //nonfinal state

                    State state;
                    state.statename = stateinput;
                    state.exit_Manes_To_Other_states = new list<Mane>();

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

    for (State &state: stateList) {
        if (state.startState) {
            cout << "startState is : " << state.statename << endl << endl;
            Choosing_Mane_And_NextState(state);
        }
    }

    for (State &state: stateList) {
        if (!state.startState) {
            cout << "this state is : " << state.statename << endl << endl;
            Choosing_Mane_And_NextState(state);
        }
    }
}

void Choosing_Mane_And_NextState(State &state) {
    list<string> maneList_for_thisState = maneList;

    bool isStartStateCheckedforFirstTime = state.startState;
    while (true) {
        if (maneList_for_thisState.empty()) {
            return;
        }

        string maneinput;
        cout << "Which Mane do you choose?(for stoping and go for next state (//)\\in start state u should choose one )"
                << endl << endl;

        for (const string &mane: maneList_for_thisState) {
            cout << mane << endl;
        }
        cout << endl;

        cin >> maneinput;
        if (maneinput == "//" && !isStartStateCheckedforFirstTime) {
            return;
        }

        auto maneIt = find(maneList_for_thisState.begin(), maneList_for_thisState.end(), maneinput);
        if (maneIt == maneList_for_thisState.end()) {
            cout << "Invalid Mane selection!" << endl;
            continue;
        }

        Mane this_mane;
        this_mane.name = maneinput;

        cout << "Mane is selected: " << maneinput << endl;
        cout << "Which State should we select by this Mane " << maneinput << " ?" << endl;

        for (const State &this_state: stateList) {
            cout << this_state.statename << endl;
        }

        string stateinput;
        cin >> stateinput;

        auto stateIt = ranges::find_if(stateList, [&stateinput](const State &this_state) {
            return this_state.statename == stateinput;
        });

        if (stateIt == stateList.end()) {
            cout << "Invalid State selection!" << endl;
            continue;
        }

        this_mane.enteredstate = &state;
        this_mane.exitstate = &(*stateIt);
        state.exit_Manes_To_Other_states->push_back(this_mane);
        maneList_for_thisState.erase(maneIt);

        if (state.startState) {
            isStartStateCheckedforFirstTime = false;
        }
    }
}

void DFA_SHow_Stucture(State *state, list<State *> visited) {
    if (!state) {
        cout << "\nError: Invalid state pointer";
        return;
    }

    if (find(visited.begin(), visited.end(), state) != visited.end()) {
        cout << " -> " << state->statename << " cycle detected  " << endl;
        return;
    }
    visited.push_back(state);

    cout << "\n" << string(visited.size() * 2, ' ') << "State: " << state->statename << endl;
    if (state->startState) cout << " (Start)";
    if (state->finalState) cout << " (Final)";

    if (!state->exit_Manes_To_Other_states) {
        cout << "\n" << string(visited.size() * 2, ' ') << "Error: No transition list" << endl;
        return;
    }

    if (state->exit_Manes_To_Other_states->empty()) {
        cout << "\n" << string(visited.size() * 2, ' ') << "No outgoing transitions" << endl;
        return;
    }

    for (const Mane &mane: *(state->exit_Manes_To_Other_states)) {
        cout << "\n" << string(visited.size() * 2, ' ')
                << "  " << mane.name << "--> " << mane.exitstate->statename;
        DFA_SHow_Stucture(mane.exitstate, visited);
    }
}


void DFA_TO_ReducedDFA() {
    list<pair<string, string>> allTople;
    list<pair<string, string>> deletedTople;
    
    // First create all possible pairs
    for (auto it1 = stateList.begin(); it1 != stateList.end(); ++it1) {
        auto it2 = it1;
        ++it2;
        while (it2 != stateList.end()) {
            allTople.push_back({it1->statename, it2->statename});
            ++it2;
        }
    }

    // Show initial pairs
    cout << "\nInitial all pairs: ";
    for (const auto &pair: allTople) {
        cout << pair.first << pair.second << " ";
    }
    cout << endl;

    // Now filter based on final states
    list<pair<string, string>> newAllTople;
    for (const auto &pair: allTople) {
        // Find the actual states to check their final status
        auto state1 = find_if(stateList.begin(), stateList.end(),
            [&pair](const State &s) { return s.statename == pair.first; });
        auto state2 = find_if(stateList.begin(), stateList.end(),
            [&pair](const State &s) { return s.statename == pair.second; });
        
        if (state1->finalState != state2->finalState) {
            deletedTople.push_back(pair);
        } else {
            newAllTople.push_back(pair);
        }
    }
    allTople = newAllTople;

    // Show results after filtering
    cout << "\nAfter checking final states:" << endl;
    cout << "Remaining pairs: ";
    for (const auto &pair: allTople) {
        cout << pair.first << pair.second << " ";
    }
    cout << endl;
    
    cout << "Deleted pairs: ";
    for (const auto &pair: deletedTople) {
        cout << pair.first << pair.second << " ";
    }
    cout << endl;
}

void cleanup() {
    for (State &state: stateList) {
        delete state.exit_Manes_To_Other_states;
    }
}
