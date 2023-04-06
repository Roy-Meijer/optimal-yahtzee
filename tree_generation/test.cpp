#include <iostream> //cout cin

struct DiceNode;
struct RerollChoiceNode;

// Struct representing a root node in the game tree.
struct RootNode {
    // Array of pointers to DiceNode objects.
    DiceNode* dice_nodes[4];
};

// Struct representing a node in the game tree corresponding to a roll of two dice.
struct DiceNode {
    // Array of pointers to RerollChoiceNode objects.
    RerollChoiceNode* reroll_choices[4];
};

// Struct representing a node in the game tree corresponding to a choice to reroll one or both dice.
struct RerollChoiceNode {
    // Array of pointers to RootNode objects.
    RootNode* root_nodes[3];
};

// Function to generate a tree of nodes representing a game with two two-sided dice.
// The tree consists of a root node, every root node has four dice_node children
// (since there are four ways to throw two two-sided dice), every dice_node child
// has 4 reroll_choice children (since you can choose to not reroll, or reroll one dice,
// or the other, or both), every one of those reroll_choice node children has at most 3
// children, and those children have two root nodes as children.
void generateTree(RootNode* root, int depth) {
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
                dice_node->reroll_choices[k] = reroll_choice_node;

                // For each child of the RerollChoiceNode...
                for (int l = 0; l <= 2; l++) {
                    // Create a new RootNode object and add it to the current RerollChoiceNode's
                    // root_nodes array.
                    RootNode* child_node = new RootNode();
                    reroll_choice_node->root_nodes[l] = child_node;

                    // Recursively generate the tree for the child node.
                    generateTree(child_node, depth - 1);
                }
            }

            // Add the current DiceNode to the root node's dice_nodes array.
            root->dice_nodes[j] = dice_node;
        }
    }
}

int main() {
    // Create a new RootNode object.
    RootNode* root = new RootNode();

    // Generate the tree for the root node.
    generateTree(root, 2);

    // Output the pointers to the generated nodes.
    std::cout << "Pointers to generated nodes:" << std::endl;
    std::cout << "Root node: " << root << std::endl;
    for (int i = 0; i < 4; i++) {
        std::cout << "Dice node " << i << ": " << root->dice_nodes[i] << std::endl;
        for (int j = 0; j < 4; j++) {
            std::cout << "Reroll choice node " << j << " of dice node " << i << ": "
                      << root->dice_nodes[i]->reroll_choices[j] << std::endl;
            for (int k = 0; k <= 2; k++) {
                std::cout << "Child node " << k << " of reroll choice node " << j
                          << " of dice node " << i << ": "
                          << root->dice_nodes[i]->reroll_choices[j]->root_nodes[k]
                          << std::endl;
            }
        }
    }

    // Clean up the memory allocated for the nodes.
    for (int i = 0; i < 4; i++) {
        delete root->dice_nodes[i];
    }
    delete root;

    return 0;
}
