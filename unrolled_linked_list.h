#ifndef UNROLLED_LINKED_LIST
#define UNROLLED_LINKED_LIST

#include <fstream>
#include <vector>

/**
 * @class UnrolledLinkedList
 *
 * This is a template class of double unrolled linked list on disk.
 * <br><br>
 * WARNING: the key type MUST have valid operator< and operator== !
 * @tparam keyType Type of Key
 * @tparam valueType Type of Value
 */
template <class keyType, class valueType>
class UnrolledLinkedList {
private:
    typedef int ptr;

    std::fstream _list;

    /// The following are private components of this linked list

    /**
     * @struct _first_node{next, pre, nodeSize, maxNodeSize}
     *
     * This is the node to pointer to the data, and metadata of the list
     */
    struct _first_node {
        ptr next; // the first main node

        ptr pre; // the last main node

        int nodeSize;

        int maxNodeSize;
    } _head;

    /**
     * @struct _main_node{key, value, target, count, next, pre}
     *
     * This is the node to store the pointer and data, and
     * link other main nodes.
     */
    struct _main_node {
        keyType key;

        valueType value;

        ptr target;

        int count;

        ptr next;

        ptr pre;
    };

    /**
     * @struct _node{key, value}
     *
     * This is the Node to store data.
     */
    struct _node {
        keyType key;

        valueType value;
    } _empty_node;

    /**
     * This function return a pair of the pointer to the main node
     * and the number of node the target is right after, for the case
     * of the main node, the second member of the pair is -1.  If the
     * key doesn't belongs to the unrolled linked list, it will the
     * node ahead of its place (except the case that the key is less than
     * any other keys, and in such case, it will return the first main node).
     * @param key
     * @return a pair of the pointer to the main node and the offset (0-based)
     */
    std::pair<ptr, int> _find(const keyType& key)
    {
        if (_head.next == 0) return std::make_pair(0, -1);
        _main_node tmp;
        ptr Ptr = _head.pre;
        _list.seekg(_head.pre);
        _list.read(reinterpret_cast<char*>(&tmp), sizeof(_main_node));

        // Searching for the approximate place (only the main node)
        while (key < tmp.key && tmp.pre != 0) {
            Ptr = tmp.pre;
            _list.seekg(Ptr);
            _list.read(reinterpret_cast<char*>(&tmp), sizeof(_main_node));
        }

        // Searching for the exact place

        if (tmp.pre == 0 && key < tmp.key) return std::make_pair(Ptr, -1);

        if (key == tmp.key || tmp.count == 0) return std::make_pair(Ptr, -1);

        _node tmpNode;
        int leftIndex = 0, rightIndex = tmp.count - 1;

        // the case that the key is between the main node and the first node
        _list.seekg(tmp.target);
        _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
        if (key < tmpNode.key) return std::make_pair(Ptr, -1);

        // the case that the key is right after the last node
        _list.seekg(tmp.target + (tmp.count - 1) * sizeof(_node));
        _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
        if (tmpNode.key < key || tmpNode.key == key) return std::make_pair(Ptr, tmp.count - 1);

        while (rightIndex - leftIndex > 1) {
            _list.seekg(tmp.target + ((rightIndex + leftIndex) / 2) * sizeof(_node));
            _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
            if (key < tmpNode.key) rightIndex = (rightIndex + leftIndex) / 2;
            else leftIndex = (rightIndex + leftIndex) / 2;
        }

        return std::make_pair(Ptr, leftIndex);
    }

    /**
     * This function return a pair of the pointer to the main node
     * and the number of node the target is right after, for the case
     * of the main node, the second member of the pair is -1.  If the
     * key doesn't belongs to the unrolled linked list, it will return
     * {-1, -1}.
     * @param key
     * @return a pair of the pointer to the main node and the offset
     */
    std::pair<ptr, int> _find_exact(const keyType& key)
    {
        if (_head.next == 0) return std::make_pair(-1, -1);

        _main_node tmp;
        ptr Ptr = _head.pre;
        _list.seekg(_head.pre);
        _list.read(reinterpret_cast<char*>(&tmp), sizeof(_main_node));

        // Searching for the approximate place (only the main node)
        while (key < tmp.key && tmp.pre != 0) {
            Ptr = tmp.pre;
            _list.seekg(Ptr);
            _list.read(reinterpret_cast<char*>(&tmp), sizeof(_main_node));
        }

        // Searching for the exact place

        if (tmp.pre == 0 && key < tmp.key) return std::make_pair(-1, -1);

        if (key == tmp.key) return std::make_pair(Ptr, -1);

        if (tmp.count == 0) return std::make_pair(-1, -1);

        _node tmpNode;
        int leftIndex = 0, rightIndex = tmp.count - 1;

        // the case that the key is between the main node and the first node
        _list.seekg(tmp.target);
        _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
        if (key < tmpNode.key) return std::make_pair(-1, -1);

        // the case that the key is right after the last node
        _list.seekg(tmp.target + (tmp.count - 1) * sizeof(_node));
        _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
        if (tmpNode.key < key) return std::make_pair(-1, -1);

        if (tmpNode.key == key) return std::make_pair(Ptr, tmp.count - 1);

        while (rightIndex - leftIndex > 1) {
            _list.seekg(tmp.target + ((rightIndex + leftIndex) / 2) * sizeof(_node));
            _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
            if (key < tmpNode.key) rightIndex = (rightIndex + leftIndex) / 2;
            else leftIndex = (rightIndex + leftIndex) / 2;
        }

        _list.seekg(tmp.target + leftIndex * sizeof(_node));
        _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
        if (tmpNode.key == key) return std::make_pair(Ptr, leftIndex);
        else return std::make_pair(-1, -1);
    }

