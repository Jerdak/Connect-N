#include "ConnectBoard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ConnectBoard::ConnectBoard(void)
	:	data_(NULL),
		width_(0),
		height_(0),
		n_Connect_(NULL)
{
}

ConnectBoard::~ConnectBoard(void)
{
	Clear();
}

///Display board (debug only)
void ConnectBoard::Display(const bool& verbose){
	for(int h = height_-1; h >= 0; h--){
		fprintf(stderr,"\n");
		for(int w = 0; w < width_; w++){
			int node = (h*width_) + w;
			if(data_[node].used && data_[node].is_me)fprintf(stderr,"1 ");
			else if(data_[node].used && !data_[node].is_me)fprintf(stderr,"2 ");
			else fprintf(stderr,"0 ");
		}
	}

	//Don't display extra information.
	if(!verbose)return;

	fprintf(stderr,"\n------column count-------\n");
	for(int w = 0; w < width_; w++){
		fprintf(stderr,"%d ",ColumnCount(w));
	}
	int value = GetValue(true);
	fprintf(stderr,"\n------n connections(me): %d-------\n",value);
	for(int w = 0; w < connectSize_; w++){
		fprintf(stderr,"[%d]%d ",w,n_Connect_[w]);
	}

	value = GetValue(false);
	fprintf(stderr,"\n------n connections(them): %d-------\n",value);

	for(int w = 0; w < connectSize_; w++){
		fprintf(stderr,"[%d]%d ",w,n_Connect_[w]);
	}
	fprintf(stderr,"\n");
}

void ConnectBoard::Clear(){
	if(data_){delete [] data_; data_ = NULL;}
	if(n_Connect_){delete [] n_Connect_; n_Connect_ = NULL;}
}

int ConnectBoard::ColumnCount(const int &column) const{
	if(data_ == NULL){
		fprintf(stderr,"Cannot get column count for board, data_ is NULL.");
		return -1;
	}
	int ct = 0;
	for(int i = column; i < size_; i+=width_){
		if(data_[i].used)ct++;
		else break;
	}
	return ct;
}

///Add move to board
bool ConnectBoard::AddMove(const int &column, const bool& is_me){
	
	if(data_ == NULL){
		fprintf(stderr,"Cannot add move to board, data_ is NULL.");
		return false;
	}
	if(!ValidColumn(column))return false;
	
	int colCt = ColumnCount(column);
	if(colCt == height_){
		fprintf(stderr,"Cannot add more tokens to column %d, height full",column);
		return false;
	}
	int node = GetNodeIndex(colCt,column);
	if(!ValidNode(node)){
		fprintf(stderr,"Cannot add node to board %d",column);
		return false;
	}
	
	data_[node].used = true;
	data_[node].is_me = is_me;

	/*
		Using the lookup kernel noted below we make the assumption that new moves added to the board
		have possible parents where there are 1's and possible child where there are 0's.

		The 'if' statement below (in the 'for' loop) could be collapsed to a single 
		statement but it serves readability by leaving both outcomes explicitly written.
	*/
	{
		//Lookup kernel, tells new node what parents to look for first, then what child.
		int reverseKernel[] = {1, 1, 0,		//		| 1 0 0 |
							   1,-1, 0,		// ==>  | 1   0 |
							   1, 0, 0};	//		| 1 1 0 |

		for(int r = -1; r < 2; r++){
			for(int c = -1; c < 2; c++){
				int rk = (r+1)*3 + (c + 1);
				int rki = 8 - rk;
				if(reverseKernel[rk]==1){
					int parentNode = GetNodeIndex(colCt + r,column + c);
					if(ValidNode(parentNode) && data_[parentNode].is_me == is_me && data_[parentNode].used){
					//	fprintf(stderr,"Adding parentnode[%d] to child[%d]\n",parentNode,node);
						data_[node].parent[rk] = &data_[parentNode];
						data_[parentNode].child[rki] = &data_[node];
					}
				} else if(reverseKernel[rk]==0){
					int childNode = GetNodeIndex(colCt + r,column + c);
					if(ValidNode(childNode) && data_[childNode].is_me == is_me && data_[childNode].used){
					//	fprintf(stderr,"Adding childnode[%d] to parent[%d]\n",childNode,node);
						data_[node].child[rk] = &data_[childNode];
						data_[childNode].parent[rki] = &data_[node];
					}
				}
			}
		}
	}
	return true;
}
// Search for orphaned children by looking at used nodes.
// If a node has been used, check if it has children that
// are listed as not being used.  This indicates that the child
// was removed from the board without telling its parents.
void ConnectBoard::DbgExitOnOrphan() const{
	for(int n = 0; n < size_; n++){
		for(int f = 0; f < 9; f++){
			if(data_[n].used){
				if(data_[n].parent[f] == NULL){	
					ConnectBoard::Node *child = data_[n].child[8-f];
					if(child != NULL && child->used == false){
						fprintf(stderr,"Orphaned child found under %d, Child[%d] Direction: %d. CHild?: %d \n",n,child->id,8-f,child->used);
						exit(1);
					} 
				} 
			} 
		}
	}
}

