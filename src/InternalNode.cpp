#include "InternalNode.hpp"
#include "LeafNode.hpp"
#include "RecordPtr.hpp"

//creates internal node pointed to by tree_ptr or creates a new one
InternalNode::InternalNode(const TreePtr &tree_ptr) : TreeNode(INTERNAL, tree_ptr) {
    this->keys.clear();
    this->tree_pointers.clear();
    if (!is_null(tree_ptr))
        this->load();
}

//max element from tree rooted at this node
Key InternalNode::max() {
    Key max_key = DELETE_MARKER;
    TreeNode* last_tree_node = TreeNode::tree_node_factory(this->tree_pointers[this->size - 1]);
    max_key = last_tree_node->max();
    delete last_tree_node;
    return max_key;
}

//if internal node contains a single child, it is returned
TreePtr InternalNode::single_child_ptr() {
    if (this->size == 1)
        return this->tree_pointers[0];
    return NULL_PTR;
}

Key borrow; // used for propogating the value from down to up.

//inserts <key, record_ptr> into subtree rooted at this node.
//returns pointer to split node if exists
//TODO: InternalNode::insert_key to be implemented
TreePtr InternalNode::insert_key(const Key &key, const RecordPtr &record_ptr) {
    TreePtr new_tree_ptr = NULL_PTR;
    // cout << "InternalNode::insert_key not implemented" << endl;
    this->load();

    vector<Key>tempv = this->keys;
    vector<Key>v;
    for(int i=0;i<tempv.size();i++)
    {
        if(tempv[i]>=0) v.push_back(tempv[i]);
        else break;
    }
    vector<TreePtr>tempp = this->tree_pointers;
    vector<TreePtr>p;
    for(int i=0;i<tempp.size();i++)
    {
        if(tempp[i]!=NULL_PTR) p.push_back(tempp[i]);
        else break;
    }

    int flag=0;
    TreePtr left = NULL_PTR;
    TreePtr right = NULL_PTR;
    for(int i=0;i<v.size();i++)
    {
        if(key <= v[i])
        {
            // then send p[i] to this recursive function
            left = p[i];
            TreeNode *child = TreeNode::tree_node_factory(p[i]);
            new_tree_ptr =  child->insert_key(key, record_ptr);
            if(child->node_type==LEAF)
            {
                TreeNode *left_child = TreeNode::tree_node_factory(left);
                borrow = left_child->max();
                delete left_child;
            }
            delete child;
            flag=1;
            break;
        }
    }
    if(flag==0)
    {
        // then send p[v.size()-1] to this recursive function.
        left = p[v.size()];
        TreeNode *child = TreeNode::tree_node_factory(left);
        new_tree_ptr = child->insert_key(key, record_ptr);
        if(child->node_type==LEAF)
        {
            TreeNode *left_child = TreeNode::tree_node_factory(left);
            borrow = left_child->max();
            delete left_child;
        }
        delete child;
    }
    right = new_tree_ptr;
    // you come here and check if the internal node should suffer a split or not
    if(!is_null(right))
    {
        // we come here when a child of this treenode has been split, so we have to make place for the new childs
        // we need to change this internal node. If after insertion of the new value, if there is no overflow, then return NULL. 
        // But if there is a overflow, then create a new internal node and put some values in it and return the pointer to that node. 
        // Also, dont forget to dump these newly created files into the disk.

        // we need to insert this left_max into this internal node and then dump it into the disk.
        vector<Key>updated_keys;
        vector<TreePtr>updated_treeptrs;
        int ind=v.size();
        for(int i=0;i<v.size();i++)
        {
            if(v[i]<borrow)
            {
                updated_keys.push_back(v[i]);
                updated_treeptrs.push_back(p[i]);
            }
            else
            {
                updated_keys.push_back(borrow);
                updated_treeptrs.push_back(left);
                updated_treeptrs.push_back(right);
                ind = i;
                break;
            }
        }
        for(int i=ind;i<v.size();i++)
        {
            updated_keys.push_back(v[i]);
            updated_treeptrs.push_back(p[i+1]);
        }
        if(ind==v.size())
        {
            updated_keys.push_back(borrow);
            updated_treeptrs.push_back(left);
            updated_treeptrs.push_back(right);
        }

        if(updated_treeptrs.size()<=FANOUT)
        {
            // newly inserted element can be put inside this internal node
            this->size = updated_treeptrs.size();
            this->keys = updated_keys;
            this->tree_pointers = updated_treeptrs;
            this->dump();
            borrow = -1;
            // cout<<"Insertion block size: "<<this->tree_pointers.size();
            return NULL_PTR;
        }
        else
        {
            // overflow happens
            // keep MIN_OCCUPANCY no of blocks in the present treenode and remaining in the newly created internal node
            vector<Key>left_keys;
            vector<TreePtr>left_treeptrs;
            vector<Key>right_keys;
            vector<TreePtr>right_treeptrs;
            ind = (updated_treeptrs.size()-2)/2;
            borrow = updated_keys[ind];
            for(int i=0;i<ind;i++)
            {
                left_keys.push_back(updated_keys[i]);
                left_treeptrs.push_back(updated_treeptrs[i]);
            }
            left_treeptrs.push_back(updated_treeptrs[ind]);
            for(int i=ind+1;i<updated_keys.size();i++)
            {
                right_keys.push_back(updated_keys[i]);
                right_treeptrs.push_back(updated_treeptrs[i]);
            }
            right_treeptrs.push_back(updated_treeptrs[updated_treeptrs.size()-1]);
            this->size = left_treeptrs.size();
            InternalNode *sibling = new InternalNode();
            sibling->size = right_treeptrs.size();
            this->keys = left_keys;
            this->tree_pointers = left_treeptrs;
            sibling->keys = right_keys;
            sibling->tree_pointers = right_treeptrs;
            this->dump();
            sibling->dump();
            return sibling->tree_ptr;
        }
    }
    else 
    {
        borrow = -1;
        // cout<<"Insertion block size: "<<this->tree_pointers.size();
        return NULL_PTR;
    }
    return NULL_PTR;
}