    /**
     * This function delete a main node
     * @param mainNode the main node to be deleted
     * @param target the place where the main node to be deleted is
     */
    void _delete_node(_main_node& mainNode, ptr target)
    {
        _main_node pre, next;

        // The case that the only main node is to be deleted
        if (mainNode.pre == 0 && mainNode.next == 0) {
            _head.pre = 0;
            _head.next = 0;
            _list.seekp(0);
            _list.write(reinterpret_cast<char*>(&_head), sizeof(_first_node));
            return;
        }

        // The case that the main node is the first main Node
        if (mainNode.pre == 0) {
            _head.next = mainNode.next;
            _list.seekg(mainNode.next);
            _list.read(reinterpret_cast<char*>(&next), sizeof(_main_node));
            next.pre = 0;
            _list.seekp(mainNode.next);
            _list.write(reinterpret_cast<char*>(&next), sizeof(_main_node));
            _list.seekp(0);
            _list.write(reinterpret_cast<char*>(&_head), sizeof(_first_node));
            return;
        }

        // The case that the main node is the last main node
        if (mainNode.next == 0) {
            _head.pre = mainNode.pre;
            _list.seekp(mainNode.pre);
            _list.read(reinterpret_cast<char*>(&pre), sizeof(_main_node));
            pre.next = 0;
            _list.seekp(mainNode.pre);
            _list.write(reinterpret_cast<char*>(&pre), sizeof(_main_node));
            _list.seekp(0);
            _list.write(reinterpret_cast<char*>(&_head), sizeof(_first_node));
            return;
        }

        // The regular case
        _list.seekp(mainNode.pre);
        _list.read(reinterpret_cast<char*>(&pre), sizeof(_main_node));
        _list.seekg(mainNode.next);
        _list.read(reinterpret_cast<char*>(&next), sizeof(_main_node));
        pre.next = mainNode.next;
        next.pre = mainNode.pre;
        _list.seekp(mainNode.pre);
        _list.write(reinterpret_cast<char*>(&pre), sizeof(_main_node));
        _list.seekp(mainNode.next);
        _list.write(reinterpret_cast<char*>(&next), sizeof(_main_node));
    }

    /**
     * This function makes a new main node right after the target.
     * <br><br>
     * WARNING: If the target is 0, that means its a empty list.
     * @param mainNode the new main node to be added to the unrolled
     * linked list
     * @param target the place where the new node is added (it MUST
     * be a main node).
     * @return the new place of the new main node.
     */
    ptr _new_node(_main_node& mainNode, ptr target)
    {
        if (target != 0) { // for a not empty list
            _main_node pre, next;
            _list.seekg(target);
            _list.read(reinterpret_cast<char*>(&pre), sizeof(_main_node));
            if (pre.next != 0) { // the case that the next main node is not the first node
                // Get the next
                _list.seekg(pre.next);
                _list.read(reinterpret_cast<char*>(&next), sizeof(_main_node));

                // Set the previous and next node
                _list.seekp(0, std::ios::end);
                mainNode.next = pre.next;
                mainNode.pre = next.pre;
                pre.next = _list.tellp();
                next.pre = _list.tellp();

                // Put the new node
                mainNode.target = pre.next + sizeof(_main_node);
                _list.write(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));

                // Get a new space for its array
                _list.seekp(_head.maxNodeSize * sizeof(_node), std::ios::end);
                _list.write(reinterpret_cast<char*>(&_empty_node), sizeof(_node));

                // Put back the previous and next main node
                _list.seekp(mainNode.next);
                _list.write(reinterpret_cast<char*>(&next), sizeof(_main_node));
                _list.seekp(mainNode.pre);
                _list.write(reinterpret_cast<char*>(&pre), sizeof(_main_node));
                return pre.next;
            } else { // the case that the next main node is the first node
                // Set the previous and next node
                _list.seekp(0, std::ios::end);
                mainNode.next = 0;
                mainNode.pre = target;
                pre.next = _list.tellp();
                _head.pre = _list.tellp();

                // Put the new node
                mainNode.target = pre.next + sizeof(_main_node);
                _list.write(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));

                // Get a new space for its array
                _list.seekp(_head.maxNodeSize * sizeof(_node), std::ios::end);
                _list.write(reinterpret_cast<char*>(&_empty_node), sizeof(_node));

                // Put back the previous and next main node
                _list.seekp(mainNode.pre);
                _list.write(reinterpret_cast<char*>(&pre), sizeof(_main_node));
                _list.seekp(0);
                _list.write(reinterpret_cast<char*>(&_head), sizeof(_first_node));
                return pre.next;
            }
        } else { // For an empty list
            _list.seekp(0, std::ios::end);
            _head.next = _list.tellp();
            _head.pre = _list.tellp();
            mainNode.target = _head.next + sizeof(_main_node);
            mainNode.next = 0;
            mainNode.pre = 0;
            _list.write(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));

            // to reserve the space for the array of the main node
            _list.seekp(_head.maxNodeSize * sizeof(_node), std::ios::end);
            _list.write(reinterpret_cast<char*>(&_empty_node), sizeof(_node));
            _list.seekp(0);
            _list.write(reinterpret_cast<char*>(&_head), sizeof(_first_node));
            return _head.next;
        }
    }

    /**
     * This function split a main node into two.
     * @param mainNode
     * @param mainNodePtr
     * @return the ptr of the main node
     */
    ptr _split(_main_node& mainNode, ptr mainNodePtr)
    {
        // Copy the extra string of nodes
        auto* nodeBuffer = new _node[mainNode.count - _head.nodeSize];
        for (int i = 0; i < mainNode.count - _head.nodeSize; ++i) {
            _list.seekg(mainNode.target + sizeof(_node) * (_head.nodeSize + i));
            _list.read(reinterpret_cast<char*>(&nodeBuffer[i]), sizeof(_node));
        }

        // Create a new node
        ptr newMainNodePtr;
        _main_node newMainNode{nodeBuffer[0].key, nodeBuffer[0].value, 0,
                               mainNode.count - _head.nodeSize - 1, 0, 0};

        // Get a new space for the nodes
        newMainNodePtr = _new_node(newMainNode, mainNodePtr);

        // Change the count of the original node
        _list.seekg(mainNodePtr);
        _list.read(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));
        mainNode.count = _head.nodeSize;
        _list.seekp(mainNodePtr);
        _list.write(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));

        // Get the new main node
        _list.seekg(newMainNodePtr);
        _list.read(reinterpret_cast<char*>(&newMainNode), sizeof(_main_node));

        // write the extra string of nodes
        _list.seekp(newMainNode.target);
        for (int i = 1; i <= newMainNode.count; ++i) {
            _list.write(reinterpret_cast<char*>(&nodeBuffer[i]), sizeof(_node));
        }
        delete[] nodeBuffer;
        return newMainNodePtr;
    }

    /**
     * This function reads a string of stuff and returns a pointer to the string.
     * @param target the place to get the data
     * @param length
     * @return a pointer to the string
     */
    char* _read(ptr target, int length)
    {
        char* buffer = new char[length];
        _list.seekg(target);
        _list.read(buffer, length);
        return buffer;
    }

    /**
     * This function writes a string of stuff.
     * @param source the source pointer
     * @param target the place to place the data
     * @param length
     */
    void _write(char* source, ptr target, int length)
    {
        _list.seekp(target);
        _list.write(source, length);
    }