// Remove a single move from the board.
bool ConnectBoard::RemoveMove(const int &column, const bool& is_me){
	if(data_ == NULL){
		fprintf(stderr,"Cannot remove move to board, data_ is NULL.");
		return false;
	}
	if(!ValidColumn(column))return false;

	int colCt = ColumnCount(column)-1;
	int node = GetNodeIndex(colCt,column);
	if(!ValidNode(node))return false;


	if(!data_[node].used){
		fprintf(stderr,"Cannot remove token from column %d, column is empty",column);
		return false;
	}
	if(data_[node].is_me != is_me){
		fprintf(stderr,"Cannot remove token from column %d, users do not match",column);
		return false;
	}
	data_[node].used = false;
	
	//Orphan this node's child and remove it from its parents.
	{
		for(int f = 0; f < 9; f++){
			int fi = 8-f;
			if(data_[node].parent[f] != NULL){
				data_[node].parent[f]->child[fi] = NULL;
			}
			if(data_[node].child[f] != NULL){
				data_[node].child[f]->parent[fi] = NULL;
			}
			data_[node].child[f] = NULL;
			data_[node].parent[f] = NULL;
		}
	}
	return true;
}

// If width or height change, modify the board accordingly.
void ConnectBoard::ResizeBoard(){
	Clear();	//clear the board before checking if width and height are valid.  if one of them are wrong, the whole board is invalid.
	
	if(width_ * height_ == 0)return;

	//1-D size
	size_ = width_ * height_;
	
	//1-D board slot storage
	data_ = new ConnectBoard::Node[size_];

	{	//Initialize traversal cache and nodes(will possible require traversal cache)
		for(int row = 0; row < height_; row++){
			for(int col = 0; col < width_; col++){
				int i = GetNodeIndex(row,col);
				data_[i].used = false;
				data_[i].id = i;
				data_[i].r = row;
				data_[i].c = col;

				for(int r = -1; r < 2; r++){
					for(int c = -1; c < 2; c++){
						int rk = (r+1)*3 + (c + 1);
						int nodeID = GetNodeIndex(row + r,col + c);
						data_[i].nodeID[rk] = nodeID;
						data_[i].parent[rk] = data_[i].child[rk] = NULL;
					
					}
				}
	
			}
		}
	}
	
	{	//Initial connection array
		connectSize_ = (width_ > height_)?width_+1:height_+1;
		n_Connect_ = new int[connectSize_];
		memset(n_Connect_,0,sizeof(int) * connectSize_);
	}
}

//Convert 2-D coordinates to a 1-D index
int ConnectBoard::GetNodeIndex(const int &row, const int &col) const{
	if(row < 0 || row >= height_)return -1;
	if(col < 0 || col >= width_) return -1;

	return col + (width_ * row);
}

///Check if column is within range
bool ConnectBoard::ValidColumn(const int &column) const{
	if(column >= width_ || column < 0){
		fprintf(stderr,"Column %d out of range %d -> %d\n",column,0,width_-1);
		return false;
	}
	return true;
}

///Check if node index is in range.
bool ConnectBoard::ValidNode(const int &node) const{
	if(node >= size_ || node < 0){
		//fprintf(stderr,"Node %d out of range %d -> %d\n",node,0,size_-1);
		return false;
	}
	return true;
}

///Check if node index (by row and column) is in range.
bool ConnectBoard::ValidNode(const int &row, const int &col) const{
	if(row<0 || row >= height_ || col<0 || col >= width_)return false;
	return ValidNode(GetNodeIndex(row,col));
}

///Direct node access (TODO:  Consider dropping)
ConnectBoard::Node* ConnectBoard::operator()(const int row, const int col) const{
	if(!ValidNode(row,col))return NULL;
	return &data_[GetNodeIndex(row,col)];
}

int ConnectBoard::GetValue(bool is_me,bool bVerbose){
	memset(n_Connect_,0,sizeof(int) * connectSize_);

	for(int n = 0; n < size_; n++){
		if(data_[n].used && data_[n].is_me == is_me){	
			for(int f = 0; f < 4; f++){
				int fi = 8 - f;
				int nValidSpace = 0;
				int nSpace = 0;
				int nConnect = 1;
				int ct = 1 ;
				int direction = fi;

				//nodes without parents are 'roots', start search here
				if(data_[n].parent[f] == NULL && f != 4){
					if(bVerbose)fprintf(stderr,"Looking at possible parent[%d].  HasParent: %d\n",n,(data_[n].parent[f]==NULL)?0:1);
					ConnectBoard::Node *child = GetNode(data_[n].nodeID[direction]);
					
					while(child && ct<n_){
						if(child->used && child->is_me==is_me){
							if(bVerbose)fprintf(stderr,"  - Child: %d\n",child->id);
							nConnect+=1;
						}
						if(child->used && child->is_me!=is_me){
							if(bVerbose)fprintf(stderr,"  - Opposing player %d\n",child->id);
							break;
						}
						if(!child->used){
							if(bVerbose)fprintf(stderr,"  - Space: %d\n",child->id);
							break;
						}
						child = GetNode(child->nodeID[direction]);		
					} 
					if(bVerbose)fprintf(stderr,"  - nConnect:%d\n",nConnect);
					if(nConnect>=n_)return 1000;	//Return a large value to indicate this board configuration is a win

					n_Connect_[nConnect] += 1;
				}
			}
		}
	}
	int total = 0;//n_Connect_[n_-1] * 30 + n_Connect_[n_-2] * 5 + n_Connect_[n_-3];
	int coeff = 30;
	for(int i = 1; i <n_-1; i++){
		total += n_Connect_[n_-i] * coeff;
		coeff -= 15;
		if(coeff<=1)coeff = 1;
	}
	return total;
}