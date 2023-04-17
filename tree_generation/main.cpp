#include <iostream>
#include <vector>
#include <algorithm>
//--------------------------
#include <graphviz/gvc.h>
#include <graphviz/cgraph.h>
//--------------------------
#include "Node.h"

// Optimizations: ------------------------------------------
//#define NAIVE 1            // Upper bound for amount of nodes
#define DICE_ROLLS_EQUAL 1  // OPTIMIZATION 1: Rolling 1 and 2 is the same as rolling 2 and 1
#define REROLL_SAME 1       // OPTIMIZATION 2: Rerolling dice 1 and dice 2 is the same if in the first throw the dice were the same.
#define REROLLS_OPTIMIZED 1 // OPTIMIZATION 3: Rerolling 1 dice shall have 2 children only, rerolling 2 dice of same value should have 3 children and rerolling none should only have one child (passthrough)
//-----------------------------------------------------------

#define NR_OF_ROUNDS 2

int nodeCount= {};

void generateNodeTree(Node* node, int depth) {
    // Break if we have reached the necessary depth
    if (depth == 0) {
        return;
    }

    Node::NodeType childType;
    int numChildren = {};
    if (node->getType() == Node::ROOT_NODE) {
        childType = Node::DICE_NODE;
        #ifdef NAIVE
        // There are four children. Throwing 1 and 1, throwing 1 and 2, throwing 2 and 1 , and finally throwing 2 and 2
        numChildren = 4;
        #endif

        #ifdef DICE_ROLLS_EQUAL
        // There are three children. Throwing 1 and 1, (throwing 1 and 2 AND throwing 2 and 1), and finally throwing 2 and 2
        numChildren = 3;
        #endif
    } else if (node->getType() == Node::DICE_NODE) {
        childType = Node::REROLL_CHOICE_NODE;
        // There are four children. Don't reroll, reroll dice 1, reroll dice 2, or rerolling both dice.
        numChildren = 4;
        

        // When first roll is 1,1 or 2,2 then rerolling dice 1 and dice 2 is the same. Gives three children!
        #ifdef REROLL_SAME
        if( (node->diceOne == 1 && node->diceTwo == 1) || (node->diceOne == 2 && node->diceTwo == 2))
        {
            numChildren = 3;
        }
        #endif
    } else if (node->getType() == Node::REROLL_CHOICE_NODE) {
        childType = Node::DICE_REROLL_OPTIONS_NODE;
        #ifdef NAIVE
        // There are (up to) 4 children. If the parent was a reroll both, then we get four children:
        // 1,1 and 1,2 and 2,1 and 2,2.
        // If the parent only rerolled dice 1, (and dice 2 has value x) we get
        // 1,x and 2,x.
        // vice versa for the other dice
        // If no dice are rerolled in the parent, it depends on how we implement it. I guess 1 child makes most sense
        // x,y where the values x and y are just the same values of the dice that were in the first round (remember, no rerolls here)
        numChildren = 4;
        #endif
        #ifdef DICE_ROLLS_EQUAL
        // There are (up to) 3 children. If the parent was a reroll-both, then we get three children:
        // 1,1 and (1,2 AND 2,1) and 2,2.
        // If the parent only rerolled dice 1, (and dice 2 has value x) we get
        // 1,x and 2,x.
        // vice versa for the other dice
        // If no dice are rerolled in the parent, it depends on how we implement it. I guess 1 child makes most sense
        // x,y where the values x and y are just the same values of the dice that were in the first round (remember, no rerolls here)
        numChildren = 3;
        #endif
        #ifdef REROLLS_OPTIMIZED
        // depending on the decision that was taken in the choice node the number of children will differ
        // No rerolls: Only one child (the same outcome of the parent. i.e., die values do not change)
        // reroll dice one/reroll dice two: two children each (rerolled die either one or two, not rerolled die stays the same)
        // reroll both: has three children (1,1), (1,2)/(2,1), (2,2)

        // Needs to be implemented: the previous decision (Reroll one, two or none)
        if (node->getRerollDecision() == Node::REROLL_TYPE::NO_REROLLS)
            numChildren = 1;
        else if (node->getRerollDecision() == Node::REROLL_TYPE::REROLL_ONEDICE) 
            numChildren = 2;
        else if (node->getRerollDecision() == Node::REROLL_TYPE::REROLL_BOTH) 
            numChildren = 3;
        #endif

    } else if (node->getType() == Node::DICE_REROLL_OPTIONS_NODE) {
        childType = Node::ROOT_NODE;
        if (depth > 1) {
            // There are two ways to score in the first round
            numChildren = 2;
        } else {
            // This is for the final nodes, now there is only one remaining option
            numChildren = 1;
        }
    } else {
        return;
    }

    for (int i = 0; i < numChildren; ++i) {
        Node* child = new Node(childType);
        node->addChild(child);

        
        // Adding dice rolls to the DICE_NODEs.
        #ifdef REROLL_SAME
        // If the current node is a ROOT_NODE, the children are DICE_NODE, and should contain the dice rolls.
        if(node->getType() == Node::NodeType::ROOT_NODE)
        {
            if(i == 0)
            {
                // This is why I made the children public. I tried doing node->getChild()->diceOne but it didn't work.
                // other fixes might be to make this generateNodeTree function a friend of the Node class (so it can access private members)
                // ??
                // Anyway, this works.
                node->children[i]->diceOne = 1;
                node->children[i]->diceTwo = 1;
            }
            // This is the dual case (so right now this only works correctly when the NAIVE optimization is NOT ON, but the 1,2=2,1 optimization is ON)
            if(i == 1) 
            {
                node->children[i]->diceOne = 1;
                node->children[i]->diceTwo = 2;
            }
            if(i == 2)
            {
                node->children[i]->diceOne = 2;
                node->children[i]->diceTwo = 2;
            }           

        }
        #endif

        //Assigning the reroll decision for the children of dice nodes
        #ifdef REROLLS_OPTIMIZED
         if(node->getType() == Node::NodeType::DICE_NODE)
        {
            if (i == 0)    
            {
                node->setRerollDecision(node->children[i], Node::REROLL_TYPE::NO_REROLLS);
            }
            //if there are 3 child nodes, the second will be reroll one dice
            // if there are 4 child nodes, the second and the third will be roll one dice
            else if ( (i == 1) || (i == 2 && numChildren == 4))    
            {
                node->setRerollDecision(node->children[i], Node::REROLL_TYPE::REROLL_ONEDICE);
            }
            else if ( (i == 2 && numChildren == 3) || i == 3)    
            {
                node->setRerollDecision(node->children[i], Node::REROLL_TYPE::REROLL_BOTH);
            }
        }
        #endif

        // Here we count how many nodes there are
        ++nodeCount;
        // Recursively make children
        generateNodeTree(child, depth - 1); 
    }
}