public:
    explicit UnrolledLinkedList(const std::string& fileName, int nodeSize = 316)
    : _list(fileName), _head{0, 0, nodeSize, 2 * nodeSize}
    {
        _list.seekg(0);
        _list.seekp(0);
        if (_list.peek() == EOF) {
            _list.seekg(0, std::ios::beg);
            _list.seekp(0, std::ios::beg);
            _list.write(reinterpret_cast<char*>(&_head), sizeof(_first_node));
        } else {
            _list.read(reinterpret_cast<char*>(&_head), sizeof(_first_node));
        }
    }

    ~UnrolledLinkedList() = default;

    /**
     * This function inserts a new key-value pair.
     * <br><br>
     * WARNING: the new node to be inserted CANNOT be the same of the
     * node in the unrolled linked list, or it may cause severe and
     * unexpected problem.
     * @param key the new key
     * @param value the value of the new key
     */
    void insert(const keyType& key, const valueType& value)
    {
        std::pair<ptr, int> position = _find(key);
        if (position.first == 0) {
            _main_node mainNode{key, value, 0, 0, 0, 0};
            _new_node(mainNode, 0);
            return;
        }

        // Get the main node
        _main_node mainNode; // the place to place the new node
        _list.seekg(position.first);
        _list.read(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));

        _node tmpNode;
        if (mainNode.pre == 0 && key < mainNode.key) {
            // Push back the data
            char* buffer = _read(mainNode.target, mainNode.count * sizeof(_node));
            _write(buffer, mainNode.target + sizeof(_node), mainNode.count * sizeof(_node));
            delete[] buffer;

            // Move the data in main node to the first node of its array
            tmpNode.key = mainNode.key;
            tmpNode.value = mainNode.value;
            _list.seekp(mainNode.target);
            _list.write(reinterpret_cast<char*>(&tmpNode), sizeof(_node));

            // set the new data in the main node
            mainNode.key = key;
            mainNode.value = value;
            ++(mainNode.count);
            _list.seekp(position.first);
            _list.write(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));
        } else {
            // Move the node(s) after the node to be inserted
            char* buffer = _read(mainNode.target + (position.second + 1) * sizeof(_node),
                                 (mainNode.count - position.second - 1) * sizeof(_node));
            _write(buffer, mainNode.target + (position.second + 2) * sizeof(_node),
                   (mainNode.count - position.second - 1) * sizeof(_node));
            delete[] buffer;

            // Put the new node
            _node newNode{key, value};
            _list.seekp(mainNode.target + (position.second + 1) * sizeof(_node));
            _list.write(reinterpret_cast<char*>(&newNode), sizeof(_node));

            // Change the main node
            ++(mainNode.count);
            _list.seekp(position.first);
            _list.write(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));
        }

        // Split the main node if it is larger its expected size
        if (mainNode.count >= _head.maxNodeSize) {
            _split(mainNode, position.first);
        }
    }

    void erase(const keyType& key)
    {
        std::pair<ptr, int> position = _find_exact(key);
        if (position.first == -1) return; // no such node

        // Get the main node
        _main_node mainNode;
        _node tmpNode;
        _list.seekg(position.first);
        _list.read(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));

        if (position.second == -1) { // the case that the target is in the main node
            if (mainNode.count == 0) { // the case that there is only one key-value pair
                _delete_node(mainNode, position.first);
            } else { // the case that there is more than one key-value pair
                // Set the main node
                _list.seekg(mainNode.target);
                _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
                mainNode.key = tmpNode.key;
                mainNode.value = tmpNode.value;
                --(mainNode.count);

                // Put the main Node
                _list.seekp(position.first);
                _list.write(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));

                // Move forward the other nodes
                char* buffer = _read(mainNode.target + sizeof(_node), mainNode.count * sizeof(_node));
                _write(buffer, mainNode.target, mainNode.count * sizeof(_node));
                delete[] buffer;
            }
        } else { // the case that the target is not in the main node
            // Set and put the main node
            --(mainNode.count);
            _list.seekp(position.first);
            _list.write(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));

            // Move forward the other nodes
            char* buffer = _read(mainNode.target + (position.second + 1) *sizeof(_node),
                                 (mainNode.count - position.second) * sizeof(_node));
            _write(buffer, mainNode.target + position.second *sizeof(_node),
                   (mainNode.count - position.second) * sizeof(_node));
            delete[] buffer;
        }
    }

    void modify(const keyType& key, const valueType& value)
    {
        std::pair<ptr, int> position = _find_exact(key);
        if (position.first == -1) return; // no such node
        _main_node mainNode;
        _list.seekg(position.first);
        _list.read(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));
        if (position.second == -1) {
            mainNode.value = value;
            _list.seekp(position.first);
            _list.write(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));
        } else {
            _node tmpNode;
            _list.seekg(mainNode.target + position.second * sizeof(_node));
            _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
            tmpNode.value = value;
            _list.seekp(mainNode.target + position.second * sizeof(_node));
            _list.write(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
        }
    }

    /**
     * The function clears all the data in the unrolled linked list
     */
    void clear()
    {
        _head.next = 0;
        _head.pre = 0;
        _list.seekp(0);
        _list.write(reinterpret_cast<char*>(&_head), sizeof(_first_node));
    }

    /**
     * This function gets the point of the value of a certain key.
     * If the key doesn't exist, a nullptr will be returned instead.
     * <br><br>
     * WARNING: If the function doesn't return a nullptr, ALWAYS free
     * the memory whenever you don't need it.
     * @param key
     * @return the point of the value of a certain key.
     * <br>
     * If the key doesn't exist, a nullptr will be returned instead.
     */
    valueType* get(const keyType& key)
    {
        std::pair<ptr, int> position = _find_exact(key);
        if (position.first == -1) return nullptr; // no such node
        _main_node mainNode;
        _list.seekg(position.first);
        _list.read(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));
        if (position.second == -1) {
            return new valueType(mainNode.value);
        } else {
            _node tmpNode;
            _list.seekg(mainNode.target + position.second * sizeof(_node));
            _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
            return new valueType(tmpNode.value);
        }
    }

    std::vector<valueType> traverse()
    {
        std::vector<valueType> values; // can be optimized
        _main_node mainNode;
        _node node;
        ptr mainPtr = _head.next;
        while (mainPtr != 0) {
            _list.seekg(mainPtr);
            _list.read(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));
            values.emplace_back(mainNode.value);
            for (int i = 0; i < mainNode.count; ++i) {
                _list.seekg(mainNode.target + i * sizeof(_node));
                _list.read(reinterpret_cast<char*>(&node), sizeof(_node));
                values.emplace_back(node.value);
            }
            mainPtr = mainNode.next;
        }
        return std::move(values);
    }

    void flush()
    {
        _list.flush();
    }
};

