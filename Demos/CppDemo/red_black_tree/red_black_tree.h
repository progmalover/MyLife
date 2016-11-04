// Not synchronized

enum rbtree_color {
    RBTREE_COLOR_NONE = 0,
    RBTREE_COLOR_BLACK,
    RBTREE_COLOR_RED,
};

enum rbtree_return_value {
    RBTREE_RETURN_VALUE_NONE = 0,
    RBTREE_RETURN_VALUE_OK,
    RBTREE_RETURN_VALUE_NODE_EXIST,
};

class NodeKey {
public:
    void getKey(void) const
    {
        return key;
    }

    void setKey(int key)
    {
        this->key = key;
    }
private:
    int key;
};

class NodeValue {
public:
    void getValue(void) const
    {
        return value;
    }

    void setValue(int value)
    {
        this->value = value;
    }
private:
    int value;
};

class Node {
public:
    NodeKey *key;
    NodeValue *value;
    enum rbtree_color color;
};

class Tree {
public:
    virtual NodeValue* get(const NodeKey *key) const = 0;
    virtual bool put(const Node *n) = 0;
    virtual bool remove(const NodeKey *key) = 0;
};

class RedBlackTree : public Tree {
public:
    RedBlackTree() {
        root = NULL;
    }

    const Node* getRoot() const
    {
        return root;
    }

    int getSize() const
    {
        return size;
    }

    // Function: get node from RBTree
    // @param key: search key of node
    // @return: node of the key, NULL if not found
    virtual NodeValue* get(const NodeKey *key) const;

    // Function: put node into RBTree
    // @param n: node to be added
    // @return: state of insertion
    enum rbtree_return_value put(const Node *n);

    // Function: remove key from RBTree
    // @param key: node to be deleted
    // @return: state of deletion
    enum rbtree_return_value remove(const Node *key, Node *ret);

private:
    int compareTo(const NodeKey *left, const NodeKey *right) const
    {
        return left->getKey() < right->getKey();
    }

    bool equalTo(const NodeKey *left, const NodeKey *right) const
    {
        return left->getKey() == right->getKey();
    }

    void fixAfterInsertion(Node *n);
    Node* successor(Node *n) const;
    Node* binarySearch(const Node *n);
    void leftRotate(const Node *n);
    void rightRotate(const Node *n);

    Node *root;
    int size;
};