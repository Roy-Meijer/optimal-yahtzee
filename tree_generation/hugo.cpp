#include <iostream> //cout cin
#include <stdlib.h> //rand

#define NR_OF_DICE 2
#define NR_OF_SIDES 2
#define NR_OF_REROLLS 1
#define NR_OF_SCORE_SECTIONS 2
#define NR_OF_ROUNDS 2 //should be equal to NR_OF_SCORE_SECTIONS, but made separate for testing purposes.

// forward declarations of structs (needed when using an array of pointers in the structs)
struct DiceNode;
struct RerollChoiceNode;
struct DiceRerollOptionsNode;

struct RootNode
{
    // id of node
    int ID = {};
    // should contain ES
    //
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
    // 
    // the probability of getting this dice roll (for naive implementation all probabilities are the same since we don't cluster)
    // 
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
    // 
    // the probability of getting this dice roll (for naive implementation all probabilities are the same since we don't cluster)
    //
    // 2 root node children (ones or twos)
    RootNode* root_nodes[2];
};

void generateTree(RootNode* root, int depth) 
{
    // If we've reached the desired depth, stop generating new nodes.
    if (depth == 0) {
        return;
    }

    // For each of the four ways to throw two two-sided dice...
    for (int i = 0; i < 4; i++) {
        // Create an array of four pointers to DiceNode objects.
        DiceNode* dice_nodes[4];

        // For each DiceNode...
        for (int j = 0; j < 4; j++) {
            // Create a new DiceNode object and store its pointer in the dice_nodes array.
            DiceNode* dice_node = new DiceNode();
            dice_nodes[j] = dice_node;

            // For each RerollChoiceNode...
            for (int k = 0; k < 4; k++) {
                // Create a new RerollChoiceNode object and add it to the current DiceNode's
                // reroll_choices array.
                RerollChoiceNode* reroll_choice_node = new RerollChoiceNode();
                dice_node->reroll_choices_nodes[k] = reroll_choice_node;

                // For each DiceRerollOptionsNode...
                for (int l = 0; l < 3; l++){
                    // Create a new DiceRerollOptionsNode object and add it to the current RerollChoiceNode's
                    // dice_reroll_options_nodes array.
                    DiceRerollOptionsNode* diceReroll_options_node = new DiceRerollOptionsNode();
                    dice_node->reroll_choices_nodes[k]->dice_reroll_options_nodes[l] = diceReroll_options_node;

                    // For each child of the DiceRerollOptionsNode...
                    for (int m = 0; m < 2; m++) {
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

int generateIDs(RootNode* root)
{
    // This is not yet working. It should be recursive, it is not.
    // It does work for one round of the tree as it returns 164 nodes, and my by-hand-calculations give 165)

    // Variable to give each node a unique ID, to count how many there are
    int node_ID = 0;

    // for all 4 root children:
    for (int i = 0; i < 4; i++) {
        root->dice_nodes[i]->ID = ++node_ID;
        // for all 4 of their children:
        for (int j = 0; j < 4; j++) {
            root->dice_nodes[i]->reroll_choices_nodes[j]->ID = ++node_ID;
            // for their 3 children:
            for (int k = 0; k < 3; k++) {
                root->dice_nodes[i]->reroll_choices_nodes[j]->dice_reroll_options_nodes[k]->ID = ++node_ID;
                // for their two children:
                for (int l = 0; l < 2; l++) {
                    root->dice_nodes[i]->reroll_choices_nodes[j]->dice_reroll_options_nodes[k]->root_nodes[l]->ID = ++node_ID;
                    // should enter the function again recursively here.
                }
            }
        }
    }
    return node_ID;
}

void printTree(RootNode* root) 
{
    // Print the root node.
    std::cout << "Root node: " << root << std::endl;

    // Print the four dice nodes.
    for (int i = 0; i < 4; i++) {
        std::cout << "    Dice node " << i << ": " << root->dice_nodes[i]->ID << std::endl;

        // Print the four reroll choice nodes for each dice node.
        for (int j = 0; j < 4; j++) {
            std::cout << "        Reroll choice node " << j << " of dice node " << i << ": "
                      << root->dice_nodes[i]->reroll_choices_nodes[j]->ID << std::endl;

            // Print the up to three child nodes for each reroll choice node.
            for (int k = 0; k <= 2; k++) {
                std::cout << "            Dice reroll options node " << k << " of reroll choice node " << j
                          << " of dice node " << i << ": "
                          << root->dice_nodes[i]->reroll_choices_nodes[j]->dice_reroll_options_nodes[k]->ID << std::endl;

                // Print the root nodes for each child node.
                if (root->dice_nodes[i]->reroll_choices_nodes[j]->dice_reroll_options_nodes[k] != nullptr) {
                    std::cout << "                Child nodes of dice reroll options node " << k
                              << " of reroll choice node " << j << " of dice node " << i << ": " << std::endl;
                    for (int l = 0; l < 2; l++) {
                        std::cout << "                    Root node " << l << " of child node " << k
                                  << " of dice reroll options node " << k << " of reroll choice node " << j
                                  << " of dice node " << i << ": "
                                  << root->dice_nodes[i]->reroll_choices_nodes[j]->dice_reroll_options_nodes[k]->root_nodes[l]->ID << std::endl;
                    }
                }
            }
        }
    }
}

int main ( )
{  
    // Create a new RootNode object.
    RootNode* root = new RootNode();

    // Generate the tree for the root node.
    generateTree(root, 2);

    // set ID for all nodes:
    int treeSize = {};
    treeSize = generateIDs(root);
    std::cout << "There are " << treeSize << " nodes in the tree.\n";

    // print the tree (or just look in debugger)
    printTree(root);

    // clean up the memory of the tree:
    return 0;
}