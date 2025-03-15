#include <iostream>
#include <vector>

using namespace std;

struct TreeNode {
    int value;
    vector<TreeNode*> children;

    TreeNode(int val) : value(val) {}
};

void printPaths(TreeNode* root, vector<int>& path) {
    if (root == nullptr) {
        return;
    }

    // Add current node to the path
    path.push_back(root->value);

    // If it's a leaf node, print the path
    if (root->children.empty()) {
        for (int i = 0; i < path.size() - 1; ++i) {
            cout << path[i] << " -> ";
        }
        cout << path.back() << endl;
    }

    // Recursively traverse each child
    for (TreeNode* child : root->children) {
        printPaths(child, path);
    }

    // Backtrack: remove the current node from the path
    path.pop_back();
}

int main() {
    // Create a sample general tree
    //       1
    //      /|\
    //     2 3 4
    //    /|   \
    //   5 6    7
    TreeNode* root = new TreeNode(1);
    root->children.push_back(new TreeNode(2));
    root->children.push_back(new TreeNode(3));
    root->children.push_back(new TreeNode(4));

    root->children[0]->children.push_back(new TreeNode(5));
    root->children[0]->children.push_back(new TreeNode(6));

    root->children[2]->children.push_back(new TreeNode(7));

    vector<int> path;

    // Call the function to print all paths
    printPaths(root, path);

    // TODO: Don't forget to free the allocated memory for the tree nodes in a complete program.

    return 0;
}