/**
 * @class DoubleUnrolledLinkedList
 *
 * This is a template class of double unrolled linked list on disk.
 * <br><br>
 * WARNING: the key type MUST have valid operator< and operator== !
 *
 * @tparam keyType1 Type of First key
 * @tparam keyType2 Type of Second key
 * @tparam valueType Type of Value
 */
template <class keyType1, class keyType2, class valueType>
class DoubleUnrolledLinkedList {
private:
    typedef int ptr;

    std::fstream _list;

    /// The following are private components of this linked list

    /**
     * @struct _first_node{next, pre, nodeSize, maxNodeSize}
     *
     * This is the node to pointer to the data, and metadata of the list
     */
    struct _first_node {
        ptr next; // the first main node

        ptr pre; // the last main node

        int nodeSize;

        int maxNodeSize;
    } _head;

    /**
     * @struct _main_node{key1, key2, value, target, count, next, pre}
     *
     * This is the node to store the pointer and data, and
     * link other main nodes.
     */
    struct _main_node {
        keyType1 key1;

        keyType2 key2;

        valueType value;

        ptr target;

        int count;

        ptr next;

        ptr pre;
    };

    /**
     * @struct _node{key1, key2, value}
     *
     * This is the Node to store data.
     */
    struct _node {
        keyType1 key1;

        keyType2 key2;

        valueType value;
    } _empty_node;

    /**
     * This function return a pair of the pointer to the main node
     * and the number of node the target is right after, for the case
     * of the main node, the second member of the pair is -1.  If the
     * key doesn't belongs to the unrolled linked list, it will the
     * node ahead of its place (except the case that the key is less than
     * any other keys, and in such case, it will return the first main node).
     * @param key
     * @return a pair of the pointer to the main node and the offset
     */
    std::pair<ptr, int> _find(const keyType1& key1, const keyType2& key2)
    {
        if (_head.next == 0) return std::make_pair(0, -1);

        _main_node tmp;
        ptr Ptr = _head.pre;
        _list.seekg(_head.pre);
        _list.read(reinterpret_cast<char*>(&tmp), sizeof(_main_node));

        // Searching for the approximate place (only the main node)
        while ((key1 < tmp.key1 || (key1 == tmp.key1 && key2 < tmp.key2)) && tmp.pre != 0) {
            Ptr = tmp.pre;
            _list.seekg(Ptr);
            _list.read(reinterpret_cast<char*>(&tmp), sizeof(_main_node));
        }

        // Searching for the exact place

        if (tmp.pre == 0 && (key1 < tmp.key1 || (key1 == tmp.key1 && key2 < tmp.key2))) return std::make_pair(Ptr, -1);

        if (key1 == tmp.key1 && key2 == tmp.key2 || tmp.count == 0) return std::make_pair(Ptr, -1);

        _node tmpNode;
        int leftIndex = 0, rightIndex = tmp.count - 1;

        // the case that the key is between the main node and the first node
        _list.seekg(tmp.target);
        _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
        if (key1 < tmpNode.key1 || (key1 == tmpNode.key1 && key2 < tmpNode.key2)) {
            return std::make_pair(Ptr, -1);
        }

        // the case that the key is right after the last node
        _list.seekg(tmp.target + (tmp.count - 1) * sizeof(_node));
        _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
        if (tmpNode.key1 < key1 || (tmpNode.key1 == key1 && (tmpNode.key2 < key2 || tmpNode.key2 == key2))) {
            return std::make_pair(Ptr, tmp.count - 1);
        }

        while (rightIndex - leftIndex > 1) {
            _list.seekg(tmp.target + ((rightIndex + leftIndex) / 2) * sizeof(_node));
            _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
            if (key1 < tmpNode.key1 || (key1 == tmpNode.key1 && key2 < tmpNode.key2)) {
                rightIndex = (rightIndex + leftIndex) / 2;
            } else {
                leftIndex = (rightIndex + leftIndex) / 2;
            }
        }

        return std::make_pair(Ptr, leftIndex);
    }

