#include <iostream>
#include <vector>
#include <algorithm>
#include <graphviz/gvc.h>
#include <graphviz/cgraph.h>

int nodeCount {};

//#define NAIVE 1            // Upper bound for amount of nodes
#define DICE_ROLLS_EQUAL 1 // rolling 1 and 2 is the same as rolling 2 and 1

class Node {
public:
    enum NodeType {
        ROOT_NODE,
        DICE_NODE,
        REROLL_CHOICE_NODE,
        DICE_REROLL_OPTIONS_NODE
    };

    // not yet implemented
    double maxExpectedScore;

    // constructor
    Node(NodeType type, Node* parent = nullptr) : type(type), parent(parent), maxExpectedScore(-1.0) {}

    // destructor
    virtual ~Node() {
        for (auto child : children) {
            delete child;
        }
    }

    NodeType getType() const {
        return type;
    }

    Node* getParent() const {
        return parent;
    }

    void setParent(Node* newParent) {
        parent = newParent;
    }

    void addChild(Node* child) {
        children.push_back(child);
        child->setParent(this);
    }

    void removeChild(Node* child) {
        auto it = std::find_if(children.begin(), children.end(),
            [=](Node* node) { return node == child; });

        if (it != children.end()) {
            children.erase(it);
            child->setParent(nullptr);
        }
    }

    const std::vector<Node*>& getChildren() const {
        return children;
    }

private:
    NodeType type;
    Node* parent;
    std::vector<Node*> children;
};

void generateNodeTree(Node* node, int depth) {
    if (depth == 0) {
        return;
    }

    Node::NodeType childType;
    int numChildren;
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
        // There are (up to) 3 children. If the parent was a reroll both, then we get three children:
        // 1,1 and (1,2 AND 2,1) and 2,2.
        // If the parent only rerolled dice 1, (and dice 2 has value x) we get
        // 1,x and 2,x.
        // vice versa for the other dice
        // If no dice are rerolled in the parent, it depends on how we implement it. I guess 1 child makes most sense
        // x,y where the values x and y are just the same values of the dice that were in the first round (remember, no rerolls here)
        numChildren = 3;
        #endif

    } else if (node->getType() == Node::DICE_REROLL_OPTIONS_NODE) {
        childType = Node::ROOT_NODE;
        if (depth > 1) {
            // there are two ways to score in the first round
            numChildren = 2;
        } else {
            // this is for the final nodes, now there is only one remaining option
            numChildren = 1;
        }
    } else {
        return;
    }

    for (int i = 0; i < numChildren; ++i) {
        Node* child = new Node(childType);
        node->addChild(child);
        // Here we count how many nodes there are
        ++nodeCount;
        generateNodeTree(child, depth - 1); // recursively make children
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

    std::string nodeName = TYPE_STRINGS[node->getType()] + "_" + std::to_string(reinterpret_cast<uintptr_t>(node));
    Agnode_t* currentNode = agnode(graph, const_cast<char*>(nodeName.c_str()), 1);
    agsafeset(currentNode, const_cast<char*>("color"), const_cast<char*>(COLORS[node->getType()].c_str()), const_cast<char*>(""));

    if (parentAgNode) {
        agedge(graph, parentAgNode, currentNode, nullptr, 1);
    }

    int childIndex = 0;
    for (const auto& child : node->getChildren()) {
        printGraphvizNodeTree(child, graph, currentNode, depth + 1, childIndex, maxSiblings); //this is the recursive part
        ++childIndex;
    }
}

void printTreeAsGraph(const std::vector<Node*>& rootNodes, const std::string& outputFilename, const std::string& outputFiletype, int maxSiblings = 1) {
    GVC_t* gvc = gvContext();
    Agraph_t* graph = agopen(const_cast<char*>("NodeTree"), Agdirected, nullptr);

    Agnode_t* sharedParentNode = agnode(graph, const_cast<char*>("start"), 1);
    agset(sharedParentNode, const_cast<char*>("color"), const_cast<char*>("black"));

    int initialSiblingIndex = 0;

    //recursively generate tree visualisation
    for (const auto& node : rootNodes) {
        printGraphvizNodeTree(node, graph, sharedParentNode, 0, initialSiblingIndex, maxSiblings);
        ++initialSiblingIndex;
    }

    agattr(graph, AGRAPH, (char *)"rankdir", (char *)"LR"); //print from left to right
    agattr(graph, AGRAPH, (char *)"nodesep", (char *)"1.5"); // increase the nodesep attribute
    gvLayout(gvc, graph, "dot"); //you can try to replace dot by neato, maybe it looks better (might take longer to generate). there are more options: https://graphviz.org/docs/layouts/
    gvRenderFilename(gvc, graph, const_cast<char*>(outputFiletype.c_str()), const_cast<char*>(outputFilename.c_str()));
    gvFreeLayout(gvc, graph);

    agclose(graph);
    gvFreeContext(gvc);
}

//not used
void printNodeTree(const Node* rootNode, int depth = 0) {
    static const std::string INDENT_STRING = "  ";
    static const std::string TYPE_STRINGS[] = {
        "roll",
        "dice",
        "reroll",
        "redice"
    };

    std::cout << std::string(depth * INDENT_STRING.size(), ' ')
              << TYPE_STRINGS[rootNode->getType()] << " (" << depth << ")" << std::endl;

    for (const auto& child : rootNode->getChildren()) {
        printNodeTree(child, depth + 1);
    }
}

int main() {
    srand(static_cast<unsigned>(time(NULL)));

    #ifdef NAIVE
    std::cout << "Using naive tree generation...\n";
    #endif

    #ifdef DICE_ROLLS_EQUAL
    std::cout << "(1,2) = (2,1)...\n";
    #endif

    //make 1 root node
    std::vector<Node*> rootNodes(1);
    ++nodeCount;

    //generate tree of depth 8
    for (auto& rootNode : rootNodes) {
        rootNode = new Node(Node::ROOT_NODE);
        generateNodeTree(rootNode, 8 );
    }

    std::cout << "In total, there are " << nodeCount << " nodes.\n";

    //save as tree.svg (picture which you can open with your internet browser)
    std::string outputFilename = "tree";
    std::string outputFiletype = "svg"; //you can also change to png
    outputFilename += "." + outputFiletype;

    int maxSiblings = 1; //increasing this number shows more nodes which are on the same layer and makes the tree less readable. it also takes more time to generate tree
    printTreeAsGraph(rootNodes, outputFilename, outputFiletype, maxSiblings); //generate tree figure

    //delete tree
    for (auto& rootNode : rootNodes) {
        delete rootNode;
    }

    return 0;
}