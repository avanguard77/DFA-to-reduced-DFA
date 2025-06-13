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

// The global helper function find_state_in_list_helper might be unused if local lambdas cover all needs.
// If it was defined as:
// State* find_state_in_list_helper(list<State>& states, const string& name) {
//     for (State& s_ref : states) {
//         if (s_ref.statename == name) {
//             return &s_ref;
//         }
//     }
//     return nullptr;
// }
// It can be kept if other functions use it, or removed if not.

void Create_Minimized_DFA(const list<pair<string, string>>& equivalent_pairs) {
    // === Current global stateList holds the original states at the beginning of this function ===

    // 1. DSU for Grouping (Uses original global stateList)
    parent.clear(); // Global DSU parent map, stores original state names
    for (const auto& state_obj : stateList) { // stateList contains original State objects
        parent[state_obj.statename] = state_obj.statename;
    }
    for (const auto& eq_pair : equivalent_pairs) { // eq_pair contains pairs of original state names
        unite_sets(eq_pair.first, eq_pair.second);
    }

    // 2. Map Representatives to Original Member State Pointers
    map<string, list<State*>> representative_to_original_members;
    list<State*> original_state_pointers;
    // Store pointers to original states before stateList is modified/cleared
    for (State& original_state_ref : stateList) {
        original_state_pointers.push_back(&original_state_ref);
    }
    for (State* original_state_ptr : original_state_pointers) {
        // find_set uses original state names and the global 'parent' map
        string rep = find_set(original_state_ptr->statename);
        representative_to_original_members[rep].push_back(original_state_ptr);
    }

    // 3. Create Temporary New States (in temp_new_states list)
    list<State> temp_new_states; // This will hold the newly formed State objects
    map<string, string> rep_to_new_name_map; // Maps original representative name to new state name

    for (auto const& [original_rep_name, members_list_of_original_ptrs] : representative_to_original_members) {
        State new_state_obj; // The new State object we are constructing
        string generated_name;

        bool is_single_true_unmerged = false;
        if (members_list_of_original_ptrs.size() == 1) {
            // A class of one. If its representative is itself, it's an unmerged state.
            if (members_list_of_original_ptrs.front()->statename == original_rep_name) {
                 is_single_true_unmerged = true;
            }
        }

        if (is_single_true_unmerged) {
            generated_name = original_rep_name; // Keep original name
        } else {
            // Merged state: concatenate sorted names of original members
            list<string> member_names_for_new_name;
            for (State* member_state_ptr : members_list_of_original_ptrs) {
                member_names_for_new_name.push_back(member_state_ptr->statename);
            }
            member_names_for_new_name.sort();
            generated_name = accumulate(member_names_for_new_name.begin(), member_names_for_new_name.end(), string(""),
                                      [](const string& a, const string& b) {
                                          return a.empty() ? b : a + "_" + b;
                                      });
        }
        new_state_obj.statename = generated_name;
        rep_to_new_name_map[original_rep_name] = generated_name; // Map original representative to the new state's name

        new_state_obj.startState = false;
        new_state_obj.finalState = false;
        for (State* member_state_ptr : members_list_of_original_ptrs) { // member_state_ptr points to an original state object
            if (member_state_ptr->startState) {
                new_state_obj.startState = true;
            }
            if (member_state_ptr->finalState) {
                new_state_obj.finalState = true;
            }
        }
        new_state_obj.exit_Manes_To_Other_states = new list<Mane>(); // Allocate new list for transitions
        temp_new_states.push_back(new_state_obj); // The new_state_obj is COPIED into temp_new_states
    }

    // Helper lambda to find states within a specific list of State objects (e.g., temp_new_states)
    auto find_state_in_given_list = [&](const string& name, list<State>& states_to_search) -> State* {
        for (State& s_ref : states_to_search) {
            if (s_ref.statename == name) {
                return &s_ref; // Returns pointer to object within states_to_search
            }
        }
        return nullptr;
    };

    // 4. Populate Transitions for states currently in `temp_new_states`
    //    Uses:
    //      - representative_to_original_members (contains State* to original states, whose transitions are still intact)
    //      - rep_to_new_name_map (maps original representative name to the new state name)
    //      - ::maneList (global list of all mane symbols)
    //    Targets:
    //      - Mane lists within State objects in `temp_new_states`.
    //      - Pointers (enteredstate, exitstate) in these new Manes will point to State objects *within `temp_new_states`*.
    for (auto const& [original_rep_name, original_members_list_ptrs] : representative_to_original_members) {
        if (original_members_list_ptrs.empty()) continue;

        string new_source_state_name = rep_to_new_name_map[original_rep_name];
        State* new_source_state_in_temp = find_state_in_given_list(new_source_state_name, temp_new_states);

        if (!new_source_state_in_temp) {
            cerr << "Critical Error during transition gen: Newly created source state '" << new_source_state_name << "' not found in temp_new_states list." << endl;
            continue;
        }
        if (!new_source_state_in_temp->exit_Manes_To_Other_states) {
            cerr << "Critical Error during transition gen: exit_Manes_To_Other_states is null for temp state '" << new_source_state_name << "'." << endl;
            continue;
        }

        for (const string& current_mane_name : ::maneList) {
            State* first_original_target_state_ptr = nullptr; // This will point to an ORIGINAL state object

            for (State* original_member_state_ptr : original_members_list_ptrs) {
                // original_member_state_ptr points to an original state, whose exit_Manes_To_Other_states is still valid.
                if (!original_member_state_ptr || !original_member_state_ptr->exit_Manes_To_Other_states) continue;

                for (const Mane& old_mane_transition : *(original_member_state_ptr->exit_Manes_To_Other_states)) {
                    if (old_mane_transition.name == current_mane_name) {
                        if (old_mane_transition.exitstate) { // This exitstate points to an ORIGINAL state
                           first_original_target_state_ptr = old_mane_transition.exitstate;
                           break; // Found a transition for current_mane_name from one of the original members
                        }
                    }
                }
                if (first_original_target_state_ptr) break; // Move to next mane_name
            }

            if (first_original_target_state_ptr) { // If a transition was found for current_mane_name
                string original_target_name = first_original_target_state_ptr->statename;
                // Find representative of this original target state's equivalence class
                string target_original_rep_name = find_set(original_target_name);
                // Get the new name for this target equivalence class
                string new_target_state_name = rep_to_new_name_map[target_original_rep_name];

                State* new_target_state_in_temp = find_state_in_given_list(new_target_state_name, temp_new_states);

                if (!new_target_state_in_temp) {
                    cerr << "Error during transition gen: New target state '" << new_target_state_name
                         << "' not found in temp_new_states for mane '" << current_mane_name
                         << "' from new source state '" << new_source_state_name << "'." << endl;
                    continue;
                }

                // Check if this exact transition (mane name to target state in temp_new_states) already added
                bool transition_exists = false;
                for (const Mane& existing_mane : *(new_source_state_in_temp->exit_Manes_To_Other_states)) {
                    if (existing_mane.name == current_mane_name && existing_mane.exitstate == new_target_state_in_temp) {
                        transition_exists = true;
                        break;
                    }
                }

                if (!transition_exists) {
                    Mane new_mane_obj;
                    new_mane_obj.name = current_mane_name;
                    new_mane_obj.enteredstate = new_source_state_in_temp; // Points to a State obj within temp_new_states
                    new_mane_obj.exitstate = new_target_state_in_temp;   // Points to a State obj within temp_new_states
                    new_source_state_in_temp->exit_Manes_To_Other_states->push_back(new_mane_obj);
                }
            }
        }
    }

    // 5. Cleanup Mane Lists of Original States (which are currently in the global stateList)
    //    The original State objects themselves will be cleared from global stateList next.
    for (State& old_state : stateList) { // stateList here is still the original list of states
        if (old_state.exit_Manes_To_Other_states != nullptr) {
            delete old_state.exit_Manes_To_Other_states;
            old_state.exit_Manes_To_Other_states = nullptr;
        }
    }
    stateList.clear(); // Empties the global stateList of its original State objects

    // 6. Set New Global stateList by COPYING State objects from temp_new_states
    stateList = temp_new_states;
    // CRITICAL: At this point, State objects have been COPIED from temp_new_states to global stateList.
    // The Mane pointers (enteredstate, exitstate) inside the new global stateList are now STALE.
    // They point to memory locations of State objects that were in temp_new_states (which will soon be out of scope).

    // 7. Re-wire Pointers in the new global stateList to point correctly within itself.
    auto find_state_in_final_global_list = [&](const string& name) -> State* {
        for (State& s_ref : stateList) { // stateList is NOW the new global list
            if (s_ref.statename == name) {
                return &s_ref; // Returns pointer to object within the final global stateList
            }
        }
        return nullptr;
    };

    for (State& final_state_ref : stateList) { // Iterate by reference through the new global stateList
        if (final_state_ref.exit_Manes_To_Other_states) {
            for (Mane& mane_to_fix : *(final_state_ref.exit_Manes_To_Other_states)) {
                // The 'statename' string within the pointed-to State object was copied correctly during the object copy.
                // We use this name to find the new memory location of that state in the final global stateList.
                if (mane_to_fix.enteredstate) {
                    string entered_name = mane_to_fix.enteredstate->statename;
                    mane_to_fix.enteredstate = find_state_in_final_global_list(entered_name);
                     if (!mane_to_fix.enteredstate) {
                        cerr << "Rewire Error: Entered state '" << entered_name << "' (for source " << final_state_ref.statename << " via mane " << mane_to_fix.name << ") not found in final list." << endl;
                    }
                }
                if (mane_to_fix.exitstate) {
                    string exit_name = mane_to_fix.exitstate->statename;
                    mane_to_fix.exitstate = find_state_in_final_global_list(exit_name);
                    if (!mane_to_fix.exitstate) {
                        cerr << "Rewire Error: Exit state '" << exit_name << "' (for source " << final_state_ref.statename << " via mane " << mane_to_fix.name << ") not found in final list." << endl;
                    }
                }
            }
        }
    }
    // temp_new_states (local list) goes out of scope here, its memory is reclaimed.
    // Pointers in the global stateList should now be valid and point within itself.
}