//deletes key from subtree rooted at this if exists
//TODO: InternalNode::delete_key to be implemented
void InternalNode::delete_key(const Key &key) {
    
    TreePtr new_tree_ptr = NULL_PTR;
    this->load();
    vector<Key>v = this->keys;
    vector<TreePtr>p = this->tree_pointers;
    // now that we have both v (values) and p(pointers) vectors, we are ready to roar
    int ind;
    int flag=0;
    TreePtr left = NULL_PTR;
    TreePtr right = NULL_PTR;
    TreePtr cur;
    for(int i=0;i<(this->size)-1;i++)
    {
        if(key <= v[i])
        {
            // then send p[i] to this recursive function
            (i-1>=0? left = p[i-1]: NULL_PTR);
            cur = p[i];
            (i+1<(this->size)? right = p[i+1]: NULL_PTR);
            ind = i;
            flag=1;
            break;
        }
    }
    if(flag==0)
    {
        ((this->size)-2>=0? left = p[this->size-2]: NULL_PTR);
        cur = p[(this->size)-1];
        right = NULL_PTR;
        ind = (this->size)-1;
    }
    cout<<"ind "<<ind<<endl;
    cout<<"left: "<<left<<endl;
    cout<<"right: "<<right<<endl;
    TreeNode* child = TreeNode::tree_node_factory(cur);
    child->delete_key(key);
    delete child;
    cout<<"current treenode pointer: "<<this->tree_ptr<<endl;
    child = TreeNode::tree_node_factory(cur);
    if((child->size)<MIN_OCCUPANCY)
    {
        cout<<"child-size is :"<<child->size<<endl;
        // first try to do redistribution with left child
        TreeNode* left_child;
        TreeNode* right_child;
        TreeNode* cur_child;
        if(!is_null(left)) left_child = TreeNode::tree_node_factory(left);
        if(!is_null(right)) right_child = TreeNode::tree_node_factory(right);
        cur_child = TreeNode::tree_node_factory(cur);

        if(!is_null(left) && left_child->size>MIN_OCCUPANCY)
        {
            // we can bring one value from left node to the current node
            // this is called redistribution. 
            if(cur_child->node_type==INTERNAL)
            {
                cout<<"1-1"<<endl;
                InternalNode *left_node = new InternalNode(left);
                InternalNode *cur_node = new InternalNode(cur);
                left_node->load();
                cur_node->load();
                Key left_to_up = left_node->keys[left_node->size-2];
                Key up_to_cur = this->keys[ind-1];
                TreePtr left_to_cur =  left_node->tree_pointers[(left_node->size)-1];

                cout<<"a"<<endl;
                // dumping left child
                // left_node->keys[left_node->size-2] = DELETE_MARKER;
                // left_node->tree_pointers[(left_node->size)-1] = NULL_PTR;
                left_node->size--;
                left_node->dump();

                cout<<"b"<<endl;
                // dumping "this" node
                this->keys[ind-1] = left_to_up;
                this->dump();

                cout<<"c"<<endl;
                // dumping cur child
                vector<Key>temp_keys;
                vector<TreePtr>temp_ptrs;
                temp_keys.push_back(up_to_cur);
                temp_ptrs.push_back(left_to_cur);
                for(int i=0;i<(cur_node->size)-1;i++)
                {
                    temp_keys.push_back(cur_node->keys[i]);
                }
                for(int i=0;i<(cur_node->size);i++)
                {
                    temp_ptrs.push_back(cur_node->tree_pointers[i]);
                }
                cout<<"d"<<endl;
                cur_node->keys = temp_keys;
                cur_node->tree_pointers = temp_ptrs;
                cur_node->size++;
                cur_node->dump();
            }
            else
            {
                cout<<"1-2"<<endl;
                LeafNode *left_node = new LeafNode(left);
                LeafNode *cur_node = new LeafNode(cur);
                left_node->load();
                cur_node->load();

                auto itr = left_node->data_pointers.rbegin();
                Key left_to_cur = left_node->data_pointers.rbegin()->first;
                RecordPtr record_ptr = left_node->data_pointers.rbegin()->second;
                // dumping left node
                left_node->data_pointers.erase(left_to_cur);
                left_node->size--;
                left_node->dump();

                // dumping cur node
                cur_node->data_pointers[left_to_cur] = record_ptr;
                cur_node->size++;
                cur_node->dump();

                // dumping "this" node
                Key left_to_up = left_node->data_pointers.rbegin()->first;
                this->keys[ind-1] = left_to_up;
                this->dump();
            }
        }
        else if(!is_null(right) && right_child->size>MIN_OCCUPANCY)
        {
            // Almost same as left
            // we can bring one value from right node to the current node
            if(cur_child->node_type==INTERNAL)
            {
                cout<<"2-1"<<endl;
                InternalNode *right_node = new InternalNode(right);
                InternalNode *cur_node = new InternalNode(cur);
                right_node->load();
                cur_node->load();
                Key right_to_up = right_node->keys[0];
                Key up_to_cur = this->keys[ind];
                TreePtr right_to_cur =  right_node->tree_pointers[0];

                cout<<"a"<<endl;
                // dumping right child
                for(int i=0;i<right_node->size-2;i++)
                {
                    right_node->keys[i] = right_node->keys[i + 1];
                }
                for(int i=0;i<right_node->size-1;i++)
                {
                    right_node->tree_pointers[i] = right_node->tree_pointers[i + 1];
                }
                right_node->size--;
                right_node->dump();

                cout<<"b"<<endl;
                // dumping "this" node
                this->keys[ind] = right_to_up;
                this->dump();

                cout<<"c"<<endl;
                // dumping cur child
                cur_node->keys.push_back(up_to_cur);
                // cur_node->keys[cur_node->size - 1] = up_to_cur;
                cur_node->tree_pointers.push_back(right_to_cur);
                // cur_node->tree_pointers[cur_node->size] = right_to_cur;
                cur_node->size++;
                cout<<"d"<<endl;
                cur_node->dump();
            }
            else
            {
                cout<<"2-2"<<endl;
                LeafNode *right_node = new LeafNode(right);
                LeafNode *cur_node = new LeafNode(cur);
                right_node->load();
                cur_node->load();

                auto itr = right_node->data_pointers.begin();
                Key right_to_cur = right_node->data_pointers.begin()->first;
                RecordPtr record_ptr = right_node->data_pointers.begin()->second;

                // dumping right node
                right_node->data_pointers.erase(right_to_cur);
                right_node->size--;
                right_node->dump();

                // dumping cur node
                cur_node->data_pointers[right_to_cur] = record_ptr;
                cur_node->size++;
                cur_node->dump();
                // (cur_node->data_pointers).insert({right_to_cur, record_ptr}); 

                // dumping "this" node
                Key right_to_up = right_to_cur;
                this->keys[ind] = right_to_up;
                this->dump();
            }
        }
        else if(!is_null(left)) // Redistribution is not possible, so doing merge
        {
            // merge with left
            if(cur_child->node_type == INTERNAL)
            {
                cout<<"3-1"<<endl;
                InternalNode *left_node = new InternalNode(left);
                InternalNode *cur_node = new InternalNode(cur);
                left_node->load();
                cur_node->load();

                Key up_to_left = this->keys[ind - 1];

                // dumping left node
                int left_itr = left_node->size - 1;
                left_node->keys.push_back(up_to_left);
                // left_node->keys[left_itr] = up_to_left;
                left_itr++;
                for(int i=0;i<cur_node->size-1;i++)
                {
                    left_node->keys.push_back(cur_node->keys[i]);
                    // left_node->keys[left_itr] = cur_node->keys[i];
                    left_itr++;
                }
                left_itr = left_node->size;
                for(int i=0;i<cur_node->size;i++)
                {
                    left_node->tree_pointers.push_back(cur_node->tree_pointers[i]);
                    // left_node->tree_pointers[left_itr] = cur_node->tree_pointers[i];
                    left_itr++;
                }
                left_node->size += cur_node->size;
                left_node->dump();

                // dumping "this" node
                for(int i=ind-1;i<this->size-2;i++)
                {
                    this->keys[i] = this->keys[i + 1];
                }
                for(int i=ind;i<this->size-1;i++)
                {
                    this->tree_pointers[i] = this->tree_pointers[i + 1];
                }
                this->size--;
                this->dump();

                // deleting cur node
                cur_node->delete_node();
            }
            else
            {
                cout<<"3-2"<<endl;
                LeafNode *left_node = new LeafNode(left);
                LeafNode *cur_node = new LeafNode(cur);
                left_node->load();
                cur_node->load();

                // dumping left node
                for(auto itr=cur_node->data_pointers.begin();itr!=cur_node->data_pointers.end();itr++)
                {
                    left_node->data_pointers.insert(*itr);
                }
                left_node->size += cur_node->size;
                left_node->next_leaf_ptr = cur_node->next_leaf_ptr;
                left_node->dump();
                cout<<"dumped left node\n";
                cout<<"left_node size : "<<left_node->size<<endl;

                // dumping "this" node
                for(int i=ind-1;i<this->size-2;i++)
                {
                    this->keys[i] = this->keys[i + 1];
                }
                cout<<"a\n";
                for(int i=ind;i<this->size-1;i++)
                {
                    this->tree_pointers[i] = this->tree_pointers[i + 1];
                }
                this->size--;
                this->dump();
                cout<<"dumped cur node\n";
                cout<<"cur_node size : "<<cur_node->size<<endl;

                // deleting cur node
                cur_node->delete_node();
            }
        }
        else if(!is_null(right))
        {
            // merge with right
            if(cur_child->node_type == INTERNAL)
            {
                cout<<"4-1"<<endl;
                InternalNode *right_node = new InternalNode(right);
                InternalNode *cur_node = new InternalNode(cur);
                right_node->load();
                cur_node->load();

                Key up_to_cur = this->keys[ind];

                // dumping cur node
                int cur_itr = cur_node->size - 1;
                cur_node->keys.push_back(up_to_cur);
                // cur_node->keys[cur_itr] = up_to_cur;
                cur_itr++;
                for(int i=0;i<right_node->size-1;i++)
                {
                    cur_node->keys.push_back(right_node->keys[i]);
                    // cur_node->keys[cur_itr] = right_node->keys[i];
                    cur_itr++;
                }
                cur_itr = cur_node->size;
                for(int i=0;i<right_node->size;i++)
                {
                    cur_node->tree_pointers.push_back(right_node->tree_pointers[i]);
                    // cur_node->tree_pointers[cur_itr] = right_node->tree_pointers[i];
                    cur_itr++;
                }
                cur_node->size += right_node->size;
                cur_node->dump();

                // dumping "this" node
                for(int i=ind;i<this->size-2;i++)
                {
                    this->keys[i] = this->keys[i + 1];
                }
                for(int i=ind+1;i<this->size-1;i++)
                {
                    this->tree_pointers[i] = this->tree_pointers[i + 1];
                }
                this->size--;
                this->dump();

                // deleting right node
                right_node->delete_node();
            }
            else
            {
                cout<<"4-2"<<endl;
                LeafNode *right_node = new LeafNode(right);
                LeafNode *cur_node = new LeafNode(cur);
                right_node->load();
                cur_node->load();

                // dumping cur node
                for(auto itr=right_node->data_pointers.begin();itr!=right_node->data_pointers.end();itr++)
                {
                    cur_node->data_pointers.insert(*itr);
                }
                cur_node->size += right_node->size;
                cur_node->next_leaf_ptr = right_node->next_leaf_ptr;
                cur_node->dump();

                // dumping "this" node
                for(int i=ind;i<this->size-2;i++)
                {
                    this->keys[i] = this->keys[i + 1];
                }
                for(int i=ind+1;i<this->size-1;i++)
                {
                    this->tree_pointers[i] = this->tree_pointers[i + 1];
                }
                this->size--;
                this->dump();

                // deleting right node
                right_node->delete_node();
            }
        }
    }
    // cout << "InternalNode::delete_key not implemented" << endl;
}

