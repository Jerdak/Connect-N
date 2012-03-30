/*
 * Node.h
 *
 *  Created on: Jan 26, 2012
 *      Author: Derek
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <math.h>

#ifndef NODE_H_
#define NODE_H_

using namespace std;

/*Structure section*/

int pow(int a, int b){
	return pow((float)a,(float)b);
}
struct Chain {
	Chain(){
		SPPoI[0] = -1000;
		SPPoI[1] = -1000;

		EPPoI[0] = -1000;
		EPPoI[1] = -1000;

		pchain = NULL;
	}
	int length, direction;
	int SP[2];				//JC - Is this the right size?
	int EP[2];				//JC - Is this the right size?
	int SPPoI [2];
    int EPPoI [2];
	Chain * pchain;
};

struct Move {
	Move(){
		memset(updates,0,sizeof(bool) * 7);
		value = 0;
		y = 0;
		useable = true;
	}
	bool useable;
	int y;
	int value;
	bool updates[7];
};

struct Chip {
	Chip(){
		pchip = NULL;
	}
	int x, y;
	Chip * pchip;
};

class Node {
	double nodeValue;
	Chain *FriendlyChains;
	Chain *EnemyChains;
	Move *Moves;
	Chip *FriendlyAll;
	Chip *EnemyAll;
	int width;
    int height;
    int N;
    int iWin;
public:
	Node (int w, int h, int n);
	Node (Node * n);
	~Node ();
	int getWin () {return iWin;}
	int getWidth () {return width;}
	int getHeight () {return height;}
	int getN () {return N;}
	double getNodeValue () {return nodeValue;}
	Chain *getFCPointer () { return FriendlyChains;}
	Chain *getECPointer () { return EnemyChains;}
	Chip *getFAllPointer () { return FriendlyAll;}	//JC - Originally this was 'Chain * getFAllPointer()' but FriendlyAll is a Chip.
	Chip *getEAllPointer () { return EnemyAll;}	//JC - Originally this was 'Chain * getEAllPointer()' but EnemyAll is a Chip.
	Move *getMoves () {return Moves;}
	Node *getNode (int x) {return nodeChildren[x];}
	void addChain (int x1, int y1, int x2, int y2, bool me);
	void addChip(int x, int y, bool me);
	void addMove (int x);
	void deleteMove (int x);
	void findMoveUpdates (int x);
	void findMoveValue (int x);
	void findNodeValue ();
	void updateNode(int x, bool me);
	void printState ();
    Node **nodeChildren;
};


Node::Node(int w, int h, int n){
	iWin = 0;
	nodeValue = 1;
	width = w;
	height = h;
	N = n;
	FriendlyChains = NULL;
	EnemyChains = NULL;
	FriendlyAll = NULL;
	EnemyAll = NULL;
	Move *mlist = new Move [width];
	Moves = mlist;
	Node **nlist = new Node *[width];
	nodeChildren = nlist;
}

Node::Node(Node * n){
	this->width = n->getWidth();
	this->height = n->getHeight();
	this->N = n->getN();
	this->iWin = n->getWin();
	this->nodeValue = n->getNodeValue();
	this->Moves = n->getMoves();
	Chip *p1 = n->getFAllPointer();
	while(p1 != NULL){
		this->addChip(p1->x, p1->y, true);
		p1 = p1->pchip;
	}
	p1 = n->getEAllPointer();
	while(p1 != NULL){
		this->addChip(p1->x,p1->y,false);
		p1 = p1->pchip;
	}
	Chain *p2 = n->getFCPointer();
	while(p2 != NULL){
		this->addChain(p2->SP[0],p2->SP[1],p2->EP[0],p2->EP[1],true);
		p2 = p2->pchain;
	}
	p2 = n->getECPointer();
	while(p2 != NULL){
		this->addChain(p2->SP[0],p2->SP[1],p2->EP[0],p2->EP[1],false);
		p2 = p2->pchain;
	}
}

Node::~Node (){
	if(FriendlyChains){
		delete [] FriendlyChains;
		FriendlyChains = NULL;
	}
	if(EnemyChains){
		delete [] EnemyChains;
		EnemyChains = NULL;
	}
	if(Moves){
		delete [] Moves;
		Moves = NULL;
	}
	if(FriendlyAll){
		delete [] FriendlyAll;
		FriendlyAll = NULL;
	}
	if(EnemyAll){
		delete [] EnemyAll;
		EnemyAll = NULL;
	}
	if(nodeChildren){
		delete [] nodeChildren;
		nodeChildren = NULL;
	}
}

/*Cretaes a new node that is a copy of the parameter
Node::Node(Node *n){
	if (n != 0){

		//JC - Is the copy supposed to point to the data in 'n' or duplicate it?  As it's written below the variables will simply point to n's data.
		FriendlyChains = n->getFCPointer();
		EnemyChains = n->getECPointer();
		Moves = n->getMoves();					//JC- Undefined function
		FriendlyAll = n->getFAllPointer();
		EnemyAll = n->getEAllPointer();
	}
}*/

