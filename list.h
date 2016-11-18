#define MAX_CHARS 10

#include <stdio.h>
#include <string.h>

class Node {
public:
  Node(char *, Node * = NULL);
  ~Node();
  
  char *getSong() const;
  Node *getNext() const;

  void setNext(Node *);
private:
  char *song;
  Node *next;
};

class List {
public:
  List();
  ~List();
  void insertAtRear(char *);
  char *getCurrent() const;
  void moveCurrent();
private:
  Node *head;
  Node *rear;
  Node *curSong;
};