void printGraphvizNodeTree(const Node* node, Agraph_t* graph, Agnode_t* parentAgNode, int depth, int siblingIndex, int maxSiblings) {
    static const std::string TYPE_STRINGS[] = {
        "roll",
        "dice",
        "reroll",
        "redice"
    };
    static const std::string COLORS[] = {
        "red",
        "green",
        "blue",
        "purple"
    };

    static const std::string TYPE_REROLL[] = {
        "reroll none",
        "reroll one",
        "reroll both"
    };

    if (siblingIndex >= maxSiblings) {
        if (siblingIndex == maxSiblings) {
           std::string nodeName = "ellipsis_" + std::to_string(depth) + "_" + std::to_string(reinterpret_cast<uintptr_t>(node));
            Agnode_t* currentNode = agnode(graph, const_cast<char*>(nodeName.c_str()), 1);
            agsafeset(currentNode, const_cast<char*>("color"), const_cast<char*>(COLORS[node->getType()].c_str()), const_cast<char*>(""));
            agset(currentNode, const_cast<char*>("label"), const_cast<char*>("..."));
            agedge(graph, parentAgNode, currentNode, nullptr, 1);
        }
        return;
    }

    std::string nodeName = TYPE_STRINGS[node->getType()] + "_" + TYPE_REROLL[node->getRerollDecision()] + "_" +  std::to_string(reinterpret_cast<uintptr_t>(node));
    Agnode_t* currentNode = agnode(graph, const_cast<char*>(nodeName.c_str()), 1);
    agsafeset(currentNode, const_cast<char*>("color"), const_cast<char*>(COLORS[node->getType()].c_str()), const_cast<char*>(""));

    if (parentAgNode) {
        agedge(graph, parentAgNode, currentNode, nullptr, 1);
    }

    int childIndex = 0;
    for (const auto& child : node->getChildren()) {
         // This is the recursive part
        printGraphvizNodeTree(child, graph, currentNode, depth + 1, childIndex, maxSiblings);
        ++childIndex;
    }
}