//Has node make move for column x
//If it's the program's turn, bool me should take a value of true
void Node::updateNode(int x, bool me){
	if((x >= 0)&&(x < width)){
		Chain * p1, *p2, *p3, *p4;
		Chip * p5;
		int reverse = 0;
		bool largeChain = false;
		int temp [] = {0,0,0,0};
		addChip(x, Moves[x].y, me);		//JC - if x < 0 || x > |Moves, crash.
		/*This section blocks out chains that belong to off player*/
		//Link to off player's chains
		if(me){
			p1 = EnemyChains;
			p2 = EnemyChains;
		}
		else{
			p1 = FriendlyChains;
			p2 = FriendlyChains;
		}
		//Search chain list
		while(p1 != NULL){
			//If one of the chains requires the new chip at its start:
			if((p1->SPPoI[0] == x) && (p1->SPPoI[1] == Moves[x].y)){
				//Block off start;
				p1->SPPoI[0] = -1000;
				p1->SPPoI[1] = -1000;
				//Update list of possible updates
				switch(p1->direction){
					case 1:
						Moves[x].updates[5] = false;
						break;
					case 2:
						Moves[x].updates[6] = false;
						break;
					case 4:
						Moves[x].updates[0] = false;
						break;
					default:
						break;
				}
			}
			//If one of the chains requires the new chip at its end:
			else if((p1->EPPoI[0] == x) && (p1->EPPoI[1] == Moves[x].y)){
				//Block off end
				p1->EPPoI[0] = -1000;
				p1->EPPoI[1] = -1000;
				//Update list of possible updates
				switch(p1->direction){
					case 1:
						Moves[x].updates[1] = false;
						break;
					case 2:
						Moves[x].updates[2] = false;
						break;
					case 3:
						Moves[x].updates[3] = false;
						break;
					case 4:
						Moves[x].updates[4] = false;
						break;
					default:
						break;
				}
			}
			//If both the start and end have been blocked off:
			if((p1->SPPoI[0] < 0)&&(p1->SPPoI[1] < 0)&&(p1->EPPoI[0] < 0)&&(p1->EPPoI[1] < 0)){
				//Remove chain
				if(me){
					if(iWin == 0){
						nodeValue -= pow(10,(p1->length-N));
					}
					if(p1 == EnemyChains){
						p2 = p1->pchain;
						EnemyChains = p2;
						p1 = p1->pchain;
					}
					else{
						p2->pchain = p1->pchain;
						p1 = p1->pchain;
					}
				}
				else{
					if(iWin == 0){
						nodeValue += pow(10,(p1->length-N));
					}
					if(p1 == FriendlyChains){
						p2 = p1->pchain;
						FriendlyChains = p2;
						p1 = p1->pchain;
					}
					else{
						p2->pchain = p1->pchain;
						p1 = p1->pchain;
					}
				}
			}
			//If not: move to next chain
			else{
				if(me){
					if(p2 == p1){
						p1 = p1->pchain;
					}
					else{
						p2 = p1;
						p1 = p1->pchain;
					}
				}
				else{
					if(p2 == p1){
						p1 = p1->pchain;
					}
					else{
						p2 = p1;
						p1 = p1->pchain;
					}
				}
			}
		}

		/*This section expands on player chains*/
		//Link to on player's chains
		if(me){
			p1 = FriendlyChains;
			p2 = FriendlyChains;
		}
		else{
			p1 = EnemyChains;
			p2 = EnemyChains;
		}
		//Search chain list
		while(p1 != NULL){
			for(int i = 0; i < 4; i++){
				temp[i] = 0;
			}
			//If one of the chains requires the new chip at its start:
			if((p1->SPPoI[0] == x) && (p1->SPPoI[1] == Moves[x].y)){
				if(p1->direction == 1)
					Moves[x].updates[5] = false;
				if(p1->direction == 2)
					Moves[x].updates[6] = false;
				if(p1->direction == 4)
					Moves[x].updates[0] = false;
				//If there is a on player chip opposite the chain's start
				if(Moves[x].updates[p1->direction]){
					//Note that
					largeChain = true;
					//Update list of possible updates
					if(p1->direction == 1)
						Moves[x].updates[1] = false;
					if(p1->direction == 2)
						Moves[x].updates[2] = false;
					if(p1->direction == 4)
						Moves[x].updates[4] = false;
					//Link to on player's chains, again
					if(me){
						p3 = FriendlyChains;
						p4 = FriendlyChains;
					}
					else{
						p3 = EnemyChains;
						p4 = EnemyChains;
					}
					//Search through on player's chains, again
					while(p3 != NULL){
						//If there is a chain going in the same direction:
						if(p3->direction == p1->direction){
							//If it requires the new chip at its end:
							if((p3->EPPoI[0] == x) && (p3->EPPoI[1] == Moves[x].y)){
								//Store the second chain's start and the first chain's end
								temp[0] = p3->SP[0];
								temp[1] = p3->SP[1];
								temp[2] = p1->EP[0];
								temp[3] = p1->EP[1];
								//Remove chains
								if(me){
									if(iWin == 0){
										nodeValue -= pow(10,(p1->length-N));
										nodeValue -= pow(10,(p3->length-N));
									}
									if(p1 == p4){
										if(p2 == FriendlyChains){
											p2 = p3->pchain;
											FriendlyChains = p2;
											p1 = p2;
										}
										else{
											p2->pchain = p3->pchain;
											p1 = p3->pchain;
										}
									}
									else if(p2 == p3){
										if(p4 == FriendlyChains){
											p2 = p1->pchain;
											FriendlyChains = p2;
											p1 = p2;
										}
										else{
											p2 = p4;
											p2->pchain = p1->pchain;
											p1 = p1->pchain;
										}
									}
									else{
										if(p1 == FriendlyChains){
											p1 = p1->pchain;
											p2 = p1;
											FriendlyChains = p1;
										}
										else{
											p2->pchain = p1->pchain;
											p1 = p1->pchain;
										}
										if(p3 == FriendlyChains){
											p3 = p3->pchain;
											p4 = p3;
											FriendlyChains = p3;
										}
									}
								}
								else{
									if(iWin == 0){
										nodeValue += pow(10,(p1->length-N));
										nodeValue += pow(10,(p3->length-N));
									}
									if(p1 == p4){
										if(p2 == EnemyChains){
											p2 = p3->pchain;
											EnemyChains = p2;
											p1 = p2;
										}
										else{
											p2->pchain = p3->pchain;
											p1 = p3->pchain;
										}
									}
									else if(p2 == p3){
										if(p4 == EnemyChains){
											p2 = p1->pchain;
											EnemyChains = p2;
											p1 = p2;
										}
										else{
											p2 = p4;
											p2->pchain = p1->pchain;
											p1 = p1->pchain;
										}
									}
									else{
										if(p1 == EnemyChains){
											p1 = p1->pchain;
											p2 = p1;
											EnemyChains = p1;
										}
										else{
											p2->pchain = p1->pchain;
											p1 = p1->pchain;
										}
										if(p3 == EnemyChains){
											p3 = p3->pchain;
											p4 = p3;
											EnemyChains = p3;
										}
									}
								}
								//Add new chain that combines them
								addChain(temp[0], temp[1], temp[2], temp[3], me);
								p3 = NULL;
								p4 = NULL;
								largeChain = false;
							}
							else{
								p4 = p3;
								p3 = p3->pchain;
							}
						}
						//Move to next chain
						else{
							p4 = p3;
							p3 = p3->pchain;
						}
					}
					//If there is no parallel chain opposite the chosen chain
					if(largeChain){
						//Determine the chip's location
						if(p1->direction == 1){
							temp[0] = x-1;
							temp[1] = Moves[x].y;
							temp[2] = x-2;
							temp[3] = temp[1];
						}
						else if(p1->direction == 2){
							temp[0] = x-1;
							temp[1] = Moves[x].y-1;
							temp[2] = x-2;
							temp[3] = temp[1]-1;
						}
						else if(p1->direction == 4){
							temp[0] = x+1;
							temp[1] = Moves[x].y-1;
							temp[2] = x+2;
							temp[3] = temp[1]-1;
						}
						//Link to on player's chip list
						if(me)
							p5 = FriendlyAll;
						else
							p5 = EnemyAll;
						//Search chip list
						while(p5 != NULL){
							//If there is a on player chip there:
							if((p5->x == temp[0])&&(p5->y == temp[1])){
								//Add it and the new chip to chain
								p1->length = 2+p1->length;
								if(p1->length >= N){
									if(me){
										iWin = 1;
										nodeValue = 2.0;
									}
									else{
										iWin = -1;
										nodeValue = 0.0;
									}
								}
								if(iWin == 0){
									if(p1->length >= N){
										if(me){
											nodeValue = 2.0;
											iWin = 1;
										}
										else{
											nodeValue = 0.0;
											iWin = -1;
										}
									}
									else{
										if(me){
											nodeValue -= pow(10,(p1->length - 2 - N));
											nodeValue += pow(10,(p1->length - N));
										}
										else{
											nodeValue += pow(10,(p1->length - 2 - N));
											nodeValue -= pow(10,(p1->length - N));
										}
									}
								}
								p1->SP[0] = temp[0];
								p1->SP[1] = temp[1];
								if((temp[2] >= 0)&&(temp[2] < width)&&(temp[3] >= 0)&&(temp[3] < height)){
									p1->SPPoI[0] = temp[2];
									p1->SPPoI[1] = temp[3];
								}
								else{
									p1->SPPoI[0] = -1000;
									p1->SPPoI[1] = -1000;
								}
								largeChain = false;
								p5 = NULL;
							}
							//Move on
							else
								p5 = p5->pchip;
						}
					}
				}
				//If there is not a on player chip opposite the chain
				else{
					//Add new chip to chain
					p1->length++;
					if(p1->length >= N){
						if(me){
							iWin = 1;
							nodeValue = 2.0;
						}
						else{
							iWin = -1;
							nodeValue = 0.0;
						}
					}
					if(iWin == 0){
						if(p1->length >= N){
							if(me){
								nodeValue = 2.0;
								iWin = 1;
							}
							else{
								nodeValue = 0.0;
								iWin = -1;
							}
						}
						else{
							if(me){
								nodeValue -= pow(10,(p1->length - 1 - N));
								nodeValue += pow(10,(p1->length - N));
							}
							else{
								nodeValue += pow(10,(p1->length - 1 - N));
								nodeValue -= pow(10,(p1->length - N));
							}
						}
					}
					p1->SP[0] = x;
					p1->SP[1] = Moves[x].y;
					if(p1->direction == 1){
						if(x > 0){
							p1->SPPoI[0] = x-1;
							p1->SPPoI[1] = Moves[x].y;
						}
						else{
							p1->SPPoI[0] = -1000;
							p1->SPPoI[1] = -1000;
						}
					}
					else if(p1->direction == 2){
						if((x > 0)&&(Moves[x].y > 0)){
							p1->SPPoI[0] = x-1;
							p1->SPPoI[1] = Moves[x].y-1;
						}
						else{
							p1->SPPoI[0] = -1000;
							p1->SPPoI[1] = -1000;
						}
					}
					else if(p1->direction == 4){
						if((x < (width-1))&&(Moves[x].y > 0)){
							p1->SPPoI[0] = x+1;
							p1->SPPoI[1] = Moves[x].y-1;
						}
						else{
							p1->SPPoI[0] = -1000;
							p1->SPPoI[1] = -1000;
						}
					}
				}
				if(p1 != NULL){
					if(me)
						p5 = EnemyAll;
					else
						p5 = FriendlyAll;
					while(p5 != NULL){
						if((p5->x == p1->SPPoI[0])&&(p5->y == p1->SPPoI[1])){
							p1->SPPoI[0] = -1000;
							p1->SPPoI[1] = -1000;
						}
						if((p5->x == p1->EPPoI[0])&&(p5->y == p1->EPPoI[1])){
							p1->EPPoI[0] = -1000;
							p1->EPPoI[1] = -1000;
						}
						p5 = p5->pchip;
					}
					if((p1->SPPoI[0] < 0)&&(p1->SPPoI[1] < 0)&&(p1->EPPoI[0] < 0)&&(p1->EPPoI[1] < 0)){
						//Remove chain
						if(me){
							if(iWin == 0){
								nodeValue -= pow(10,(p1->length-N));
							}
							if(p1 == FriendlyChains){
								p2 = p1->pchain;
								FriendlyChains = p2;
								p1 = p1->pchain;
							}
							else{
								p2->pchain = p1->pchain;
								p1 = p1->pchain;
							}
						}
						else{
							if(iWin == 0){
								nodeValue += pow(10,(p1->length-N));
							}
							if(p1 == EnemyChains){
								p2 = p1->pchain;
								EnemyChains = p2;
								p1 = p1->pchain;
							}
							else{
								p2->pchain = p1->pchain;
								p1 = p1->pchain;
							}
						}
					}
				}
			}
			//If one of the chains requires the new chip at its end:
			else if((p1->EPPoI[0] == x) && (p1->EPPoI[1] == Moves[x].y)){
				Moves[x].updates[p1->direction] = false;
				//If there is a on player chip opposite the chain's end
				if(((p1->direction == 4)&&(Moves[x].updates[0]))||((p1->direction == 1)&&(Moves[x].updates[5]))||((p1->direction == 2)&&(Moves[x].updates[6]))){
					//Note that
					largeChain = true;
					//Update list of possible updates
					if(p1->direction == 4){
						Moves[x].updates[0] = false;
					}
					else{
						Moves[x].updates[4+p1->direction] = false;
					}
					//Link to on player's chains, again
					if(me){
						p3 = FriendlyChains;
						p4 = FriendlyChains;
					}
					else{
						p3 = EnemyChains;
						p4 = EnemyChains;
					}
					//Search through on player's chains, again
					while(p3 != NULL){
						//If there is a chain going in the same direction:
						if(p3->direction == p1->direction){
							//If it requires the new chip at its start:
							if((p3->SPPoI[0] == x) && (p3->SPPoI[1] == Moves[x].y)){
								//Store the first chain's start and the second chain's end
								temp[0] = p1->SP[0];
								temp[1] = p1->SP[1];
								temp[2] = p3->EP[0];
								temp[3] = p3->EP[1];
								//Remove chains
								if(me){
									if(iWin == 0){
										nodeValue -= pow(10,(p1->length-N));
										nodeValue -= pow(10,(p3->length-N));
									}
									if(p1 == p4){
										if(p2 == FriendlyChains){
											p2 = p3->pchain;
											FriendlyChains = p2;
											p1 = p2;
										}
										else{
											p2->pchain = p3->pchain;
											p1 = p3->pchain;
										}
									}
									else if(p2 == p3){
										if(p4 == FriendlyChains){
											p2 = p1->pchain;
											FriendlyChains = p2;
											p1 = p2;
										}
										else{
											p2 = p4;
											p2->pchain = p1->pchain;
											p1 = p1->pchain;
										}
									}
									else{
										if(p1 == FriendlyChains){
											p1 = p1->pchain;
											p2 = p1;
											FriendlyChains = p1;
										}
										else{
											p2->pchain = p1->pchain;
											p1 = p1->pchain;
										}
										if(p3 == FriendlyChains){
											p3 = p3->pchain;
											p4 = p3;
											FriendlyChains = p3;
										}
									}
								}
								else{
									if(iWin == 0){
										nodeValue += pow(10,(p1->length-N));
										nodeValue += pow(10,(p3->length-N));
									}
									if(p1 == p4){
										if(p2 == EnemyChains){
											p2 = p3->pchain;
											EnemyChains = p2;
											p1 = p2;
										}
										else{
											p2->pchain = p3->pchain;
											p1 = p3->pchain;
										}
									}
									else if(p2 == p3){
										if(p4 == EnemyChains){
											p2 = p1->pchain;
											EnemyChains = p2;
											p1 = p2;
										}
										else{
											p2 = p4;
											p2->pchain = p1->pchain;
											p1 = p1->pchain;
										}
									}
									else{
										if(p1 == EnemyChains){
											p1 = p1->pchain;
											p2 = p1;
											EnemyChains = p1;
										}
										else{
											p2->pchain = p1->pchain;
											p1 = p1->pchain;
										}
										if(p3 == EnemyChains){
											p3 = p3->pchain;
											p4 = p3;
											EnemyChains = p3;
										}
									}
								}
								//Add new chain that combines them
								addChain(temp[0], temp[1], temp[2], temp[3], me);
								p3 = NULL;
								p4 = NULL;
								largeChain = false;
							}
						}
						//Move to next chain
						else{
							p4 = p3;
							p3 = p3->pchain;
						}
					}
					//If there is no parallel chain opposite the chosen chain
					if(largeChain){
						//Determine the chip's location
						if(p1->direction == 1){
							temp[0] = x+1;
							temp[1] = Moves[x].y;
							temp[2] = x+2;
							temp[3] = temp[1];
						}
						else if(p1->direction == 2){
							temp[0] = x+1;
							temp[1] = Moves[x].y+1;
							temp[2] = x+2;
							temp[3] = temp[1]+1;
						}
						else if(p1->direction == 4){
							temp[0] = x-1;
							temp[1] = Moves[x].y+1;
							temp[2] = x-2;
							temp[3] = temp[1]+1;
						}
						//Link to on player's chip list
						if(me)
							p5 = FriendlyAll;
						else
							p5 = EnemyAll;
						//Search chip list
						while(p5 != NULL){
							//If there is a on player chip there:
							if((p5->x == temp[0])&&(p5->y == temp[1])){
								//Add it and the new chip to chain
								p1->length = 2+p1->length;
								if(p1->length >= N){
									if(me){
										iWin = 1;
										nodeValue = 2.0;
									}
									else{
										iWin = -1;
										nodeValue = 0.0;
									}
								}
								if(iWin == 0){
									if(p1->length >= N){
										if(me){
											nodeValue = 2.0;
											iWin = 1;
										}
										else{
											nodeValue = 0.0;
											iWin = -1;
										}
									}
									else{
										if(me){
											nodeValue -= pow(10,(p1->length - 2 - N));
											nodeValue += pow(10,(p1->length - N));
										}
										else{
											nodeValue += pow(10,(p1->length - 2 - N));
											nodeValue -= pow(10,(p1->length - N));
										}
									}
								}
								p1->EP[0] = temp[0];
								p1->EP[1] = temp[1];
								if((temp[2] >= 0)&&(temp[2] < width)&&(temp[3] >= 0)&&(temp[3] < height)){
									p1->EPPoI[0] = temp[2];
									p1->EPPoI[1] = temp[3];
								}
								else{
									p1->EPPoI[0] = -1000;
									p1->EPPoI[1] = -1000;
								}
								largeChain = false;
								p5 = NULL;
							}
							//Move on
							else
								p5 = p5->pchip;
						}
					}
				}
				//If there is not a on player chip opposite the chain
				else{
					//Add new chip to chain
					if(p1->direction == 3)
						Moves[x].updates[3] = false;
					p1->length++;
					if(p1->length >= N){
						if(me){
							iWin = 1;
							nodeValue = 2.0;
						}
						else{
							iWin = -1;
							nodeValue = 0.0;
						}
					}
					if(iWin == 0){
						if(p1->length >= N){
							if(me){
								nodeValue = 2.0;
								iWin = 1;
							}
							else{
								nodeValue = 0.0;
								iWin = -1;
							}
						}
						else{
							if(me){
								nodeValue -= pow(10,(p1->length - 1 - N));
								nodeValue += pow(10,(p1->length - N));
							}
							else{
								nodeValue += pow(10,(p1->length - 1 - N));
								nodeValue -= pow(10,(p1->length - N));
							}
						}
					}
					p1->EP[0] = x;
					p1->EP[1] = Moves[x].y;
					if(p1->direction == 1){
						if(x < (width-1)){
							p1->EPPoI[0] = x+1;
							p1->EPPoI[1] = Moves[x].y;
						}
						else{
							p1->EPPoI[0] = -1000;
							p1->EPPoI[1] = -1000;
						}
					}
					else if(p1->direction == 2){

						if((x < (width-1))&&(Moves[x].y < (height-1))){
							p1->EPPoI[0] = x+1;
							p1->EPPoI[1] = Moves[x].y+1;
						}
						else{
							p1->EPPoI[0] = -1000;
							p1->EPPoI[1] = -1000;
						}
					}
					else if(p1->direction == 3){
						if(Moves[x].y < (height-1)){
							p1->EPPoI[0] = x;
							p1->EPPoI[1] = Moves[x].y+1;
						}
						else{
							p1->EPPoI[0] = -1000;
							p1->EPPoI[1] = -1000;
						}
					}
					else if(p1->direction == 4){
						if((x > 0)&&(Moves[x].y < (height-1))){
							p1->EPPoI[0] = x-1;
							p1->EPPoI[1] = Moves[x].y+1;
						}
						else{
							p1->EPPoI[0] = -1000;
							p1->EPPoI[1] = -1000;
						}
					}
				}
				if(p1 != NULL){
					if(me)
						p5 = EnemyAll;
					else
						p5 = FriendlyAll;
					while(p5 != NULL){
						if((p5->x == p1->SPPoI[0])&&(p5->y == p1->SPPoI[1])){
							p1->SPPoI[0] = -1000;
							p1->SPPoI[1] = -1000;
						}
						if((p5->x == p1->EPPoI[0])&&(p5->y == p1->EPPoI[1])){
							p1->EPPoI[0] = -1000;
							p1->EPPoI[1] = -1000;
						}
						p5 = p5->pchip;
					}
					if((p1->SPPoI[0] < 0)&&(p1->SPPoI[1] < 0)&&(p1->EPPoI[0] < 0)&&(p1->EPPoI[1] < 0)){
						//Remove chain
						if(me){
							if(iWin == 0){
								nodeValue -= pow(10,(p1->length-N));
							}
							if(p1 == FriendlyChains){
								p2 = p1->pchain;
								FriendlyChains = p2;
								p1 = p1->pchain;
							}
							else{
								p2->pchain = p1->pchain;
								p1 = p1->pchain;
							}
						}
						else{
							if(iWin == 0){
								nodeValue += pow(10,(p1->length-N));
							}
							if(p1 == EnemyChains){
								p2 = p1->pchain;
								EnemyChains = p2;
								p1 = p1->pchain;
							}
							else{
								p2->pchain = p1->pchain;
								p1 = p1->pchain;
							}
						}
					}
				}
			}

			//If not: move to next chain
			else{
				if(p1 != NULL){
					if(me){
						if(p2 == FriendlyChains){
							p1 = p1->pchain;
						}
						else{
							p2 = p1;
							p1 = p1->pchain;
						}
					}
					else{
						if(p2 == EnemyChains){
							p1 = p1->pchain;
						}
						else{
							p2 = p1;
							p1 = p1->pchain;
						}
					}
				}
			}
		}
		/*This section determines if there are any solitary on player chips next to the new chip
		 * which need to be turned into chains*/
		//For all possible updates


		for(int i = 0; i < 7; i++){
			//If a direction could be updated
			for(int j = 0; j < 4; j++){
				temp[j] = 0;
			}
			if(Moves[x].updates[i]){
				//Determines chip's position
				reverse = 0;
				switch(i){
					case 0:
						temp[0] = x;
						temp[1] = Moves[x].y;
						temp[2] = x-1;
						temp[3] = temp[1]+1;
						break;
					case 1:
						temp[0] = x;
						temp[1] = Moves[x].y;
						temp[2] = x-1;
						temp[3] = temp[1];
						reverse = 1;
						break;
					case 2:
						temp[0] = x;
						temp[1] = Moves[x].y;
						temp[2] = x-1;
						temp[3] = temp[1]-1;
						reverse = 1;
						break;
					case 3:
						temp[0] = x;
						temp[1] = Moves[x].y;
						temp[2] = x;
						temp[3] = temp[1]-1;
						reverse = 1;
						break;
					case 4:
						temp[0] = x;
						temp[1] = Moves[x].y;
						temp[2] = x+1;
						temp[3] = temp[1]-1;
						reverse = 1;
						break;
					case 5:
						temp[0] = x;
						temp[1] = Moves[x].y;
						temp[2] = x+1;
						temp[3] = temp[1];
						break;
					case 6:
						temp[0] = x;
						temp[1] = Moves[x].y;
						temp[2] = x+1;
						temp[3] = temp[1]+1;
						break;
					default:
						break;
				}
				//Link to on player's chip list
				if(me){
					p5 = FriendlyAll;}
				else{
					p5 = EnemyAll;}
				//Search chip list
				while(p5 != NULL){
					//If there is a solitary on player chip next to new chip
					if((p5->x == temp[2])&&(p5->y == temp[3])){
						//Add new 2x chain
						if(reverse)
							this->addChain(temp[2], temp[3], temp[0], temp[1], me);
						else
							this->addChain(temp[0], temp[1], temp[2], temp[3], me);
						p5 = NULL;
					}
					//Move on
					else{
						p5 = p5->pchip;
					}
				}
			}
		}
		//Finally, update move x
		this->addMove(x);
	}
}

