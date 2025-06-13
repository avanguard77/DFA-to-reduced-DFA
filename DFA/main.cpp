#include <iostream>
#include <list>
#include <algorithm>
#include <ranges>
#include <vector>
#include <map>
#include <set>
#include <numeric>
#include <functional> // For std::function, if needed, though not directly by DSU usually

struct Mane;
using namespace std;

// DSU Data Structures
map<string, string> parent;

// DSU Find operation
string find_set(string s) {
    if (parent[s] == s)
        return s;
    return parent[s] = find_set(parent[s]);
}

// DSU Unite operation
void unite_sets(string a, string b) {
    a = find_set(a);
    b = find_set(b);
    if (a != b)
        parent[b] = a;
}

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

// Forward declaration for helper function if its definition is after Create_Minimized_DFA
State* find_state_in_list_helper(list<State>& states, const string& name);

void Create_Minimized_DFA(const list<pair<string, string>>& equivalent_pairs) {
    // 1. DSU for Grouping
    parent.clear(); // Clear any previous DSU states
    for (const auto& state_obj : stateList) {
        parent[state_obj.statename] = state_obj.statename;
    }

    for (const auto& eq_pair : equivalent_pairs) {
        unite_sets(eq_pair.first, eq_pair.second);
    }

    // 2. Map Representatives to Members
    map<string, list<State*>> representative_to_class_members;
    list<State*> original_state_pointers; // To keep track of original state pointers

    // Need to iterate through the global stateList by reference to get valid pointers
    for (State& original_state_ref : stateList) {
        original_state_pointers.push_back(&original_state_ref);
    }

    for (State* original_state_ptr : original_state_pointers) {
        string rep = find_set(original_state_ptr->statename);
        representative_to_class_members[rep].push_back(original_state_ptr);
    }

    // 3. Create Temporary New States
    list<State> temp_new_states;
    map<string, string> rep_to_new_name_map;

    for (auto const& [rep_name, members_list] : representative_to_class_members) {
        State new_state;
        string generated_name;
        if (members_list.size() == 1) {
            generated_name = members_list.front()->statename;
        } else {
            list<string> member_names;
            for (State* member_state : members_list) {
                member_names.push_back(member_state->statename);
            }
            member_names.sort(); // Sort names for consistent naming
            generated_name = accumulate(member_names.begin(), member_names.end(), string(""),
                                      [](const string& a, const string& b) {
                                          return a.empty() ? b : a + "_" + b;
                                      });
        }
        new_state.statename = generated_name;
        rep_to_new_name_map[rep_name] = generated_name;

        new_state.startState = false;
        new_state.finalState = false;
        for (State* member_state : members_list) {
            if (member_state->startState) {
                new_state.startState = true;
            }
            if (member_state->finalState) {
                new_state.finalState = true;
            }
        }
        new_state.exit_Manes_To_Other_states = new list<Mane>();
        temp_new_states.push_back(new_state); // State object is copied here
    }

    // 4. Cleanup Old stateList
    for (State& old_state : stateList) { // Iterate by reference to modify
        if (old_state.exit_Manes_To_Other_states != nullptr) {
            delete old_state.exit_Manes_To_Other_states;
            old_state.exit_Manes_To_Other_states = nullptr;
        }
    }
    stateList.clear();

    // 5. Set New Global stateList
    stateList = temp_new_states; // temp_new_states are copied into stateList

    // 6. Populate Transitions for New States
    // Helper function find_state_in_new_globallist (can use a lambda or separate function)
    // This lambda searches the current global stateList
    auto find_state_in_current_global_list = [&](const string& name) -> State* {
        for (State& s_ref : stateList) {
            if (s_ref.statename == name) {
                return &s_ref;
            }
        }
        return nullptr;
    };

    // We need to re-iterate through the original structure to build new transitions.
    // The representative_to_class_members map still holds pointers to original states,
    // which is fine for accessing their transition information.

    for (auto const& [rep_name, members_list] : representative_to_class_members) {
        if (members_list.empty()) continue;

        string new_source_name = rep_to_new_name_map[rep_name];
        State* new_source_state = find_state_in_current_global_list(new_source_name);

        if (!new_source_state) {
            cerr << "Error: New source state " << new_source_name << " not found in global list." << endl;
            continue;
        }

        // Pick any original state from the class to read its transitions
        State* original_state_from_class = members_list.front();
        if (!original_state_from_class || !original_state_from_class->exit_Manes_To_Other_states) {
            continue;
        }

        for (const Mane& old_mane : *(original_state_from_class->exit_Manes_To_Other_states)) {
            if (!old_mane.exitstate) continue;

            string original_destination_state_name = old_mane.exitstate->statename;
            string dest_representative_name = find_set(original_destination_state_name); // DSU still has original names
            string target_new_name = rep_to_new_name_map[dest_representative_name];
            State* new_target_state = find_state_in_current_global_list(target_new_name);

            if (!new_target_state) {
                cerr << "Error: New target state " << target_new_name << " not found for mane " << old_mane.name << endl;
                continue;
            }

            // Check if transition already exists
            bool transition_exists = false;
            for (const Mane& existing_mane : *(new_source_state->exit_Manes_To_Other_states)) {
                if (existing_mane.name == old_mane.name && existing_mane.exitstate == new_target_state) {
                    transition_exists = true;
                    break;
                }
            }

            if (!transition_exists) {
                Mane new_mane;
                new_mane.name = old_mane.name;
                new_mane.enteredstate = new_source_state; // Correctly points to the new state in the global list
                new_mane.exitstate = new_target_state;   // Correctly points to the new state in the global list
                new_source_state->exit_Manes_To_Other_states->push_back(new_mane);
            }
        }
    }
}