//runs range query on subtree rooted at this node
void InternalNode::range(ostream &os, const Key &min_key, const Key &max_key) const {
    BLOCK_ACCESSES++;
    for (int i = 0; i < this->size - 1; i++) {
        if (min_key <= this->keys[i]) {
            auto* child_node = TreeNode::tree_node_factory(this->tree_pointers[i]);
            child_node->range(os, min_key, max_key);
            delete child_node;
            return;
        }
    }
    auto* child_node = TreeNode::tree_node_factory(this->tree_pointers[this->size - 1]);
    child_node->range(os, min_key, max_key);
    delete child_node;
}

//exports node - used for grading
void InternalNode::export_node(ostream &os) {
    TreeNode::export_node(os);
    for (int i = 0; i < this->size - 1; i++)
        os << this->keys[i] << " ";
    os << endl;
    for (int i = 0; i < this->size; i++) {
        auto child_node = TreeNode::tree_node_factory(this->tree_pointers[i]);
        child_node->export_node(os);
        delete child_node;
    }
}

//writes subtree rooted at this node as a mermaid chart
void InternalNode::chart(ostream &os) {
    string chart_node = this->tree_ptr + "[" + this->tree_ptr + BREAK;
    chart_node += "size: " + to_string(this->size) + BREAK;
    chart_node += "]";
    os << chart_node << endl;

    for (int i = 0; i < this->size; i++) {
        auto tree_node = TreeNode::tree_node_factory(this->tree_pointers[i]);
        tree_node->chart(os);
        delete tree_node;
        string link = this->tree_ptr + "-->|";

        if (i == 0)
            link += "x <= " + to_string(this->keys[i]);
        else if (i == this->size - 1) {
            link += to_string(this->keys[i - 1]) + " < x";
        } else {
            link += to_string(this->keys[i - 1]) + " < x <= " + to_string(this->keys[i]);
        }
        link += "|" + this->tree_pointers[i];
        os << link << endl;
    }
}

ostream& InternalNode::write(ostream &os) const {
    TreeNode::write(os);
    for (int i = 0; i < this->size - 1; i++) {
        if (&os == &cout)
            os << "\nP" << i + 1 << ": ";
        os << this->tree_pointers[i] << " ";
        if (&os == &cout)
            os << "\nK" << i + 1 << ": ";
        os << this->keys[i] << " ";
    }
    if (&os == &cout)
        os << "\nP" << this->size << ": ";
    os << this->tree_pointers[this->size - 1];
    return os;
}

istream& InternalNode::read(istream& is) {
    TreeNode::read(is);
    this->keys.assign(this->size - 1, DELETE_MARKER);
    this->tree_pointers.assign(this->size, NULL_PTR);
    for (int i = 0; i < this->size - 1; i++) {
        if (&is == &cin)
            cout << "P" << i + 1 << ": ";
        is >> this->tree_pointers[i];
        if (&is == &cin)
            cout << "K" << i + 1 << ": ";
        is >> this->keys[i];
    }
    if (&is == &cin)
        cout << "P" << this->size;
    is >> this->tree_pointers[this->size - 1];
    return is;
}