//Creates a new chain given two locations (start and end) and adds it to the top of the list
void Node::addChain(int x1, int y1, int x2, int y2, bool me){
	Chain *c = new Chain();
	Chip *p5;
	c->SP[0] = x1;
	c->SP[1] = y1;
	c->EP[0] = x2;
	c->EP[1] = y2;
	//Calculates direction and length of chain
	if((x1 == x2)&&(y1 < y2)){
		c->direction = 3;
		c->length = y2-y1+1;
	}
	else if((x1 > x2)&&(y1 < y2)){
		c->direction = 4;
		c->length = y2-y1+1;
	}
	else if(x1 < x2){
		if(y1 == y2){
			c->direction = 1;
			c->length = x2-x1+1;
		}
		else if(y1 < y2){
			c->direction = 2;
			c->length = x2-x1+1;
		}
	}

	if(c->length >= N){
		if(me){
			iWin = 1;
			nodeValue = 2.0;
		}
		else{
			iWin = -1;
			nodeValue = 0.0;
		}
	}
	//Calculates expansion points
	if(c->direction == 1){
		if(x1 > 0){
			c->SPPoI[0] = x1-1;
			c->SPPoI[1] = y1;
		}
		if(x2 < (width-1)){
			c->EPPoI[0] = x2+1;
			c->EPPoI[1] = y1;
		}
	}
	else if(c->direction == 2){
		if((x1 > 0) && (y1 > 0)){
			c->SPPoI[0] = x1-1;
			c->SPPoI[1] = y1-1;
		}
		if((x2 < (width-1)) && (y2 < (height-1))){
			c->EPPoI[0] = x2+1;
			c->EPPoI[1] = y2+1;
		}
	}
	else if(c->direction == 3){
		if(y2 < (height-1)){
			c->EPPoI[0] = x2;
			c->EPPoI[1] = y2+1;
		}
	}
	else if(c->direction == 4){
		if((x1 < (width-1)) && (y1 > 0)){
			c->SPPoI[0] = x1+1;
			c->SPPoI[1] = y1-1;
		}
		if((x2 > 0) && (y2 < (height-1))){
			c->EPPoI[0] = x2-1;
			c->EPPoI[1] = y2+1;
		}
	}
	//Check blocked chain points
	if(me)
		p5 = EnemyAll;
	else
		p5 = FriendlyAll;
	while(p5 != NULL){
		if((p5->x == c->SPPoI[0])&&(p5->y == c->SPPoI[1])){
			c->SPPoI[0] = -1000;
			c->SPPoI[1] = -1000;
		}
		if((p5->x == c->EPPoI[0])&&(p5->y == c->EPPoI[1])){
			c->EPPoI[0] = -1000;
			c->EPPoI[1] = -1000;
		}
		p5 = p5->pchip;
	}
	//If chain is good add it
	if(((c->SPPoI[0] >= 0)&&(c->SPPoI[0] < width)&&(c->SPPoI[1] >= 0)&&(c->SPPoI[1] < height))||((c->EPPoI[0] >= 0)&&(c->EPPoI[0] < width)&&(c->EPPoI[1] >= 0)&&(c->EPPoI[1] < height))){
		//Adds value to total
		if (iWin == 0){
			if(c->length >= N){
				if(me){
					nodeValue = 2.0;
					iWin = 1;
				}
				else{
					nodeValue = 0.0;
					iWin = -1;
				}
			}
			else{
				if(me){
					nodeValue += pow(10,(c->length-N));
				}
				else{
					nodeValue -= pow(10,(c->length-N));
				}
			}
		}
		//Adds chain to beginning of correct list
		if (me){
			c->pchain = FriendlyChains;
			FriendlyChains = c;
		}
		else{
			c->pchain = EnemyChains;
			EnemyChains = c;
		}
	}
}

