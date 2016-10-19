// Homepage: http://weibo.com/stormsnow
// Copyright (c) 2016, gangyewei <gangyewei@aliyun.com>.

// Refer to http://www.importnew.com/21818.html
#include "red_black_tree.h"

NodeValue*
RedBlackTree::get(const NodeKey *key) const
{
    // Binary search is OK
    assert(key);
    Node *root = getRoot();

    if (root == NULL) {
        return NULL;
    }

    return binarySearch(root, key);
}

NodeValue*
RedBlackTree::get(const Node *key) const
{
    assert(key->key);
    return get(key->key);
}

enum rbtree_return_value
RedBlackTree::put(const Node *n)
{
    assert(n);
    // first search if it's exist
    if (get(n->key)) {
        return RBTREE_RETURN_VALUE_NODE_EXIST;
    }

    // then put it
    Node *child = root;
    Node *parent = root;
    bool is_left = false;

    while(1) {
        if (!child) {
            if (is_left) {
                parent->left = n;
            } else {
                parent->right = n;
            }
            break;
        }

        if (compareTo(parent->key, *key)) {
            child = parent->left;
            is_left = true;
        } else {
            child = parent->right;
            is_left = false;
        }
    }

    // then rotate
    fixAfterInsertion(n);
    ++size;
    return RBTREE_RETURN_VALUE_OK;
}

enum rbtree_return_value
RedBlackTree::remove(const Node *n)
{
    assert(n);
    assert(n->key);

    if (!get(n->key)) {
        return RBTREE_RETURN_VALUE_NODE_EXIST;
    }

    if (n->left != NULL && n->right != NULL) {
        Node *s = successor(key);
    } else if (n->left != NULL || n->right != NULL) {
        Node *s = (n->left != NULL)?n->left:n->right;
        if (n->parent == NULL) {
            root = s;
        } else if (n->parent->left == n) {
            n->parent->left = s;
        } else {
            n->parent->right = s;
        }
        if (s->color == RBTREE_COLOR_BLACK) {
            fixAfterDeletion(s);
        }
    } else {
        if (n->parent == NULL) {
            root = NULL;
        } else {
            if (n->parent->left == n) {
                n->parent->left = NULL;
            } else if (n->parent->right == n) {
                n->parent->right = NULL;
            } else {
                rbtree_not_reached();
            }
            if (n->parent->color == RBTREE_COLOR_BLACK) {
                fixAfterDeletion(n->parent);
            }
        }
    }

    return RBTREE_RETURN_VALUE_OK;
}

inline static enum rbtree_color
colorOf(Node *n)
{
    if (n == NULL) {
        return RBTREE_COLOR_BLACK;
    } else {
        return n->color;
    }
}

void
RedBlackTree::fixAfterInsertion(Node *n)
{
    assert(n);
    // color of new insertion node should be RED, refer to rule 4
    n->color = RBTREE_COLOR_RED;

    while (n && n->parent && n->parent != RBTREE_COLOR_RED) {
        if (n->parent == n->parent->parent->left) {
            Node *rppnode = n->parent->parent->right;
            if (rppnode && rppnode->color == RBTREE_COLOR_RED) {
                n->parent->color = RBTREE_COLOR_BLACK;
                rppnode->color = RBTREE_COLOR_BLACK;
                n->parent->parent = RBTREE_COLOR_RED;
                n = n->parent->parent;
            } else {
                if (n == n->parent->right) {
                    n - n->parent;
                    leftRotate(n);
                }
                n->parent->color = RBTREE_COLOR_BLACK;
                n->parent->parent->color = RBTREE_COLOR_BLACK;
                rightRotate(n->parent->parent);
            }
        } else {
            Node *lppnode = n->parent->parent->left;
            if (lppnode->color == RBTREE_COLOR_RED) {
                n->parent->color = RBTREE_COLOR_BLACK;
                lppnode->color = RBTREE_COLOR_BLACK;
                n->parent->parent->color = RBTREE_COLOR_RED;
                n = n->parent->parent;
            } else {
                if (n == n->parent->left) {
                    n = n->parent;
                    rightRotate(n);
                }
                n->parent->color = RBTREE_COLOR_BLACK;
                n->parent->parent->color = RBTREE_COLOR_BLACK;
                leftRotate(n->parent->parent);
            }
        }
    }
}