    /**
     * This function return a pair of the pointer to the main node
     * and the number of node the target is right after, for the case
     * of the main node, the second member of the pair is -1.  If the
     * key doesn't belongs to the unrolled linked list, it will return
     * {-1, -1}.
     * @param key1
     * @param key2
     * @return a pair of the pointer to the main node and the offset
     */
    std::pair<ptr, int> _find_exact(const keyType1& key1, const keyType2& key2)
    {
        if (_head.next == 0) return std::make_pair(-1, -1);

        _main_node tmp;
        ptr Ptr = _head.pre;
        _list.seekg(_head.pre);
        _list.read(reinterpret_cast<char*>(&tmp), sizeof(_main_node));

        // Searching for the approximate place (only the main node)
        while ((key1 < tmp.key1 || (key1 == tmp.key1 && key2 < tmp.key2)) && tmp.pre != 0) {
            Ptr = tmp.pre;
            _list.seekg(Ptr);
            _list.read(reinterpret_cast<char*>(&tmp), sizeof(_main_node));
        }

        // Searching for the exact place

        if (tmp.pre == 0 && (key1 < tmp.key1 || (key1 == tmp.key1 && key2 < tmp.key2))) return std::make_pair(-1, -1);

        if (key1 == tmp.key1 && key2 == tmp.key2) return std::make_pair(Ptr, -1);

        if (tmp.count == 0) return std::make_pair(-1, -1);

        _node tmpNode;
        int leftIndex = 0, rightIndex = tmp.count - 1;

        // the case that the key is between the main node and the first node
        _list.seekg(tmp.target);
        _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
        if (key1 < tmpNode.key1 || (key1 == tmpNode.key1 && key2 < tmpNode.key2)) return std::make_pair(-1, -1);

        // the case that the key is right after the last node
        _list.seekg(tmp.target + (tmp.count - 1) * sizeof(_node));
        _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
        if (tmpNode.key1 < key1 || (tmpNode.key1 == key1 && tmpNode.key2 < key2)) return std::make_pair(-1, -1);

        if (tmpNode.key1 == key1 && tmpNode.key2 == key2) return std::make_pair(Ptr, tmp.count - 1);

        while (rightIndex - leftIndex > 1) {
            _list.seekg(tmp.target + ((rightIndex + leftIndex) / 2) * sizeof(_node));
            _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
            if (key1 < tmpNode.key1 || (key1 == tmpNode.key1 && key2 < tmpNode.key2)) {
                rightIndex = (rightIndex + leftIndex) / 2;
            } else {
                leftIndex = (rightIndex + leftIndex) / 2;
            }
        }

        _list.seekg(tmp.target + leftIndex * sizeof(_node));
        _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
        if (tmpNode.key1 == key1 && tmpNode.key2 == key2) return std::make_pair(Ptr, leftIndex);
        else return std::make_pair(-1, -1);
    }

    /**
     * This function return a pair of the pointer to the main node
     * and the number of node the target is right after, for the case
     * of the main node, the second member of the pair is -1.  If the
     * key doesn't belongs to the unrolled linked list, it will return
     * {-1, -1}.
     * @param key1
     * @param key2
     * @return a pair of the pointer to the main node and the offset
     */
    std::pair<ptr, int> _single_find(const keyType1& key1)
    {
        if (_head.next == 0) return std::make_pair(-1, -1);

        _main_node tmp;
        ptr Ptr = _head.pre;
        _list.seekg(_head.pre);
        _list.read(reinterpret_cast<char*>(&tmp), sizeof(_main_node));
        bool exist = false;

        // Searching for the approximate place (only the main node)
        while ((key1 < tmp.key1 || key1 == tmp.key1) && tmp.pre != 0) {
            if (!exist && key1 == tmp.key1) exist = true;
            Ptr = tmp.pre;
            _list.seekg(Ptr);
            _list.read(reinterpret_cast<char*>(&tmp), sizeof(_main_node));
        }
        if (exist) {
            while (key1 == tmp.key1 && tmp.pre != 0) {
                Ptr = tmp.pre;
                _list.seekg(Ptr);
                _list.read(reinterpret_cast<char*>(&tmp), sizeof(_main_node));
            }
        }

        // Searching for the exact place

        if (tmp.pre == 0 && key1 < tmp.key1) return std::make_pair(-1, -1);

        if (tmp.pre == 0 && key1 == tmp.key1) return std::make_pair(Ptr, -1);

        if (tmp.count == 0) {
            if (exist) return std::make_pair(tmp.next, -1);
            else return std::make_pair(-1, -1);
        }

        _node tmpNode;
        int leftIndex = 0, rightIndex = tmp.count - 1;

        if (exist) {
            // for the last node
            _list.seekg(tmp.target + (tmp.count - 1) * sizeof(_node));
            _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
            if (tmpNode.key1 < key1) return std::make_pair(tmp.next, -1);

            // for the first node
            _list.seekg(tmp.target);
            _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
            if (tmpNode.key1 == key1) return std::make_pair(Ptr, 0);

            while (rightIndex - leftIndex > 1) {
                _list.seekg(tmp.target + ((rightIndex + leftIndex) / 2) * sizeof(_node));
                _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
                if (key1 == tmpNode.key1) rightIndex = (rightIndex + leftIndex) / 2;
                else leftIndex = (rightIndex + leftIndex) / 2;
            }
            return std::make_pair(Ptr, rightIndex);
        } else {
            // the case that the key is between the main node and the first node
            _list.seekg(tmp.target);
            _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
            if (key1 < tmpNode.key1) return std::make_pair(-1, -1);
            if (key1 == tmpNode.key1) return std::make_pair(Ptr, 0);

            // the case that the key is right after the last node
            _list.seekg(tmp.target + (tmp.count - 1) * sizeof(_node));
            _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
            if (tmpNode.key1 < key1) return std::make_pair(-1, -1);

            while (rightIndex - leftIndex > 1) {
                _list.seekg(tmp.target + ((rightIndex + leftIndex) / 2) * sizeof(_node));
                _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
                if (tmpNode.key1 < key1) leftIndex = (rightIndex + leftIndex) / 2;
                else rightIndex = (rightIndex + leftIndex) / 2;
            }

            _list.seekg(tmp.target + rightIndex * sizeof(_node));
            _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
            if (tmpNode.key1 == key1) return std::make_pair(Ptr, rightIndex);
            else return std::make_pair(-1, -1);
        }
    }