//Creates a new chip given a location and adds it to the top of the list
void Node::addChip(int x, int y, bool me){
	Chip *c = new Chip();
	c->x = x;
	c->y = y;
	//Adds to top of correct list
	if(me){
		c->pchip = FriendlyAll;
		FriendlyAll = c;
	}
	else{
		c->pchip = EnemyAll;
		EnemyAll = c;
	}
}

//Blocks of move x
void Node::deleteMove(int x){
	if((x >= 0)&&(x < width)){
		Moves[x].y = width;
		Moves[x].useable = false;
		Moves[x].value = -1000;
	}
}

void Node::findMoveUpdates(int x){
	for(int i = 0; i < 7; i++){
		Moves[x].updates[i] = false;
	}
	if(x > 0){
		if(Moves[x].y > 0){
			Moves[x].updates[3] = true;
			if(Moves[x].y < ((Moves[x-1].y)-1)){
				Moves[x].updates[0] = true;
				Moves[x].updates[1] = true;
				Moves[x].updates[2] = true;
			}
			else if(Moves[x].y < Moves[x-1].y){
				Moves[x].updates[1] = true;
				Moves[x].updates[2] = true;
			}
			else if((Moves[x].y) == (Moves[x-1].y))
				Moves[x].updates[2] = true;
		}
		else if(Moves[x].y == 0){
			if(Moves[x-1].y >= 2){
				Moves[x].updates[0] = true;
				Moves[x].updates[1] = true;
			}
			else if(Moves[x-1].y == 1){
				Moves[x].updates[1] = true;
			}
		}
	}
	if(x < (width-1)){
		if(Moves[x].y > 0){
			Moves[x].updates[3] = true;
			if(Moves[x].y < ((Moves[x+1].y)-1)){
				Moves[x].updates[6] = true;
				Moves[x].updates[5] = true;
				Moves[x].updates[4] = true;
			}
			else if(Moves[x].y < Moves[x+1].y){
				Moves[x].updates[5] = true;
				Moves[x].updates[4] = true;
			}
			else if((Moves[x].y) == (Moves[x+1].y))
				Moves[x].updates[4] = true;
		}
		else if(Moves[x].y == 0){
			if(Moves[x+1].y >= 2){
				Moves[x].updates[6] = true;
				Moves[x].updates[5] = true;
			}
			else if(Moves[x+1].y == 1){
				Moves[x].updates[5] = true;
			}
		}
	}
}