void printTreeAsGraph(const std::vector<Node*>& rootNodes, const std::string& outputFilename, const std::string& outputFiletype, int maxSiblings = 1) {
    GVC_t* gvc = gvContext();
    Agraph_t* graph = agopen(const_cast<char*>("NodeTree"), Agdirected, nullptr);

    Agnode_t* sharedParentNode = agnode(graph, const_cast<char*>("start"), 1);
    agset(sharedParentNode, const_cast<char*>("color"), const_cast<char*>("black"));

    int initialSiblingIndex = 0;

    // Recursively generate tree visualisation
    for (const auto& node : rootNodes) {
        printGraphvizNodeTree(node, graph, sharedParentNode, 0, initialSiblingIndex, maxSiblings);
        ++initialSiblingIndex;
    }

    agattr(graph, AGRAPH, (char *)"rankdir", (char *)"LR");  //print from left to right
    agattr(graph, AGRAPH, (char *)"nodesep", (char *)"1.5"); // increase the nodesep attribute
    gvLayout(gvc, graph, "dot"); //you can try to replace dot by neato, maybe it looks better (might take longer to generate). there are more options: https://graphviz.org/docs/layouts/

    // Save to 'graphs' folder:
    std::string outputFolderName = "graphs/";
    std::string outputFilenameIncludingFolder = outputFolderName + outputFilename;
    gvRenderFilename(gvc, graph, const_cast<char*>(outputFiletype.c_str()), const_cast<char*>(outputFilenameIncludingFolder.c_str()));
    gvFreeLayout(gvc, graph);

    agclose(graph);
    gvFreeContext(gvc);
}

int main() {
    srand(static_cast<unsigned>(time(NULL)));

    #ifdef NAIVE
    std::cout << "Using naive tree generation...\n";
    #endif

    #ifdef DICE_ROLLS_EQUAL
    std::cout << "Using optimization: (1,2) = (2,1)...\n";
    #endif

    #ifdef NAIVE
    #ifdef DICE_ROLLS_EQUAL
    std::cout << "ERROR! These shouldn't be on at the same time.\n";
    #endif
    #endif

    #ifdef REROLL_SAME
    std::cout << "Using optimization: reroll d1 = reroll d2 (when the parent dice is 1,1 or 2,2)...\n";
    #endif

    #ifdef REROLLS_OPTIMIZED
    std::cout << "Using optimization: reroll d1/d2 has two children, reroll d1 AND d2 has three children...\n";
    #endif

    // Make 1 root node
    std::vector<Node*> rootNodes(1);
    // (don't forget to count it)
    ++nodeCount;

    // Generate tree of two rounds (each round has 4 levels of nodes)
    for (auto& rootNode : rootNodes) {
        rootNode = new Node(Node::ROOT_NODE);
        generateNodeTree(rootNode, NR_OF_ROUNDS*4 );
    }

    std::cout << "In total, there are " << nodeCount << " nodes.\n";

    // Save as tree.svg (picture which you can open with your internet browser)
    std::string outputFilename = "tree";
    std::string outputFiletype = "svg"; //you can also change this to png
    outputFilename += "." + outputFiletype;

    // Increasing this number shows more nodes which are on the same layer and makes the tree less readable. it also takes more time to generate tree
    int maxSiblings = 4; 
    // Generate tree figure
    printTreeAsGraph(rootNodes, outputFilename, outputFiletype, maxSiblings); 

    // Delete tree
    for (auto& rootNode : rootNodes) {
        delete rootNode;
    }

    return 0;
}