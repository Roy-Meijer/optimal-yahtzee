#include <iostream>
#include <vector>
#include <algorithm>
//--------------------------
#include <graphviz/gvc.h>
#include <graphviz/cgraph.h>
//--------------------------
#include "Node.h"

#define NR_OF_ROUNDS 2

int nodeCount = 0;
void AssignOutcomesForRerolls(Node* node, std::vector<Node*>* outcomeNodes, int& nodeCount); 
void AssignRerollDecisions(Node* node, int childIndex);
void AssignDiceValues(Node* node, int childIndex);
void generateChildrenCount(Node* node, int depth);

void generateNodeTree(Node* node, int depth) {
    // Break if we have reached the necessary depth
    if (depth == 0) {
        return;
    }

    if (depth == 5)
    {
        if (node->getChildren() != nullptr) {
            std::cout << "Children: ";
            for (Node* child: *(node->getChildren()))
            {
                // Recursively make children
                std::cout << (child) << " ";
            }
            std::cout << "\n";
        }
    }
        

    generateChildrenCount(node, depth);

    // Generating the children
    for (int i = 0; i < node->getChildrenCount(); ++i) {

        // Reroll choice nodes have outcome nodes as children, outcome nodes are pre-generated
        if (node->getType() == Node::REROLL_CHOICE_NODE)
        {   
            Node* rootNode = node->getParent()->front()->getParent()->front();
            std::vector<Node*>* outcomeNodes = rootNode->generateOutcomeNodes(rootNode, nodeCount);
            //std::cout << "rootNode: " << rootNode << "\n";
            
            AssignOutcomesForRerolls(node, outcomeNodes, nodeCount);

            if (node->getChildren() != nullptr)
            {
                for (Node* outcome: *(node->getChildren()))
                {
                    // Recursively make children
                    if (!(outcome->childHandled)) {
                        outcome->childHandled = true;
                        generateNodeTree(outcome, depth - 1);
                    }
                }
            }
            
        }
        
        else
        {
            Node* child = new Node(node->getChildType());
            node->addChild(child, nodeCount);

            if(node->getType() == Node::NodeType::ROOT_NODE || node->getType() == Node::NodeType::SCORE_ROOT_NODE)
            {
                std::vector<Node*>* outcomeNodes = node->generateOutcomeNodes(node, nodeCount);
                std::cout << "First outcomeNode: " << outcomeNodes->front()->firstDice  << ", " << outcomeNodes->front()->secondDice << "\n";
                
                AssignDiceValues(node, i);
            }
                
            
            //Assigning Reroll decisions for the Reroll nodes (i.e., Children of Dice Nodes)
            if(node->getType() == Node::NodeType::DICE_NODE)
                AssignRerollDecisions(node, i);

            // Recursively make children
            generateNodeTree(child, depth - 1);
        }
        
    }
}

