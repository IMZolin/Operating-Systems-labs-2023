#pragma once
#include <atomic>
#include <iostream>

template<typename T>
class Queue {
public:
    void push(T val);
    void pop();
    bool empty();
    T front();

protected:
    struct Node {
        T data;
        Node* next;

        Node(const T& val) : data(val), next(nullptr) {}
    };

    std::atomic<Node*> head = nullptr;
    std::atomic<Node*> tail = nullptr;
};

template<typename T>
void Queue<T>::push(T val) {
    auto newNode = new Node(val);
    Node* last = tail.exchange(newNode);
    if (!last) {
        head.store(newNode);
    } else {
        last->next = newNode;
    }
}

template<typename T>
void Queue<T>::pop() {
    Node* first = head.load();
    if (!first) {
        return;
    }

    Node* newFirst = first->next;
    if (!newFirst) {
        head.store(nullptr);
        tail.store(nullptr);
    } else {
        head.store(newFirst);
    }

    delete first;
}

template<typename T>
bool Queue<T>::empty() {
    return head.load() == nullptr;
}

template<typename T>
T Queue<T>::front() {
    Node* first = head.load();
    if (!first) {
        return T();
    }

    return first->data;
}