//Updates move x to be one chip higher
void Node::addMove(int x){
	if((x >= 0)&&(x < width)){
		Move m;
		//Get y value
		Moves[x].y++;
		if(Moves[x].y < height)
			Moves[x].useable = true;
		else
			Moves[x].useable = false;
		//Determines where there are chips around it
		this->findMoveUpdates(x);
		//Calculates value of move
		this->findMoveValue(x);			//JC - Where is 'y' declared and or created?  Also, if x<0||x>|Moves|, this will crash.
		//Updates any neighbor moves
		if(x > 0){
			this->findMoveValue(x-1);
			this->findMoveUpdates(x-1);
		}
		if(x < (width-1)){
			this->findMoveValue(x+1);
			this->findMoveUpdates(x+1);
		}
	}
}

//Returns a value based on the number of chains leading into the node
void Node::findMoveValue(int x){
	int y = Moves[x].y;
	if(((x < 0)||(x > width))||((y < 0)||(y > height))){
		Moves[x].value = -1000;
	}
	else{
		int value = 0;
		//Search all of the program's chains
		Chain * p = FriendlyChains;
		while(p != NULL){
			//Add one to value if chain leads into move
			if((p->SPPoI[0] == x) && (p->SPPoI[1] == y))
				value++;
			else if((p->EPPoI[0] == x) && (p->EPPoI[1] == y))
				value++;
			p = p->pchain;
		}
		//Search all the enemies chains
		p = EnemyChains;
		while(p != NULL){
			//Add one to value if chain leads into move
			if((p->SPPoI[0] == x) && (p->SPPoI[1] == y)){
				value++;
			}
			else if((p->EPPoI[0] == x) && (p->EPPoI[1] == y)){
				value++;
			}
			p = p->pchain;
		}

		Moves[x].value = value;
	}
}