    /**
     * This function delete a main node
     * @param mainNode the main node to be deleted
     * @param target the place where the main node to be deleted is
     */
    void _delete_node(_main_node& mainNode, ptr target)
    {
        _main_node pre, next;

        // The case that the only main node is to be deleted
        if (mainNode.pre == 0 && mainNode.next == 0) {
            _head.pre = 0;
            _head.next = 0;
            _list.seekp(0);
            _list.write(reinterpret_cast<char*>(&_head), sizeof(_first_node));
            return;
        }

        // The case that the main node is the first main Node
        if (mainNode.pre == 0) {
            _head.next = mainNode.next;
            _list.seekg(mainNode.next);
            _list.read(reinterpret_cast<char*>(&next), sizeof(_main_node));
            next.pre = 0;
            _list.seekp(mainNode.next);
            _list.write(reinterpret_cast<char*>(&next), sizeof(_main_node));
            _list.seekp(0);
            _list.write(reinterpret_cast<char*>(&_head), sizeof(_first_node));
            return;
        }

        // The case that the main node is the last main node
        if (mainNode.next == 0) {
            _head.pre = mainNode.pre;
            _list.seekp(mainNode.pre);
            _list.read(reinterpret_cast<char*>(&pre), sizeof(_main_node));
            pre.next = 0;
            _list.seekp(mainNode.pre);
            _list.write(reinterpret_cast<char*>(&pre), sizeof(_main_node));
            _list.seekp(0);
            _list.write(reinterpret_cast<char*>(&_head), sizeof(_first_node));
            return;
        }

        // The regular case
        _list.seekp(mainNode.pre);
        _list.read(reinterpret_cast<char*>(&pre), sizeof(_main_node));
        _list.seekg(mainNode.next);
        _list.read(reinterpret_cast<char*>(&next), sizeof(_main_node));
        pre.next = mainNode.next;
        next.pre = mainNode.pre;
        _list.seekp(mainNode.pre);
        _list.write(reinterpret_cast<char*>(&pre), sizeof(_main_node));
        _list.seekp(mainNode.next);
        _list.write(reinterpret_cast<char*>(&next), sizeof(_main_node));
    }

    /**
     * This function makes a new main node right after the target.
     * <br><br>
     * WARNING: If the target is 0, that means its a empty list.
     * @param mainNode the new main node to be added to the unrolled
     * linked list
     * @param target the place where the new node is added (it MUST
     * be a main node).
     * @return the new place of the new main node.
     */
    ptr _new_node(_main_node& mainNode, ptr target)
    {
        if (target != 0) { // the case that the list isn't empty
            // Get the previous main node
            _main_node pre, next;
            _list.seekg(target);
            _list.read(reinterpret_cast<char*>(&pre), sizeof(_main_node));

            if (pre.next != 0) { // the node isn't the last main node
                // Get the next main node
                _list.seekg(pre.next);
                _list.read(reinterpret_cast<char*>(&next), sizeof(_main_node));

                // Change the data of both the previous and next node
                _list.seekp(0, std::ios::end);
                mainNode.next = pre.next;
                mainNode.pre = next.pre;
                pre.next = _list.tellp();
                next.pre = _list.tellp();

                // Set the new main node
                mainNode.target = pre.next + sizeof(_main_node);
                _list.write(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));

                // Set a new space
                _list.seekp(_head.maxNodeSize * sizeof(_node), std::ios::end);
                _list.write(reinterpret_cast<char*>(&_empty_node), sizeof(_node));

                // Put back both the previous and next node
                _list.seekp(mainNode.next);
                _list.write(reinterpret_cast<char*>(&next), sizeof(_main_node));
                _list.seekp(mainNode.pre);
                _list.write(reinterpret_cast<char*>(&pre), sizeof(_main_node));
                return pre.next;
            } else {
                // Change the data of both the previous and next node
                _list.seekp(0, std::ios::end);
                mainNode.next = 0;
                mainNode.pre = target;
                pre.next = _list.tellp();
                _head.pre = _list.tellp();

                // Set the new main node
                mainNode.target = pre.next + sizeof(_main_node);
                _list.write(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));

                // Set a new space
                _list.seekp(_head.maxNodeSize * sizeof(_node), std::ios::end);
                _list.write(reinterpret_cast<char*>(&_empty_node), sizeof(_node));

                // Put back both the previous and next node
                _list.seekp(mainNode.pre);
                _list.write(reinterpret_cast<char*>(&pre), sizeof(_main_node));
                _list.seekp(0);
                _list.write(reinterpret_cast<char*>(&_head), sizeof(_first_node));
                return pre.next;
            }
        } else { // the case of an empty list
            _list.seekp(0, std::ios::end);
            _head.next = _list.tellp();
            _head.pre = _list.tellp();
            mainNode.target = _head.next + sizeof(_main_node);
            mainNode.next = 0;
            mainNode.pre = 0;
            _list.write(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));

            // to reserve the space for the array of the main node
            _list.seekp(_head.maxNodeSize * sizeof(_node), std::ios::end);
            _list.write(reinterpret_cast<char*>(&_empty_node), sizeof(_node));
            _list.seekp(0);
            _list.write(reinterpret_cast<char*>(&_head), sizeof(_first_node));
            return _head.next;
        }
    }

    /**
     * This function split a main node into two.
     * @param mainNode
     * @param mainNodePtr
     * @return the ptr of the main node
     */
    ptr _split(_main_node& mainNode, ptr mainNodePtr)
    {
        // Copy the extra string of nodes
        auto* nodeBuffer = new _node[mainNode.count - _head.nodeSize];
        for (int i = 0; i < mainNode.count - _head.nodeSize; ++i) {
            _list.seekg(mainNode.target + sizeof(_node) * (_head.nodeSize + i));
            _list.read(reinterpret_cast<char*>(&nodeBuffer[i]), sizeof(_node));
        }

        // Create a new node
        ptr newMainNodePtr;
        _main_node newMainNode{nodeBuffer[0].key1, nodeBuffer[0].key2, nodeBuffer[0].value,
                               0, mainNode.count - _head.nodeSize - 1, 0, 0};

        // Get a new space for the nodes
        newMainNodePtr = _new_node(newMainNode, mainNodePtr);

        // Change the count of the original node
        _list.seekg(mainNodePtr);
        _list.read(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));
        mainNode.count = _head.nodeSize;
        _list.seekp(mainNodePtr);
        _list.write(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));

        // Get the new main node
        _list.seekg(newMainNodePtr);
        _list.read(reinterpret_cast<char*>(&newMainNode), sizeof(_main_node));

        // write the extra string of nodes
        _list.seekp(newMainNode.target);
        for (int i = 1; i <= newMainNode.count; ++i) {
            _list.write(reinterpret_cast<char*>(&nodeBuffer[i]), sizeof(_node));
        }
        delete[] nodeBuffer;
        return newMainNodePtr;
    }

    /**
     * This function reads a string of stuff and returns a pointer to the string.
     * @param target the place to get the data
     * @param length
     * @return a pointer to the string
     */
    char* _read(ptr target, int length)
    {
        char* buffer = new char[length];
        _list.seekg(target);
        _list.read(buffer, length);
        return buffer;
    }

    /**
     * This function writes a string of stuff.
     * @param source the source pointer
     * @param target the place to place the data
     * @param length
     */
    void _write(char* source, ptr target, int length)
    {
        _list.seekp(target);
        _list.write(source, length);
    }