void
RedBlackTree::fixAfterDeletion(Node *n)
{
    assert(n);
    while (n != root && colorOf(n)) {
        if (n == n->parent->left) {
            Node *rpnode = n->parent->right;
            if (colorOf(rpnode) == RBTREE_COLOR_RED) {
                rpnode->color = RBTREE_COLOR_BLACK;
                n->parent->color = RBTREE_COLOR_RED;
                leftRotate(n->parent);
                rpnode = n->parent->right;
            }
            if (colorOf(rpnode->left) == RBTREE_COLOR_BLACK &&
                colorOf(rpnode->right) == RBTREE_COLOR_BLACK) {
                rpnode->color = RBTREE_COLOR_RED;
                n = n->parent;
            } else {
                if (colorOf(rpnode->right) == RBTREE_COLOR_BLACK) {
                    rpnode->left->color = RBTREE_COLOR_BLACK;
                    rpnode->color = RBTREE_COLOR_RED;
                    rightRotate(rpnode);
                    rpnode = n->parent->right;
                }
                rpnode->color = n->parent->color;
                n->parent->color = RBTREE_COLOR_BLACK;
                rpnode->right->color = RBTREE_COLOR_BLACK;
                leftRotate(n->parent);
                n = root;
            }
        } else {    // Symmetry with the previous situation
            if (n == n->parent->right) {
                Node *lpnode = n->parent->left;
                if (colorOf(lpnode) == RBTREE_COLOR_RED) {
                    lpnode->color = RBTREE_COLOR_BLACK;
                    n->parent->color = RBTREE_COLOR_RED;
                    rightRotate(n->parent);
                    lpnode = n->parent->left;
                }
                if (colorOf(lpnode->right) == RBTREE_COLOR_BLACK) {
                    lpnode->left->color = RBTREE_COLOR_BLACK;
                    lpnode->color = RBTREE_COLOR_RED;
                    n = n->parent;
                } else {
                    if (colorOf(lpnode->left) == RBTREE_COLOR_BLACK) {
                        lpnode->right->color = RBTREE_COLOR_BLACK;
                        lpnode->color = RBTREE_COLOR_RED;
                        leftRotate(lpnode);
                        lpnode = lpnode->parent->left;
                    }
                    lpnode->color = colorOf(n->parent);
                    n->parent->color = RBTREE_COLOR_BLACK;
                    lpnode->left->color = RBTREE_COLOR_BLACK;
                    rightRotate(n->parent);
                    n = root;
                }
            }
        }
    }
}

Node*
RedBlackTree::successor(Node *n) const
{
    assert(n);

    if (n->right) {
        Node *successor = n->right;
        while (successor->left != NULL) {
            successor = successor->left;
        }
        return successor;
    } else {
        Node *successor = n->parent;
        Node *child = n;
        while (successor != NULL && child == successor->right) {
            child = successor;
            successor = successor->parent;
        }
    }
}

Node*
RedBlackTree::binarySearch(const Node *root,
    const NodeKey *key) const
{
    assert(key);
    assert(root);

    Node *temp = root;
    Node *res = NULL;

    while(1) {
        if (euqalTo(temp->key, key)) {
            res = temp;
            break;
        } else if (compareTo(temp->key, key)) {
            temp = temp->left;
        } else {
            temp = temp->right;
        }
    }

    return res;
}

void
RedBlackTree::leftRotate(Node *n)
{
    assert(n);

    // there are three link we have to change:
    // n <--> n's parent
    // n <--> n's right
    // n's right <--> n's right's left
    Node *rnode = n->right;

    n->right = rnode->left;
    if (rnode->left) {
        rnode->left->parent = n;
    }

    rnode->parent = n->parent;
    if (p->parent == NULL) {
        root = rnode;
    } else if (p->parent->left == n) {
        p->parent->left = rnode;
    } else {
        p->parent->right = rnode;
    }

    rnode->left = n;

    p->parent = rnode;
}

void
RedBlackTree::rightRotate(const Node *n)
{
    assert(n);

    Node *lnode = n->left;

    n->left = lnode->right;
    if (lnode->right) {
        lnode->left->parent = n;
    }

    lnode->parent = n->parent;
    if (p->parent == NULL) {
        root = lnode;
    } else if (p->parent->left == n) {
        p->parent->left = lnode;
    } else {
        p->parent->right = lnode;
    }

    lnode->right = n;

    n->parent = lnode;
}