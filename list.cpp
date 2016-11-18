#include "list.h"

Node::Node(char *s, Node *n) {
  int len = strlen(s);
  song = new char[len + 1];
  strcpy(song, s);

  next = n; 
}

Node::~Node() {
  delete[] song;
  song = NULL;
  next = NULL;
}

char *Node::getSong() const {
  return song;
}

Node *Node::getNext() const {
  return next;
}

void Node::setNext(Node *n) {
  next = n;
}

List::List() {
  head = rear = curSong = NULL;
}

List::~List() {
  if(head) {
    while(head->getNext() != rear) {
      Node *tmp = head;
      head = head->getNext();
      delete tmp;
    }
  }
  head = rear = curSong = NULL;
}

void List::insertAtRear(char *s) {
  if(head == NULL) {
    head = new Node(s);
    head->setNext(head);
    rear = curSong = head;
  } else {
    rear->setNext(new Node(s, head));
    rear = rear->getNext();
  }
}

char *List::getCurrent() const {
  return curSong->getSong();
}

void List::moveCurrent() {
  curSong = curSong->getNext();
}