int main() {
    InputGetter_State_Mane();
    Connect_State_Mane();

    DFA_Delete_Trap_UnuseableState();
    State *startState = nullptr; // Ensure startState is initialized
    for (State& state : stateList) { // Use reference here
        if (state.startState) {
            startState = &state;
            DFA_SHow_Stucture(startState);
            // break; // Optional: if there's only one start state, can break early
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
         // If no start state, show all components, or just the first state.
         // For robust display, one might iterate all states or show multiple components if disjoint.
         // DFA_SHow_Stucture(&stateList.front()); // Example: Show from the first if no explicit start
         cout << "Minimized DFA has no designated start state. Showing all states:" << endl;
         DFA_SHow_Stucture(); // Call the version that prints all states if available & appropriate
    } else if (stateList.empty()) {
        cout << "DFA is empty after minimization." << endl;
    }

    cleanup(); // Add this line
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
        for (State *trapState_ptr : trapStates) { // trapStates contains State* pointers
            cout << "- " << trapState_ptr->statename;
            if (trapState_ptr->finalState) cout << " (Was Final State)";
            if (trapState_ptr->startState) cout << " (Was Start State)";
            cout << endl;

            // Deallocate the Mane list for this trap state BEFORE it's removed from stateList
            if (trapState_ptr->exit_Manes_To_Other_states != nullptr) {
                delete trapState_ptr->exit_Manes_To_Other_states;
                trapState_ptr->exit_Manes_To_Other_states = nullptr; // Good practice
            }
        }

        // Now, remove trap states from stateList
        // The predicate for remove_if now checks if a state in stateList corresponds to any of the trapState_ptr by statename
        stateList.remove_if([&trapStates](const State& s_in_list) {
            // Check if s_in_list.statename matches any statename in trapStates (list of State*)
            return std::any_of(trapStates.begin(), trapStates.end(),
                               [&s_in_list](const State* trap_ptr) {
                                   // Ensure trap_ptr is not null before dereferencing, though it shouldn't be here.
                                   if (trap_ptr) {
                                       return trap_ptr->statename == s_in_list.statename;
                                   }
                                   return false;
                               });
        });

        cout << "\nTrap states have been removed from the DFA and their resources deallocated." << endl;
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