public:
    explicit DoubleUnrolledLinkedList(const std::string& fileName, int nodeSize = 316)
    : _list(fileName), _head{0, 0, nodeSize, 2 * nodeSize}
    {
        _list.seekg(0);
        _list.seekp(0);
        if (_list.peek() == EOF) {
            _list.seekg(0, std::ios::beg);
            _list.seekp(0, std::ios::beg);
            _list.write(reinterpret_cast<char*>(&_head), sizeof(_first_node));
        } else {
            _list.read(reinterpret_cast<char*>(&_head), sizeof(_first_node));
        }
    }

    ~DoubleUnrolledLinkedList() = default;

    /**
     * This function inserts a new key-value pair.
     * <br><br>
     * WARNING: the new node to be inserted CANNOT be the same of the
     * node in the unrolled linked list, or it may cause severe and
     * unexpected problem.
     * @param key1 the new key1
     * @param key2 the new key2
     * @param value the value of the new key
     */
    void insert(const keyType1& key1, const keyType2& key2, const valueType& value)
    {
        std::pair<ptr, int> position = _find(key1, key2);
        if (position.first == 0) {
            _main_node mainNode{key1, key2, value, 0, 0, 0, 0};
            _new_node(mainNode, 0);
            return;
        }

        // Get the main node
        _main_node mainNode; // the place to place the new node
        _list.seekg(position.first);
        _list.read(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));

        _node tmpNode;
        if (mainNode.pre == 0 && (key1 < mainNode.key1 || (key1 == mainNode.key1 && key2 < mainNode.key2))) {
            // Push back the data
            char* buffer = _read(mainNode.target, mainNode.count * sizeof(_node));
            _write(buffer, mainNode.target + sizeof(_node), mainNode.count * sizeof(_node));
            delete[] buffer;

            // Move the data in main node to the first node of its array
            tmpNode.key1 = mainNode.key1;
            tmpNode.key2 = mainNode.key2;
            tmpNode.value = mainNode.value;
            _list.seekp(mainNode.target);
            _list.write(reinterpret_cast<char*>(&tmpNode), sizeof(_node));

            // set the new data in the main node
            mainNode.key1 = key1;
            mainNode.key2 = key2;
            mainNode.value = value;
            ++(mainNode.count);
            _list.seekp(position.first);
            _list.write(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));
        } else {
            // Move the node(s) after the node to be inserted
            char* buffer = _read(mainNode.target + (position.second + 1) * sizeof(_node),
                                 (mainNode.count - position.second - 1) * sizeof(_node));
            _write(buffer, mainNode.target + (position.second + 2) * sizeof(_node),
                   (mainNode.count - position.second - 1) * sizeof(_node));
            delete[] buffer;

            // Put the new node
            _node newNode{key1, key2, value};
            _list.seekp(mainNode.target + (position.second + 1) * sizeof(_node));
            _list.write(reinterpret_cast<char*>(&newNode), sizeof(_node));

            // Change the main node
            ++(mainNode.count);
            _list.seekp(position.first);
            _list.write(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));
        }

        // Split the main node if it is larger its expected size
        if (mainNode.count >= _head.maxNodeSize) {
            _split(mainNode, position.first);
        }
    }

    void erase(const keyType1& key1, const keyType2& key2)
    {
        std::pair<ptr, int> position = _find_exact(key1, key2);
        if (position.first == -1) return; // no such node

        // Get the main node
        _main_node mainNode;
        _node tmpNode;
        _list.seekg(position.first);
        _list.read(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));

        if (position.second == -1) { // the case that the data is in the main node
            if (mainNode.count == 0) { // the case that the main node has no other members
                _delete_node(mainNode, position.first);
            } else { // the case that the main node has other members
                // Set the main node
                _list.seekg(mainNode.target);
                _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
                mainNode.key1 = tmpNode.key1;
                mainNode.key2 = tmpNode.key2;
                mainNode.value = tmpNode.value;
                --(mainNode.count);

                // Put the main Node
                _list.seekp(position.first);
                _list.write(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));

                // Move forward the other nodes
                char* buffer = _read(mainNode.target + sizeof(_node), mainNode.count * sizeof(_node));
                _write(buffer, mainNode.target, mainNode.count * sizeof(_node));
                delete[] buffer;
            }
        } else { // the case that the data is in the array of the main node
            // Set and put the main node
            --(mainNode.count);
            _list.seekp(position.first);
            _list.write(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));

            // Move forward the other nodes
            char* buffer = _read(mainNode.target + (position.second + 1) *sizeof(_node),
                                 (mainNode.count - position.second) * sizeof(_node));
            _write(buffer, mainNode.target + position.second *sizeof(_node),
                   (mainNode.count - position.second) * sizeof(_node));
            delete[] buffer;
        }
    }

    void modify(const keyType1& key1, const keyType2& key2,
                const valueType& value)
    {
        std::pair<ptr, int> position = _find_exact(key1, key2);
        if (position.first == -1) return; // no such node\

        // Get the main node
        _main_node mainNode;
        _list.seekg(position.first);
        _list.read(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));

        if (position.second == -1) { // the case that the data is in the main node
            mainNode.value = value;
            _list.seekp(position.first);
            _list.write(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));
        } else { // the case that the data is in the array of the main node
            _node tmpNode;
            _list.seekg(mainNode.target + position.second * sizeof(_node));
            _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
            tmpNode.value = value; // Modify the value
            _list.seekp(mainNode.target + position.second * sizeof(_node));
            _list.write(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
        }
    }

    /**
     * The function clears all the data in the unrolled linked list
     */
    void clear()
    {
        _head.next = 0;
        _head.pre = 0;
        _list.seekp(0);
        _list.write(reinterpret_cast<char*>(&_head), sizeof(_first_node));
    }

    /**
     * This function gets the point of the value of a certain key pair.
     * If the key pair doesn't exist, a nullptr will be returned instead.
     * <br><br>
     * WARNING: If the function doesn't return a nullptr, ALWAYS free
     * the memory whenever you don't need it.
     * @param key1
     * @param key2
     * @return the point of the value of a certain key.
     * <br>
     * If the key pair doesn't exist, a nullptr will be returned instead.
     */
    valueType* get(const keyType1& key1, const keyType2& key2)
    {
        std::pair<ptr, int> position = _find_exact(key1, key2);
        if (position.first == -1) return nullptr; // no such node

        _main_node mainNode;
        _list.seekg(position.first);
        _list.read(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));
        if (position.second == -1) {
            return new valueType(mainNode.value);
        } else {
            _node tmpNode;
            _list.seekg(mainNode.target + position.second * sizeof(_node));
            _list.read(reinterpret_cast<char*>(&tmpNode), sizeof(_node));
            return new valueType(tmpNode.value);
        }
    }

    std::vector<valueType> traverse()
    {
        std::vector<valueType> values; // can be optimized
        _main_node mainNode;
        _node node;
        ptr mainPtr = _head.next;
        while (mainPtr != 0) {
            _list.seekg(mainPtr);
            _list.read(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));
            values.emplace_back(mainNode.value);
            for (int i = 0; i < mainNode.count; ++i) {
                _list.seekg(mainNode.target + i * sizeof(_node));
                _list.read(reinterpret_cast<char*>(&node), sizeof(_node));
                values.emplace_back(node.value);
            }
            mainPtr = mainNode.next;
        }
        return std::move(values);
    }

    std::vector<valueType> traverse(const keyType1& key1)
    {
        std::vector<valueType> values; // can be optimized

        std::pair<ptr, int> position = _single_find(key1);

        // The case of there are no such key
        if (position.first == -1) {
            return std::move(values);
        }

        ptr Ptr = position.first;
        _main_node mainNode;
        _node node;
        _list.seekg(Ptr);
        _list.read(reinterpret_cast<char*>(&mainNode), sizeof(_main_node));
        if (position.second != -1) {
            // Traverse all the data in the array of the main node
            for (int i = position.second; i < mainNode.count; ++i) {
                _list.seekg(mainNode.target + i * sizeof(_node));
                _list.read(reinterpret_cast<char*>(&node), sizeof(_node));
                if (!(node.key1 == key1)) break;
                values.emplace_back(node.value);
            }

            // Move to the next node
            Ptr = mainNode.next;
            _list.seekg(Ptr);
            _list.read(reinterpret_cast<char*>(&mainNode), sizeof(mainNode));
        }
        while (mainNode.key1 == key1) {
            values.emplace_back(mainNode.value);

            // Traverse all the data in the array of the main node
            for (int i = 0; i < mainNode.count; ++i) {
                _list.seekg(mainNode.target + i * sizeof(_node));
                _list.read(reinterpret_cast<char*>(&node), sizeof(_node));
                if (!(node.key1 == key1)) break;
                values.emplace_back(node.value);
            }

            // Move to the next node
            Ptr = mainNode.next;
            _list.seekg(Ptr);
            _list.read(reinterpret_cast<char*>(&mainNode), sizeof(mainNode));
        }
        return std::move(values);
    }

    void flush()
    {
        _list.flush();
    }
};

#endif // UNROLLED_LINKED_LIST
