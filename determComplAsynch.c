#include "determComplAsynch.h"

FA* determComplAsynch(FA* fa){
    // Find '*'-elimination table states
    int** synchStates = malloc(fa->nbStates*sizeof(int*));
    for(int i = 0; i < fa->nbStates; i++){
        synchStates[i] = malloc((fa->nbStates+1)*sizeof(int));
        synchStates[i][0] = 0;
    }
    // Add initial states
    for(int i = 1; i <= fa->init[0]; i++){
        synchStates[0][0]++;
        synchStates[0][i] = fa->init[i]-1;
    }
    int nb = 1;
    for(int i = 1; i <= fa->nbStates; i++){
        // Add targets of non '*'-transition
        if(!inArray(fa->transTable[i][0][1], synchStates[0]) && target(fa->transTable[i][0][1], fa)){
            synchStates[nb][0]++;
            synchStates[nb][1] = fa->transTable[i][0][1];
            nb++;
        }
    }
    // Make '*'-closures (states reachable with '*'-transitions)
    for(int i = 0; i < nb; i++){
        closures(synchStates[i], fa);
    }

    // Construct synchronous fa
    FA* synchFa = malloc(sizeof(FA));
    synchFa->nbAlpha = fa->nbAlpha;
    synchFa->nbStates = nb;

    // Initialize the table
    synchFa->transTable = malloc((synchFa->nbStates+1)*sizeof(int**));
    for(int i = 0; i < synchFa->nbStates+1; i++){
        synchFa->transTable[i] = malloc((synchFa->nbAlpha+2)*sizeof(int*));
        for(int j = 0; j < synchFa->nbAlpha+2; j++){
            synchFa->transTable[i][j] = malloc((synchFa->nbStates+1)*sizeof(int));
            synchFa->transTable[i][j][0] = 0;
        }
    }

    // Fill alphabet
    for(int j = 1; j <= synchFa->nbAlpha; j++){
        synchFa->transTable[0][j][0] = 96+j;
    }
    synchFa->transTable[0][synchFa->nbAlpha+1][0] = 42;

    // Construction of the table
    synchFa->init = malloc(2*sizeof(int));
    synchFa->init[0] = 1;
    synchFa->init[1] = 1;

    // Import states and transitions
    for(int i = 1; i <= synchFa->nbStates; i++){
        // Add state
        synchFa->transTable[i][0][0]++;
        synchFa->transTable[i][0][1] = synchStates[i-1][1];
        for(int j = 1; j <= synchFa->nbAlpha; j++){
            for(int k = 1; k <= synchStates[i-1][0]; k++){
                for(int l = 1; l <= fa->transTable[searchLin(fa, synchStates[i-1][k])][j][0]; l++){
                    // Add transitions of closures if they are not already in it
                    if(!inArray(fa->transTable[searchLin(fa, synchStates[i-1][k])][j][l], synchFa->transTable[i][j])){
                        int nb = ++synchFa->transTable[i][j][0];
                        synchFa->transTable[i][j][nb] = fa->transTable[searchLin(fa, synchStates[i-1][k])][j][l];
                    }
                }
            }
        }
    }

    // Find final states
    synchFa->term = malloc((synchFa->nbStates+1)*sizeof(int));
    synchFa->term[0] = 0;
    int final, j;
    for(int i = 1; i <= synchFa->nbStates; i++){
        final = 0;
        j = 1;
        while(j <= fa->term[0] && !final){
            // Terminal if one original terminal state in the closure
            if(inArray(fa->term[j]-1, synchStates[i-1])){
                final = 1;
                synchFa->term[0]++;
                synchFa->term[synchFa->term[0]] = i;
            }
            j++;
        }
    }

    printf("\n*** Synchronous automaton ***\n");
    displayAutomaton(synchFa);
    FA* detFa = determCompl(synchFa);
    freeFa(fa);

    return detFa;
}

int target(int x, FA* fa){
    for(int i = 1; i <= fa->nbStates; i++){
        for(int j = 1; j <= fa->nbAlpha; j++){
            // x is a target if it is in a transition (excluding '*')
            if(inArray(x, fa->transTable[i][j]))
                return 1;
        }
    }
    return 0;
}

void closures(int* array, FA* fa){
    int nb = 0;
    // Stop when no more closure found
    while(array[0] != nb){
        nb = array[0];
        for(int i = 1; i <= nb; i++){
            for(int j = 1; j <= fa->transTable[searchLin(fa, array[i])][fa->nbAlpha+1][0]; j++){
                // Add states reachable with '*'-transitions from states already in the array
                if(!inArray(fa->transTable[searchLin(fa, array[i])][fa->nbAlpha+1][j], array)){
                    array[0]++;
                    array[array[0]] = fa->transTable[searchLin(fa, array[i])][fa->nbAlpha+1][j];
                }
            }
        }
    }
}