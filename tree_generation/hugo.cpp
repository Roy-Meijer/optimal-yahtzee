#include <iostream> //cout cin
#include <stdlib.h> //rand

#define NR_OF_DICE 2
#define NR_OF_SIDES 2
#define NR_OF_REROLLS 1
#define NR_OF_SCORE_SECTIONS 2
#define NR_OF_ROUNDS 2 //should be equal to NR_OF_SCORE_SECTIONS, but made separate for testing purposes.

int global_ID_incrementer = {};

// forward declarations of structs (needed when using an array of pointers)
struct DiceNode;
struct RerollChoiceNode;
struct DiceRerollOptionsNode;

struct RootNode
{
    // id of node
    int ID = {};
    // should contain ES

    // scorecard:
    //int scorecard[NR_OF_SCORE_SECTIONS] = {};
    // 4 dice_node children
    DiceNode* dice_nodes[4];
};

struct DiceNode
{
    // id of node
    int ID = {};
    // the specific dice roll (array of values)
    //int diceRoll[NR_OF_DICE] = {};
    // the probability of getting this dice roll (for naive implementation all probabilities are the same since we don't cluster)
    //double probability = {};
    // 4 choice_node children
    RerollChoiceNode* reroll_choices_nodes[4];
};

struct RerollChoiceNode
{
    // id of node
    int ID = {};
    // (should contain ES)
    //
    // contains 1, 2 or 3 DiceRerollChoiceNode children
    DiceRerollOptionsNode* dice_reroll_options_nodes[3];
};

struct DiceRerollOptionsNode
{
    // id of node
    int ID = {};
    // the specific dice roll (array of values)
    // int diceRoll[NR_OF_DICE] = {};
    // (the probability)
    //
    // 2 root node children (ones or twos)
    RootNode* root_nodes[2];
};

void generateTree(RootNode* root, int depth) {
    // If we've reached the desired depth, stop generating new nodes.
    if (depth == 0) {
        return;
    }

    // set ID:
    root->ID = global_ID_incrementer;
    ++global_ID_incrementer;

    // For each of the four ways to throw two two-sided dice...
    for (int i = 0; i < 4; i++) {
        // Create an array of four pointers to DiceNode objects.
        DiceNode* dice_nodes[4];

        // For each DiceNode...
        for (int j = 0; j < 4; j++) {
            // Create a new DiceNode object and store its pointer in the dice_nodes array.
            DiceNode* dice_node = new DiceNode();
            dice_nodes[j] = dice_node;
            // set ID:
            //dice_node->ID = global_ID_incrementer;
            //++global_ID_incrementer;

            // For each RerollChoiceNode...
            for (int k = 0; k < 4; k++) {
                // Create a new RerollChoiceNode object and add it to the current DiceNode's
                // reroll_choices array.
                RerollChoiceNode* reroll_choice_node = new RerollChoiceNode();
                dice_node->reroll_choices_nodes[k] = reroll_choice_node;

                // For each DiceRerollOptionsNode...
                for (int l = 0; l < 4; l++){
                    // Create a new DiceRerollOptionsNode object and add it to the current RerollChoiceNode's
                    // dice_reroll_options_nodes array.
                    DiceRerollOptionsNode* diceReroll_options_node = new DiceRerollOptionsNode();
                    dice_node->reroll_choices_nodes[k]->dice_reroll_options_nodes[l] = diceReroll_options_node;

                    // For each child of the DiceRerollOptionsNode...
                    for (int m = 0; m <= 2; m++) {
                        // Create a new RootNode object and add it to the current DiceRerollOptionsNode's
                        // root_nodes array.
                        RootNode* child_node = new RootNode();
                        diceReroll_options_node->root_nodes[m] = child_node;

                        // Recursively generate the tree for the child node.
                        generateTree(child_node, depth - 1);
                    }
                }
                
            }

            // Add the current DiceNode to the root node's dice_nodes array.
            root->dice_nodes[j] = dice_node;
        }
    }
}

int main ( )
{  
    // Create a new RootNode object.
    RootNode* root = new RootNode();

    // Generate the tree for the root node.
    generateTree(root, 2);

    // print the tree (for now just look in debugger)

    // clean up the memory of the tree:

    return 0;
}