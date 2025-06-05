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

void Create_Minimized_DFA(const list<pair<string, string>>& equivalent_pairs);

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

void Create_Minimized_DFA(const list<pair<string, string>>& equivalent_pairs) {
    if (equivalent_pairs.empty()) {
        cout << "\nNo equivalent states found. The DFA is already minimal." << endl;
        return;
    }

    cout << "\n=== Creating Minimized DFA ===" << endl;
    
    // Create sets of equivalent states
    list<list<string>> equivalent_groups;
    list<string> processed_states;

    // Group equivalent states together
    for (const auto& pair : equivalent_pairs) {
        bool found = false;
        for (auto& group : equivalent_groups) {
            if (find(group.begin(), group.end(), pair.first) != group.end() ||
                find(group.begin(), group.end(), pair.second) != group.end()) {
                // Add both states to the group if not already present
                if (find(group.begin(), group.end(), pair.first) == group.end())
                    group.push_back(pair.first);
                if (find(group.begin(), group.end(), pair.second) == group.end())
                    group.push_back(pair.second);
                found = true;
                break;
            }
        }
        if (!found) {
            // Create new group
            list<string> new_group = {pair.first, pair.second};
            equivalent_groups.push_back(new_group);
        }
        processed_states.push_back(pair.first);
        processed_states.push_back(pair.second);
    }

    // Create new states for each group and standalone states
    list<State> minimized_states;
    
    // First, add grouped states
    for (const auto& group : equivalent_groups) {
        State new_state;
        string combined_name;
        bool is_start = false;
        bool is_final = false;
        
        // Combine names and check properties
        for (const auto& state_name : group) {
            if (!combined_name.empty()) combined_name += "_";
            combined_name += state_name;
            
            // Find original state
            auto orig_state = find_if(stateList.begin(), stateList.end(),
                [&state_name](const State& s) { return s.statename == state_name; });
            
            if (orig_state->startState) is_start = true;
            if (orig_state->finalState) is_final = true;
        }
        
        new_state.statename = combined_name;
        new_state.startState = is_start;
        new_state.finalState = is_final;
        new_state.exit_Manes_To_Other_states = new list<Mane>();
        
        // Take transitions from first state in group (they're equivalent, so any state's transitions work)
        auto first_state = find_if(stateList.begin(), stateList.end(),
            [&group](const State& s) { return s.statename == group.front(); });
            
        // Copy transitions, updating destination states if needed
        for (const Mane& old_mane : *(first_state->exit_Manes_To_Other_states)) {
            Mane new_mane = old_mane;
            new_mane.enteredstate = &new_state;
            
            // Update exit state if it's part of a group
            string exit_name = old_mane.exitstate->statename;
            for (const auto& eq_group : equivalent_groups) {
                if (find(eq_group.begin(), eq_group.end(), exit_name) != eq_group.end()) {
                    // Find or create combined state
                    string combined_exit_name;
                    for (const auto& s : eq_group) {
                        if (!combined_exit_name.empty()) combined_exit_name += "_";
                        combined_exit_name += s;
                    }
                    exit_name = combined_exit_name;
                    break;
                }
            }
            
            // Find or create the exit state
            auto exit_state = find_if(minimized_states.begin(), minimized_states.end(),
                [&exit_name](const State& s) { return s.statename == exit_name; });
                
            if (exit_state == minimized_states.end()) {
                // Create new state if it doesn't exist yet
                State new_exit_state;
                new_exit_state.statename = exit_name;
                new_exit_state.exit_Manes_To_Other_states = new list<Mane>();
                
                // Find original state to copy properties
                auto orig_exit = find_if(stateList.begin(), stateList.end(),
                    [&old_mane](const State& s) { return s.statename == old_mane.exitstate->statename; });
                    
                new_exit_state.startState = orig_exit->startState;
                new_exit_state.finalState = orig_exit->finalState;
                
                minimized_states.push_back(new_exit_state);
                exit_state = --minimized_states.end();
            }
            
            new_mane.exitstate = &(*exit_state);
            new_state.exit_Manes_To_Other_states->push_back(new_mane);
        }
        
        minimized_states.push_back(new_state);
    }
    
    // Add remaining states that weren't combined
    for (const auto& old_state : stateList) {
        if (find(processed_states.begin(), processed_states.end(), old_state.statename) == processed_states.end()) {
            State new_state = old_state;
            new_state.exit_Manes_To_Other_states = new list<Mane>();
            
            // Copy transitions, updating destination states if needed
            for (const Mane& old_mane : *(old_state.exit_Manes_To_Other_states)) {
                Mane new_mane = old_mane;
                new_mane.enteredstate = &new_state;
                
                // Update exit state if it's part of a group
                string exit_name = old_mane.exitstate->statename;
                for (const auto& group : equivalent_groups) {
                    if (find(group.begin(), group.end(), exit_name) != group.end()) {
                        string combined_name;
                        for (const auto& s : group) {
                            if (!combined_name.empty()) combined_name += "_";
                            combined_name += s;
                        }
                        exit_name = combined_name;
                        break;
                    }
                }
                
                // Find or create the exit state
                auto exit_state = find_if(minimized_states.begin(), minimized_states.end(),
                    [&exit_name](const State& s) { return s.statename == exit_name; });
                    
                if (exit_state == minimized_states.end()) {
                    State new_exit_state;
                    new_exit_state.statename = exit_name;
                    new_exit_state.exit_Manes_To_Other_states = new list<Mane>();
                    new_exit_state.startState = old_mane.exitstate->startState;
                    new_exit_state.finalState = old_mane.exitstate->finalState;
                    minimized_states.push_back(new_exit_state);
                    exit_state = --minimized_states.end();
                }
                
                new_mane.exitstate = &(*exit_state);
                new_state.exit_Manes_To_Other_states->push_back(new_mane);
            }
            
            minimized_states.push_back(new_state);
        }
    }
    
    // Clean up old states
    for (State& state : stateList) {
        delete state.exit_Manes_To_Other_states;
    }
    
    // Replace old DFA with minimized one
    stateList = minimized_states;
    
    cout << "\nMinimized DFA structure:" << endl;
    DFA_SHow_Stucture();
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

    // Check transitions for each pair
    auto it = allTople.begin();
    while (it != allTople.end()) {
        // Find the actual states for this pair
        auto state1 = find_if(stateList.begin(), stateList.end(),
            [&it](const State &s) { return s.statename == it->first; });
        auto state2 = find_if(stateList.begin(), stateList.end(),
            [&it](const State &s) { return s.statename == it->second; });

        bool shouldDelete = false;
        
        // Check each mane
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

            // If one state has a transition and the other doesn't
            if (hasTransition1 != hasTransition2) {
                shouldDelete = true;
                cout << "\nPair " << it->first << it->second << " deleted because only one state has transition on mane " << mane << endl;
                break;
            }

            // If both states have transitions, check if they go to different states
            if (hasTransition1 && hasTransition2) {
                if (nextState1->statename != nextState2->statename) {
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
        }

        if (shouldDelete) {
            deletedTople.push_back(*it);
            it = allTople.erase(it);
        } else {
            ++it;
        }
    }

    // Show final results
    cout << "\nFinal results after checking transitions:" << endl;
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

    // Create minimized DFA
    Create_Minimized_DFA(allTople);
}

void cleanup() {
    for (State &state: stateList) {
        delete state.exit_Manes_To_Other_states;
    }
}