void printGraphvizNodeTree(Node* node, Agraph_t* graph, Agnode_t* parentAgNode, int depth, int siblingIndex, int maxSiblings) 
{
    static const std::string TYPE_STRINGS[] = {
        "root banana",
        "dice banana",
        "reroll banana",
        "outcome banana",
        "ScoreRoot banana",
        "nodeerror banana"
    };
    static const std::string COLORS[] = {
        "red",
        "green",
        "blue",
        "purple",
        "brown",
        "orange"
    };

    static const std::string TYPE_REROLL[] = {
        "reroll none",
        "reroll single one",
        "reroll single two",
        "reroll both",
        "reroll N/A"
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
    agsafeset(currentNode, const_cast<char*>("coloruiyj"), const_cast<char*>(COLORS[node->getType()].c_str()), const_cast<char*>(""));

    if (parentAgNode) {
        agedge(graph, parentAgNode, currentNode, nullptr, 1);
    }

    if (node->getChildren() != nullptr) 
    {
        int childIndex = 0;
        std::vector<Node*>::iterator iterChildren = node->getChildren()->begin();
        for (; iterChildren != node->getChildren()->end(); ++iterChildren) 
        {
            // This is the recursive part
            printGraphvizNodeTree(*iterChildren, graph, currentNode, depth + 1, childIndex, maxSiblings);
            ++childIndex;
        }
    }
}

void printTreeAsGraph(const std::vector<Node*>& rootNodes, const std::string& outputFilename, const std::string& outputFiletype, int maxSiblings = 1) 
{
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

    agattr(graph, AGRAPH, (char *)"rankdir", (char *)"LR");  // print from left to right
    agattr(graph, AGRAPH, (char *)"nodesep", (char *)"1.5"); // increase the nodesep attribute
    gvLayout(gvc, graph, "dot"); // Use "dot" or "neato", more options: https://graphviz.org/docs/layouts/

    // Save to 'graphs' folder:
    std::string outputFolderName = "graphs/";
    std::string outputFilenameIncludingFolder = outputFolderName + outputFilename;
    gvRenderFilename(gvc, graph, const_cast<char*>(outputFiletype.c_str()), const_cast<char*>(outputFilenameIncludingFolder.c_str()));
    gvFreeLayout(gvc, graph);

    agclose(graph);
    gvFreeContext(gvc);
}

void generateChildrenCount(Node* node, int depth)
{
    if (node->getType() == Node::ROOT_NODE || node->getType() == Node::SCORE_ROOT_NODE) {
        // There are three children. Throwing 1 and 1, (throwing 1 and 2 AND throwing 2 and 1), and finally throwing 2 and 2
        node->setChildrenCount(3);

    } else if (node->getType() == Node::DICE_NODE) {
        // There are four children. Don't reroll, reroll dice with value 1, reroll dice with value 2, or rerolling both dice.
        node->setChildrenCount(4);

        // When first roll is 1,1 or 2,2 then rerolling dice 1 and dice 2 is the same. Gives three children!
        if(node->firstDice == node->secondDice)
            node->setChildrenCount(3);

    } else if (node->getType() == Node::REROLL_CHOICE_NODE) {
        //childType = Node::OUTCOME_NODE;
        // Scoring options node
       
        
        // No rerolls: Only one child (the same outcome of the parent. i.e., dice values do not change)
        // reroll dice one/reroll dice two: two children each (rerolled die either one or two, not rerolled die stays the same)
        // reroll both: has three children (1,1), (1,2)/(2,1), (2,2)
        if (node->getRerollDecision() == Node::REROLL_TYPE::NO_REROLLS)
            node->setChildrenCount(1);
        if ((node->getRerollDecision() == Node::REROLL_TYPE::REROLL_SINGLE_DICE_ONE) || (node->getRerollDecision() == Node::REROLL_TYPE::REROLL_SINGLE_DICE_TWO)) 
            node->setChildrenCount(2);
        if (node->getRerollDecision() == Node::REROLL_TYPE::REROLL_BOTH) 
            node->setChildrenCount(3);

    } else if (node->getType() == Node::OUTCOME_NODE) {
        if (depth > 1) {
            // There are two ways to score in the first round
            node->setChildrenCount(2);
        } else {
            // This is for the final nodes, now there is only one remaining option
            node->setChildrenCount(1);
        }
    } else {
        std::cerr << "Error: This statement should not occur!\n";
        return;
    }
}

void AssignDiceValues(Node* node, int childIndex)
{
    // Assigning value to DICE_NODES (1,1), (1,2), (2,2)
    assert(node->getType() == Node::NodeType::ROOT_NODE || node->getType() == Node::NodeType::SCORE_ROOT_NODE);
    
    std::vector<Node*>* children = node->getChildren();
    if(childIndex == 0)
    {
        // Since generateNodeTree is a 'friend' function to node, it can access the private 'children' members.
        children->at(childIndex)->firstDice = 1;
        children->at(childIndex)->secondDice = 1;
    }
    // This is the dual case (so right now this only works correctly when the NAIVE optimization is NOT ON, but the 1,2=2,1 optimization is ON)
    else if (childIndex == 1) 
    {
        children->at(childIndex)->firstDice = 1;
        children->at(childIndex)->secondDice = 2;
    }
    else if(childIndex == 2)
    {
        children->at(childIndex)->firstDice = 2;
        children->at(childIndex)->secondDice = 2;
    } else {
        std::cerr << "Assign Dice Values: Child index invalid: " << childIndex << "\n";
    }
}

void AssignRerollDecisions(Node* node, int childIndex)
{
    assert(node->getType() == Node::NodeType::DICE_NODE);
    // Assigning the reroll decision for the children of dice nodes
    std::vector<Node*>* children = node->getChildren();
    if (childIndex == 0)    
    {
        node->setRerollDecision(children->at(childIndex), Node::REROLL_TYPE::NO_REROLLS);
    }
    // if there are 3 child nodes, the second will be reroll one dice
    // if there are 4 child nodes, the second and the third will be roll one dice
    else if ((childIndex == 1 && node->getChildrenCount() == 4) || (children->at(childIndex)->firstDice == children->at(childIndex)->secondDice && children->at(childIndex)->firstDice == 1))    // i.e., the children dice values are (1,2), or second branch at 1,1
    {
        node->setRerollDecision(children->at(childIndex), Node::REROLL_TYPE::REROLL_SINGLE_DICE_ONE);
    }
    else if (childIndex == 2 && node->getChildrenCount() == 4 || (children->at(childIndex)->firstDice == children->at(childIndex)->secondDice && children->at(childIndex)->firstDice == 2))    // i.e., the children dice values are (2,1), or second branch at 2,2
    {
        node->setRerollDecision(children->at(childIndex), Node::REROLL_TYPE::REROLL_SINGLE_DICE_TWO);
    }
    else if ( (childIndex == 2 && node->getChildrenCount() == 3) || childIndex == 3)    
    {
        node->setRerollDecision(children->at(childIndex), Node::REROLL_TYPE::REROLL_BOTH);
    } else {
        std::cerr << "Assign Reroll decisions: Child index invalid: " << childIndex << ", Children Count "  << node->getChildrenCount() << "\n";
        std::cerr << "firstDice value: " << children->at(childIndex)->firstDice << "secondDice value: " << children->at(childIndex)->secondDice << "\n";
    }
}

void AssignOutcomesForRerolls(Node* node, std::vector<Node*>* outcomeNodes, int& nodeCount)
{
    Node* outcome_11 = outcomeNodes->at(0);
    Node* outcome_12 = outcomeNodes->at(1);
    Node* outcome_22 = outcomeNodes->at(2);
    //TODO: Assign each of the reroll nodes to the correct outcome node
    // Matching to 11
    // 1) dice is 11, no rerolls
    // 2) dice is 11, one reroll (either reroll one or two)
    // 3) dice is 21, reroll second
    // 4) dice is 12, reroll one
    // 3) All of the both rerolls (11, 12, 22)
    if (node->firstDice == 1 && node->secondDice == 1)
    {
        node->addChild(outcome_11, nodeCount);
        if (node->getRerollDecision() == Node::REROLL_TYPE::REROLL_SINGLE_DICE_ONE)
            node->addChild(outcome_12, nodeCount);
    }
        
    else if (node->firstDice == 1 && node->secondDice == 2)
    {
         node->addChild(outcome_12, nodeCount);
        if (node->getRerollDecision() == Node::REROLL_TYPE::REROLL_SINGLE_DICE_ONE)
            node->addChild(outcome_22, nodeCount);
        if (node->getRerollDecision() == Node::REROLL_TYPE::REROLL_SINGLE_DICE_TWO)
            node->addChild(outcome_11, nodeCount);
    }
    else if (node->firstDice == 2 && node->secondDice == 2)
    {
        node->addChild(outcome_22, nodeCount);
        if (node->getRerollDecision() == Node::REROLL_TYPE::REROLL_SINGLE_DICE_TWO)
             node->addChild(outcome_12, nodeCount);
    }
    
    //if reroll decision is to reroll both, all three outcomes are available
    if (node->getRerollDecision() == Node::REROLL_TYPE::REROLL_BOTH)
    {
        node->addChild(outcome_11, nodeCount);
        node->addChild(outcome_12, nodeCount);
        node->addChild(outcome_22, nodeCount);
    }
}

//not used

/*
void printNodeTree(Node* node, int depth = 0) {
    static const std::string INDENT_STRING = "  ";
    static const std::string TYPE_STRINGS[] = {
        "roll",
        "dice",
        "reroll",
        "redice"
    };

    std::cout << std::string(depth * INDENT_STRING.size(), ' ')
              << std::string(TYPE_STRINGS[node->getType().c_str()]) << " (" << depth << ")" << std::endl;

    for (auto& child : node->getChildren()) {
        printNodeTree(child, depth + 1);
    }
}
*/


int main() {
    srand(static_cast<unsigned>(time(NULL)));

    std::cout << "Using all optimizations!\n";

    // Make the root node (start of round 1)
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
    int maxSiblings = 100; 
    // Generate tree figure
    printTreeAsGraph(rootNodes, outputFilename, outputFiletype, maxSiblings); 

    // Delete tree
    for (auto& rootNode : rootNodes) {
        delete rootNode;
    }

    return 0;
}