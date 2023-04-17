#include <iostream>
#include <vector>
#include <algorithm>
#include <graphviz/gvc.h>
#include <graphviz/cgraph.h>

int nodeCount {};

//#define NAIVE 1            // Upper bound for amount of nodes
#define DICE_ROLLS_EQUAL 1 // OPTIMIZATION 1: Rolling 1 and 2 is the same as rolling 2 and 1
#define REROLL_SAME 1      // OPTIMIZATION 2: Rerolling dice 1 and dice 2 is the same if in the first throw the dice were the same.

class Node {
public:
    enum NodeType {
        ROOT_NODE,
        DICE_NODE,
        REROLL_CHOICE_NODE,
        DICE_REROLL_OPTIONS_NODE
    };

    // Not yet implemented
    double maxExpectedScore;

    // The two possible dice rolls. Make it public to not have to deal with getters and setters
    // Two ints instead of array so I don't have to deal with array pointers and stuff
    int diceOne = {-1};
    int diceTwo = {-1};

    // Constructor
    Node(NodeType type, Node* parent = nullptr) : type(type), parent(parent), maxExpectedScore(-1.0) {}

    // Destructor
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

    // Made public so that I can access it more easily lol (don't try this at home)
    std::vector<Node*> children;
private:
    NodeType type;
    Node* parent;
    //std::vector<Node*> children;
};
