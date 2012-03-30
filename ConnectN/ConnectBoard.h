#pragma once
#include <stdio.h>
#include <stdlib.h>

///Property MACRO
#define PROPERTY(NAME,VARIABLE,TYPE)	void put##NAME(TYPE value){VARIABLE = value;}\
										TYPE get##NAME(){return VARIABLE;}\
										__declspec(property(get=get##NAME,put=put##NAME)) TYPE VARIABLE;



/**	Connect-N Board
	@descrip
		Simple class to handle tracking the connect-N board.
		Tokens are added by specifying a column to use,
		the token is then "dropped" to the lowest position
		in that column like in the actual connect-N game.

		Attempting to place tokens in full columns or outside
		the range of the board will result in error
*/
class ConnectBoard
{
	/** Node struct
	@descrip
		Nodes are the individual positions on a connect-N board.
		They maintain a "used" state and a "me" state.  The "me"
		state refers to whether this node was created by us or
		the opposing player.
	*/
	struct Node {
		bool is_me;				//Player is me
		bool used;				//Node is used
		bool bTraversed;		//Node has been traversed

		Node *parent[9];		//3x3 parent lookup kernel centered on this node
		Node *child[9];			//3x3 child loopup kernel centered on this node

		int nodeID[9];			//3x3 surrounding node ID lookup table (Lists all neighbors around this node)

		int id;					//Node ID
		int r,c;				//Row,Column
	};
public:
	

	ConnectBoard(void);
	~ConnectBoard(void);

	//Add a new move/token to the board at 'column' for player 'is_me'
	bool AddMove(const int &column, const bool& is_me);

	//Remove a move/token from the board at 'column' for player 'is_me'
	bool RemoveMove(const int &column, const bool& is_me);

	//Clear the board of all data, frees arrays too 
	void Clear();

	//Returns the total number of columns on this board.
	int ColumnCount(const int &column) const;

	//(DEBUG ONLY): Test for orphaned children, exit if one is found.
	void DbgExitOnOrphan() const;

	//Display the board on the command line
	void Display(const bool &verbose=false);

	//Returns if column is valid, meaning it is within the board's bounds
	bool ValidColumn(const int &column) const;

	//Returns true iff 0 <= node < width * height
	bool ValidNode(const int &node) const;

	//Returns true iff 0 <= row < height AND 0 <= col < width
	bool ValidNode(const int &row, const int &col) const;

	
	
	//
	// Accessor(s)
	//
	const int& get_width()const{return width_;}
	void set_width(const int& width){width_ = width; ResizeBoard();}

	const int& get_height()const{return height_;}
	void set_height(const int& height){height_ = height; ResizeBoard();}

	const int& get_n()const{return n_;}
	void set_n(const int& n){n_ = n;}

	//Convert 2-D coordinates to a 1-D index in to the node array
	int GetNodeIndex(const int &row, const int &col) const;

	//Get pointer to raw data
	Node *GetDataPtr() const {return data_;}

	//Get node from node ID
	Node *GetNode(const int& nodeId)const{if(!ValidNode(nodeId))return NULL; return &data_[nodeId];}

	//Returns the "value" of the board for player 'is_me'.
	int GetValue(bool is_me,bool bVerbose=false);


	//
	// Operator overload(s)
	//
	Node* operator()(const int row, const int col) const;

protected:

	//Resize the board if width_ or height_ ever change.
	void ResizeBoard();

private:

	//Raw board data, stored as 1D array[width_*height_]
	Node *data_;

	//Max allowed length of a connected chain
	int connectSize_;

	//Size(number of slots) for this board (width_*height_)
	int size_;

	//Actual board width (number of columns)
	int width_;

	//Actual board height(number of rows)
	int height_;

	//The 'N' of Connect-N
	int n_;

	//Shared pool for storing node traversal state (intended to be shared by node structs but not currently doing so.)
	bool *traversalCache_;

	//Connection array, stores the total number of chains with length n at n_Connect_[n],.
	int *n_Connect_;
};