void Node::printState(){
	cout << "Value: " << nodeValue << " Win: " << iWin << "\n";
	Chain *p1;
	Chip *p2;
	for(int i = 0; i < width; i++){
		cout << "Move" << i << ": " << Moves[i].y << " " << Moves[i].value << " " << Moves[i].useable << " ";
		for(int j = 0; j < 7; j++){
			cout << Moves[i].updates[j];
		}
		cout << "\n";
	}
	p1 = FriendlyChains;
	cout << "Friendly Chains:\n";
	while(p1 != NULL){
		cout << "Length: " << p1->length << " Direction: " << p1->direction << " SP: " << p1->SP[0] << "," << p1->SP[1] << " EP: " << p1->EP[0] << "," << p1->EP[1] << " PoI: ";
		if(p1->SPPoI[0] != -1000)
			cout << p1->SPPoI[0] << ",";
		else
			cout << "B,";
		if(p1->SPPoI[1] != -1000)
			cout << p1->SPPoI[1] << " ";
		else
			cout << "B ";
		if(p1->EPPoI[0] != -1000)
			cout << p1->EPPoI[0] << ",";
		else
			cout << "B,";
		if(p1->EPPoI[1] != -1000)
			cout << p1->EPPoI[1] << "\n";
		else
			cout << "B\n";
		p1 = p1->pchain;
	}

	p1 = EnemyChains;
	cout << "Enemy Chains:\n";
	while(p1 != NULL){
		cout << "Length: " << p1->length << " Direction: " << p1->direction << " SP: " << p1->SP[0] << "," << p1->SP[1] << " EP: " << p1->EP[0] << "," << p1->EP[1] << " PoI: ";
		if(p1->SPPoI[0] != -1000)
			cout << p1->SPPoI[0] << ",";
		else
			cout << "B,";
		if(p1->SPPoI[1] != -1000)
			cout << p1->SPPoI[1] << " ";
		else
			cout << "B ";
		if(p1->EPPoI[0] != -1000)
			cout << p1->EPPoI[0] << ",";
		else
			cout << "B,";
		if(p1->EPPoI[1] != -1000)
			cout << p1->EPPoI[1] << "\n";
		else
			cout << "B\n";
		p1 = p1->pchain;
	}

	p2 = FriendlyAll;
	cout << "Friendly Chips:\n";
	while(p2 != NULL){
		cout << p2->x << "," << p2->y << "\n";
		p2 = p2->pchip;
	}
	p2 = EnemyAll;
	cout << "Enemy Chips:\n";
	while(p2 != NULL){
		cout << p2->x << "," << p2->y << "\n";
		p2 = p2->pchip;
	}
}

//JC - Include a sample usage.


#endif /* NODE_H_ */