// Definition of the helper function (if not a lambda inside Create_Minimized_DFA)
// This helper can search any provided list of State objects.
State* find_state_in_list_helper(list<State>& states, const string& name) {
    for (State& s_ref : states) {
        if (s_ref.statename == name) {
            return &s_ref;
        }
    }
    return nullptr;
}


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

    DFA_TO_ReducedDFA(); // This function calculates equivalent_pairs

    // Example: Call Create_Minimized_DFA with the output of DFA_TO_ReducedDFA
    // Assuming DFA_TO_ReducedDFA now returns or stores its result in a way that can be passed here.
    // For now, let's simulate having some equivalent pairs.
    // This part needs to be connected with DFA_TO_ReducedDFA's actual output.
    list<pair<string, string>> equivalent_pairs_from_reduction;
    // Example: equivalent_pairs_from_reduction.push_back({"q1", "q2"});
    // equivalent_pairs_from_reduction.push_back({"q3", "q4"});

    // The actual equivalent_pairs should be populated by DFA_TO_ReducedDFA()
    // For testing purposes, you might need to modify DFA_TO_ReducedDFA to return these pairs
    // or store them in a global/accessible variable.
    // Create_Minimized_DFA(equivalent_pairs_from_reduction);


    // After minimization, show the structure again
    cout << "\n\n=== DFA Structure After Minimization ===\n";
    State *new_start_state = nullptr;
    for (State &state_ref : stateList) { // Iterate by reference
        if (state_ref.startState) {
            new_start_state = &state_ref;
            DFA_SHow_Stucture(new_start_state); // Show structure from the new start state
            break;
        }
    }
    if (!new_start_state && !stateList.empty()) {
         DFA_SHow_Stucture(&stateList.front()); // Or show all components if no start state
    } else if (stateList.empty()) {
        cout << "DFA is empty after minimization." << endl;
    }


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

    // Show results after final state filtering
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

    // New section: Check mane transitions and their destinations
    bool changesMade;
    do {
        changesMade = false;
    auto it = allTople.begin();
    while (it != allTople.end()) {
        // Find the actual states for this pair
        auto state1 = find_if(stateList.begin(), stateList.end(),
            [&it](const State &s) { return s.statename == it->first; });
        auto state2 = find_if(stateList.begin(), stateList.end(),
            [&it](const State &s) { return s.statename == it->second; });

        bool shouldDelete = false;

            // Check if both states have the same mane transitions
        for (const string &mane : maneList) {
            State* nextState1 = nullptr;
            State* nextState2 = nullptr;
            bool hasTransition1 = false;
            bool hasTransition2 = false;

            // Find where state1 goes with this mane
            for (const Mane &transition : *(state1->exit_Manes_To_Other_states)) {
                if (transition.name == mane) {
                    nextState1 = transition.exitstate;
                    hasTransition1 = true;
                    break;
                }
            }

            // Find where state2 goes with this mane
            for (const Mane &transition : *(state2->exit_Manes_To_Other_states)) {
                if (transition.name == mane) {
                    nextState2 = transition.exitstate;
                    hasTransition2 = true;
                    break;
                }
            }

                // If both states have transitions for this mane
            if (hasTransition1 && hasTransition2) {
                    // Create pair from destination states (in correct order)
                    pair<string, string> destPair;
                    if (nextState1->statename < nextState2->statename) {
                        destPair = {nextState1->statename, nextState2->statename};
                    } else {
                        destPair = {nextState2->statename, nextState1->statename};
                    }

                    // Check if this destination pair is in deletedTople
                    if (find(deletedTople.begin(), deletedTople.end(), destPair) != deletedTople.end()) {
                        shouldDelete = true;
                        cout << "\nPair " << it->first << it->second << " leads to deleted pair "
                             << destPair.first << destPair.second << " with mane " << mane << endl;
                        break;
                }
            }
        }

        if (shouldDelete) {
            deletedTople.push_back(*it);
            it = allTople.erase(it);
                changesMade = true;
        } else {
            ++it;
        }
    }
    } while (changesMade);

    // Show results after mane transition checking
    cout << "\nAfter checking mane transitions:" << endl;
    cout << "Remaining pairs (these are the equivalent pairs): ";
    for (const auto &pair: allTople) {
        cout << "(" << pair.first << ", " << pair.second << ") ";
    }
    cout << endl;

    cout << "Deleted pairs (these are distinguishable): ";
    for (const auto &pair: deletedTople) {
        cout << "(" << pair.first << ", " << pair.second << ") ";
    }
    cout << endl;

    // Create_Minimized_DFA should be called with 'allTople' as equivalent_pairs
    Create_Minimized_DFA(allTople);
}

void cleanup() {
    for (State &state: stateList) {
        if (state.exit_Manes_To_Other_states != nullptr) {
            delete state.exit_Manes_To_Other_states;
            state.exit_Manes_To_Other_states = nullptr; // Good practice
        }
    }
    stateList.clear(); // Clear the list itself
}