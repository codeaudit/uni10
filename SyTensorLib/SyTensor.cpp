#include "SyTensor.h"
int64_t SyTensor_t::ELEMNUM = 0;
int SyTensor_t::COUNTER = 0;
int64_t SyTensor_t::MAXELEMNUM = 0;
int64_t SyTensor_t::MAXELEMTEN = 0;

SyTensor_t::SyTensor_t(const SyTensor_t& SyT):
	name(SyT.name), status(SyT.status), bonds(SyT.bonds), blocks(SyT.blocks), labels(SyT.labels),
    RBondNum(SyT.RBondNum), elemNum(SyT.elemNum), Qidx(SyT.Qidx),
	RQidx2Off(SyT.RQidx2Off), CQidx2Off(SyT.CQidx2Off){
	cout<<"COPY CONSTRUCTING " << this << endl;
	RQidx2Blk.assign(SyT.RQidx2Blk.size(), NULL);
	for (int i = 0; i < SyT.RQidx2Blk.size(); i++)
		if(SyT.RQidx2Blk[i])
			RQidx2Blk[i] = &(blocks[SyT.RQidx2Blk[i]->qnum]);
	if(SyT.status & INIT){
		elem = (DOUBLE*)malloc(sizeof(DOUBLE) * SyT.elemNum);
		map<Qnum_t,Block_t>::iterator it; 
		for ( it = blocks.begin() ; it != blocks.end(); it++ )
			it->second.elem = &(elem[it->second.offset]);
		ELEMNUM += elemNum;
		if(ELEMNUM > MAXELEMNUM)
			MAXELEMNUM = ELEMNUM;
		if(elemNum > MAXELEMTEN)
			MAXELEMTEN = elemNum;
		memcpy(elem, SyT.elem, sizeof(DOUBLE) * SyT.elemNum);
	}
	COUNTER++;
}	

SyTensor_t& SyTensor_t::operator=(const SyTensor_t& SyT){
	cout<<"ASSING CONSTRUCTING " << this << endl;
	name = SyT.name;
	status = SyT.status;
	bonds = SyT.bonds;
	blocks = SyT.blocks;
	labels = SyT.labels;
	RBondNum = SyT.RBondNum;
	Qidx = SyT.Qidx;
	RQidx2Off = SyT.RQidx2Off;
	CQidx2Off = SyT.CQidx2Off;
	RQidx2Blk.assign(SyT.RQidx2Blk.size(), NULL);
	for (int i = 0; i < SyT.RQidx2Blk.size(); i++)
		if(SyT.RQidx2Blk[i])
			RQidx2Blk[i] = &(blocks[SyT.RQidx2Blk[i]->qnum]);
	if(SyT.status & INIT){
		ELEMNUM -= elemNum;	//free original memory
		elemNum = SyT.elemNum;
		elem = (DOUBLE*)realloc(elem, sizeof(DOUBLE) * SyT.elemNum);
		map<Qnum_t,Block_t>::iterator it; 
		for ( it = blocks.begin() ; it != blocks.end(); it++ )
			it->second.elem = &(elem[it->second.offset]);
		ELEMNUM += elemNum;
		if(ELEMNUM > MAXELEMNUM)
			MAXELEMNUM = ELEMNUM;
		if(elemNum > MAXELEMTEN)
			MAXELEMTEN = elemNum;
		memcpy(elem, SyT.elem, sizeof(DOUBLE) * SyT.elemNum);
	}
	return *this;
}

SyTensor_t::SyTensor_t(vector<Bond_t>& _bonds, vector<int>& _labels, const string& _name): name(_name), status(0), bonds(_bonds){
	assert(_bonds.size() > 0); //No bond in Tensor, Error!
	initSyT();
	addLabel(_labels);
	COUNTER++;
}
SyTensor_t::SyTensor_t(vector<Bond_t>& _bonds, const string& _name): name(_name), status(0), bonds(_bonds){
	assert(_bonds.size() > 0); //No bond in Tensor, Error!
	cout<<"CONSTRUCTING " << this << endl;
	initSyT();
	COUNTER++;
}

SyTensor_t::~SyTensor_t(){
	cout<<"DESTRUCTING " << this << endl;
	if(status & INIT){
		free(elem);
		ELEMNUM -= elemNum;
	}
	COUNTER--;
}

Block_t& SyTensor_t::getBlock(Qnum_t qnum){
	return blocks[qnum];
}

void SyTensor_t::check(){
	cout<<"Existing Tensors: " << COUNTER << endl; 
	cout<<"Allocated Elem: " << ELEMNUM << endl;
	cout<<"Max Allocated Elem: " << MAXELEMNUM << endl;
	cout<<"Max Allocated Elem for a Tensor: " << MAXELEMTEN << endl;
}

void SyTensor_t::addLabel(vector<int>& newLabels){
	set<int> labelS(&(newLabels[0]), &(newLabels[newLabels.size()]));
	assert(bonds.size() == labelS.size());
	labels = newLabels;
	status |= HAVELABEL;
}

void SyTensor_t::reshape(vector<int>& newLabels, int rowBondNum){
	assert(status & INIT);
	assert(status & HAVELABEL);
	assert(labels.size() == newLabels.size());
	int bondNum = bonds.size();
	int rsp_outin[bondNum];	//rsp_outin[2] = 1 means the index "2" of SyTout is the index "1" of SyTin, opposite to the order in TensorLib
	int rsp_inout[bondNum];	//rsp_inout[2] = 1 means the index "2" of SyTin is the index "1" of SyTout, the same as the order in TensorLib
	int cnt = 0;
	for(int i = 0; i < bondNum; i++)
		for(int j = 0; j < bondNum; j++)
			if(labels[i] == newLabels[j]){
				rsp_outin[j] = i;	
				rsp_inout[i] = j;	
				cnt++;
			}
	assert(cnt == newLabels.size());
	bool inorder = true;
	for(int i = 1; i < bondNum; i++)
		if(((rsp_outin[i] + bondNum - i) % bondNum) != rsp_outin[0]){
			inorder = false;
			break;
		}
	if(inorder  && rsp_outin[0] == 0 && RBondNum == rowBondNum);	//do nothing
	else if(inorder  && rsp_outin[0] != 0 && (bondNum - RBondNum) == rowBondNum){
		this->transpose();
		printf("TRANSPOSE!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	}
	else{
		printf("LALALALALALALAL\n");
		vector<Bond_t> outBonds;
		for(int b = 0; b < bonds.size(); b++)
			outBonds.push_back(bonds[rsp_outin[b]]);
		for(int b = 0; b < bonds.size(); b++){
			if(b < rowBondNum){
				if(outBonds[b].type == BD_COL)
					for(int q = 0; q < outBonds[b].Qnums.size(); q++)
						outBonds[b].Qnums[q] = -outBonds[b].Qnums[q];
				outBonds[b].type = BD_ROW;
			}
			else{
				if(outBonds[b].type == BD_ROW)
					for(int q = 0; q < outBonds[b].Qnums.size(); q++)
						outBonds[b].Qnums[q] = -outBonds[b].Qnums[q];
				outBonds[b].type = BD_COL;
			}
		}
		SyTensor_t SyTout(outBonds, "Reshaped " + name);
		SyTout.addLabel(newLabels);
		if(status & HAVEELEM){
			int QcolNum = 1;
			int QcolNum_in = 1;
			for(int b = bondNum - 1; b >= 0; b--)
				if(SyTout.bonds[b].type == BD_COL)
					QcolNum *= SyTout.bonds[b].Qnums.size();
				else
					break;
			for(int b = bondNum - 1; b >= 0; b--)
				if(bonds[b].type == BD_COL)
					QcolNum_in *= bonds[b].Qnums.size();
				else
					break;
			
			vector<int> Qin_acc(bondNum, 1); 
			vector<int> Dupb(bondNum, 0);
			for(int b = bondNum	- 1; b > 0; b--)
				Qin_acc[b - 1] = Qin_acc[b] * bonds[b].Qnums.size();
			int Qoff = 0;	//for SyTout
			int Qoff_in;
			int bend;
			int BcolNum, BcolNum_in;
			int DcolNum, DcolNum_in;
			int boff = 0, boff_in = 0;
			int cnt, cnt_in;
			vector<int> Qidxs(bondNum, 0); 
			vector<int> idxs;
			vector<int> Din_acc(bondNum, 1);	//Degeneracy acc

			while(1){
				if(SyTout.Qidx[Qoff]){
					Qoff_in = 0;
					DcolNum = 1;
					DcolNum_in = 1;
					for(int b = 0; b < bondNum; b++){
						Qoff_in += Qidxs[b] * Qin_acc[rsp_outin[b]];
						Dupb[b] = SyTout.bonds[b].Qdegs[Qidxs[b]];
						if(SyTout.bonds[b].type == BD_COL)
							DcolNum *= SyTout.bonds[b].Qdegs[Qidxs[b]];
						if(bonds[b].type == BD_COL)
							DcolNum_in *= bonds[b].Qdegs[Qidxs[rsp_inout[b]]];
					}
					assert(Qidx[Qoff_in]);
					for(int b = bondNum	- 1; b > 0; b--)
						Din_acc[b - 1] = Din_acc[b] * bonds[b].Qdegs[Qidxs[rsp_inout[b]]];
					BcolNum = (SyTout.RQidx2Blk[Qoff / QcolNum])->Cnum;
					boff = (SyTout.RQidx2Blk[Qoff / QcolNum])->offset + SyTout.RQidx2Off[Qoff / QcolNum] * BcolNum + SyTout.CQidx2Off[Qoff % QcolNum];
					BcolNum_in = (RQidx2Blk[Qoff_in / QcolNum_in])->Cnum;
					boff_in = (RQidx2Blk[Qoff_in / QcolNum_in])->offset + RQidx2Off[Qoff_in / QcolNum_in] * BcolNum_in + CQidx2Off[Qoff_in % QcolNum_in];
					cnt = 0;
					cnt_in = 0;
					idxs.assign(bondNum, 0);
					while(1){
						SyTout.elem[boff + (cnt / DcolNum) * BcolNum + cnt % DcolNum] = elem[boff_in + (cnt_in / DcolNum_in) * BcolNum_in + cnt_in % DcolNum_in];
						cnt++;
						for(bend = bondNum - 1; bend >= 0; bend--){
							idxs[bend]++;
							if(idxs[bend] < Dupb[bend]){
								cnt_in += Din_acc[rsp_outin[bend]];
								break;
							}
							else{
								cnt_in -= Din_acc[rsp_outin[bend]] * (idxs[bend] - 1);
								idxs[bend] = 0;
							}
						}
						if(bend < 0)
							break;
					}
				}
				for(bend = bondNum - 1; bend >= 0; bend--){
					Qidxs[bend]++;
					if(Qidxs[bend] < SyTout.bonds[bend].Qnums.size())
						break;
					else
						Qidxs[bend] = 0;
				}
				Qoff++;
				if(bend < 0)
					break;
			}
			SyTout.status |= HAVEELEM;
		}
		*this = SyTout;
	}
}

void SyTensor_t::transpose(){
	assert(status & INIT);
	int bondNum = bonds.size();
	int rsp_outin[bondNum];	//rsp_outin[2] = 1 means the index "2" of SyTout is the index "1" of SyTin, opposite to the order in TensorLib
	int rbondNum = 0;
	for(int b = 0; b < bondNum; b++)
		if(bonds[b].type == BD_ROW)
			rbondNum++;
		else
			break;
	int cbondNum = bondNum - rbondNum;
	for(int b = 0; b < bondNum; b++)
		if(b < cbondNum)
			rsp_outin[b] = rbondNum + b;
		else
			rsp_outin[b] = b - cbondNum;
	vector<int> outLabels(bondNum, 0);
	vector<Bond_t> outBonds;
	if(status & HAVELABEL){
		for(int b = 0; b < bonds.size(); b++){
			outBonds.push_back(bonds[rsp_outin[b]]);
			outLabels[b] = labels[rsp_outin[b]];
		}
	}
	else
		for(int b = 0; b < bonds.size(); b++)
			outBonds[b] = bonds[rsp_outin[b]];

	for(int b = 0; b < bondNum; b++){
		for(int q = 0; q < outBonds[b].Qnums.size(); q++)
			outBonds[b].Qnums[q] = -outBonds[b].Qnums[q];
		if(b < cbondNum)
			outBonds[b].type = BD_ROW;
		else
			outBonds[b].type = BD_COL;
	}
	SyTensor_t SyTout(outBonds, "Transpoesed " + name);
	if(status & HAVELABEL)
		SyTout.addLabel(outLabels);
	if(status & HAVEELEM){
		map<Qnum_t,Block_t>::iterator it_in; 
		map<Qnum_t,Block_t>::iterator it_out; 
		double* elem_in;
		double* elem_out;
		int Rnum, Cnum;
		for ( it_in = blocks.begin() ; it_in != blocks.end(); it_in++ ){
			it_out = SyTout.blocks.find(-(it_in->first));
			Rnum = it_in->second.Rnum;
			Cnum = it_in->second.Cnum;
			elem_in = it_in->second.elem;
			elem_out = it_out->second.elem;
			for(int i = 0; i < Rnum; i++)
				for(int j = 0; j < Cnum; j++)
					elem_out[j * Rnum + i] = elem_in[i * Cnum + j];
		}   
		SyTout.status |= HAVEELEM;
	}
	*this = SyTout;
}

void SyTensor_t::addRawElem(DOUBLE* rawElem){
	assert((status & INIT));   //If not INIT, CANNOT add elements
	int bondNum = bonds.size();
	vector<int> Qidxs(bondNum, 0); 
	vector<int> idxs(bondNum, 0); 
	vector<int> idxUpb(bondNum, 0); //upper bound of a index of some block
	vector<int> rAcc(bondNum, 1); 
	map<Qnum_t,Block_t>::iterator git;
	map<Qnum_t,int> grp_cnt;
	for (git = blocks.begin(); git != blocks.end(); git++)
		grp_cnt[git->first] = git->second.offset;
	for(int b = bondNum - 1; b > 0; b--)
		rAcc[b - 1] = rAcc[b] * bonds[b].dim;
	int bend = 0;
	int64_t roff = 0;
	int64_t boff = 0;
	int Qoff = 0;
	int QcolNum = 1;
	int BcolNum = 0;
	int DcolNum = 0;	//Degeneracy column number
	int cnt;
	for(int b = bondNum - 1; b >= 0; b--)
		if(bonds[b].type == BD_COL)
			QcolNum *= bonds[b].Qnums.size();
		else
			break;
	while(1){
		if(Qidx[Qoff]){
			roff = 0;
			DcolNum = 1;
			for(int b = 0; b < bondNum; b++){
				idxs[b] = bonds[b].offsets[Qidxs[b]];
				idxUpb[b] = idxs[b] + bonds[b].Qdegs[Qidxs[b]];
				roff += idxs[b] * rAcc[b];
				if(bonds[b].type == BD_COL)
					DcolNum *= bonds[b].Qdegs[Qidxs[b]];
			}

			BcolNum = (RQidx2Blk[Qoff / QcolNum])->Cnum;
			boff = (RQidx2Blk[Qoff / QcolNum])->offset + RQidx2Off[Qoff / QcolNum] * BcolNum + CQidx2Off[Qoff % QcolNum];

			cnt = 0;
			while(1){
				elem[boff + (cnt / DcolNum) * BcolNum + cnt % DcolNum] = rawElem[roff];
				for(bend = bondNum - 1; bend >= 0; bend--){
					idxs[bend]++;
					if(idxs[bend] < idxUpb[bend]){
						roff += rAcc[bend];
						break;
					}
					else{
						roff -= rAcc[bend] * (idxs[bend] - bonds[bend].offsets[Qidxs[bend]] - 1);
						idxs[bend] = bonds[bend].offsets[Qidxs[bend]];
					}
				}
				if(bend < 0)
					break;
				cnt++;
			}
		}   
		for(bend = bondNum - 1; bend >= 0; bend--){
			Qidxs[bend]++;
			if(Qidxs[bend] < bonds[bend].Qnums.size())
				break;
			else
				Qidxs[bend] = 0;
		}
		Qoff++;
		if(bend < 0)
			break;
	}
	status |= HAVEELEM;
}

void SyTensor_t::grouping(){
	blocks.clear();
	int row_bondNum = 0;
	int col_bondNum = 0;
	int row_Qdim = 1;
	int col_Qdim = 1;
	for(int i = 0; i < bonds.size(); i++){
		if(bonds[i].type == BD_ROW){
			row_Qdim *= bonds[i].Qnums.size();
			row_bondNum++;
		}
		else{
			col_Qdim *= bonds[i].Qnums.size();
			col_bondNum++;
		}
	}
	RBondNum = row_bondNum;
	map<Qnum_t,int> row_QnumMdim;
	vector<int> row_offs(row_bondNum, 0);
	map<Qnum_t,vector<int> > row_Qnum2Qidx;
	Qnum_t qnum(0, 0);
	int dim;
	int boff = 0;
	RQidx2Off.assign(row_Qdim, 0);
	CQidx2Off.assign(col_Qdim, 0);
	RQidx2Blk.assign(row_Qdim, NULL);
	Qidx.assign(row_Qdim * col_Qdim, false);
	if(row_bondNum){
		while(1){
			qnum.set();
			dim = 1;
			for(int b = 0; b < row_bondNum; b++){
				qnum = qnum * bonds[b].Qnums[row_offs[b]];
				dim *= bonds[b].Qdegs[row_offs[b]];
			}
			if(row_QnumMdim.find(qnum) != row_QnumMdim.end()){
				RQidx2Off[boff] = row_QnumMdim[qnum];
				row_QnumMdim[qnum] += dim;
			}
			else{
				RQidx2Off[boff] = 0;
				row_QnumMdim[qnum] = dim;
			}
			row_Qnum2Qidx[qnum].push_back(boff);
			boff++;
			int bidx;
			for(bidx = row_bondNum - 1; bidx >= 0; bidx--){
				row_offs[bidx]++;
				if(row_offs[bidx] < bonds[bidx].Qnums.size())
					break;
				else
					row_offs[bidx] = 0;
			}
			if(bidx < 0)	//run over all row_bond offsets
				break;
		}
	}
	else{
		qnum.set();
		row_QnumMdim[qnum] = 1;
		row_Qnum2Qidx[qnum].push_back(0);
	}
	map<Qnum_t,int>::iterator itt;

	map<Qnum_t,int> col_QnumMdim;
	vector<int> col_offs(col_bondNum, 0);
	map<Qnum_t,vector<int> > col_Qnum2Qidx;
	boff = 0;
	if(col_bondNum){
		while(1){
			qnum.set();
			dim = 1;
			for(int b = 0; b < col_bondNum; b++){
				qnum = qnum * bonds[b + row_bondNum].Qnums[col_offs[b]];
				dim *= bonds[b + row_bondNum].Qdegs[col_offs[b]];
			}
			if(row_QnumMdim.find(qnum) != row_QnumMdim.end()){
				if(col_QnumMdim.find(qnum) != col_QnumMdim.end()){
					CQidx2Off[boff] = col_QnumMdim[qnum];
					col_QnumMdim[qnum] += dim;
				}
				else{
					CQidx2Off[boff] = 0;
					col_QnumMdim[qnum] = dim;
				}
				col_Qnum2Qidx[qnum].push_back(boff);
			}
			boff++;
			int bidx;
			for(bidx = col_bondNum - 1; bidx >= 0; bidx--){
				col_offs[bidx]++;
				if(col_offs[bidx] < bonds[bidx + row_bondNum].Qnums.size())
					break;
				else
					col_offs[bidx] = 0;
			}
			if(bidx < 0)	//run over all row_bond offsets
				break;
		}
	}
	else{
		qnum.set();
		if(row_QnumMdim.find(qnum) != row_QnumMdim.end()){
			col_QnumMdim[qnum] = 1;
			col_Qnum2Qidx[qnum].push_back(0);
		}
	}

	map<Qnum_t,int>::iterator it;
	map<Qnum_t,int>::iterator it2;
	Block_t blk;
	int off = 0;
	for ( it2 = col_QnumMdim.begin() ; it2 != col_QnumMdim.end(); it2++ ){
		it = row_QnumMdim.find(it2->first);
		blk.Rnum = it->second;
		blk.Cnum = it2->second;
		blk.qnum = it2->first;
		blk.offset = off;
		off += blk.Rnum * blk.Cnum;
		blocks[it->first] = blk;
		Block_t* blkptr= &(blocks[it->first]);
		vector<int>& tmpRQidx = row_Qnum2Qidx[it->first];
		vector<int>& tmpCQidx = col_Qnum2Qidx[it->first];
		for(int i = 0; i < tmpRQidx.size(); i++){
			RQidx2Blk[tmpRQidx[i]] = blkptr;
			for(int j = 0; j < tmpCQidx.size(); j++)
				Qidx[tmpRQidx[i] * col_Qdim + tmpCQidx[j]] = true;
		}
	}
}

void SyTensor_t::initSyT(){
	grouping();
	assert(blocks.size() > 0); //No block in Tensor, Error!
	Block_t blk = blocks.rbegin()->second;
	elemNum = blk.offset + (blk.Rnum * blk.Cnum);
	elem = (DOUBLE*)malloc(sizeof(DOUBLE) * elemNum);
	map<Qnum_t,Block_t>::iterator it; 
	for ( it = blocks.begin() ; it != blocks.end(); it++ )
		it->second.elem = &(elem[it->second.offset]);

	ELEMNUM += elemNum;
	if(ELEMNUM > MAXELEMNUM)
		MAXELEMNUM = ELEMNUM;
	if(elemNum > MAXELEMTEN)
		MAXELEMTEN = elemNum;
	memset(elem, 0, sizeof(DOUBLE) * elemNum);
	status |= INIT;
}

double SyTensor_t::at(vector<int> idxs)const{
	int bondNum = bonds.size();
	vector<int> Qidxs(bondNum, 0);
	for(int b = 0; b < bondNum; b++){
		for(int q = bonds[b].offsets.size() - 1; q >= 0; q--){
			if(idxs[b] < bonds[b].offsets[q])
				continue;
			Qidxs[b] = q;
			break;
		}
	}

	vector<int> Q_acc(bondNum, 1); 
	for(int b = bondNum	- 1; b > 0; b--)
		Q_acc[b - 1] = Q_acc[b] * bonds[b].Qnums.size();
	int Qoff = 0;
	int DcolNum = 1;	//Degeneracy column number
	for(int b = 0; b < bondNum; b++){
		Qoff += Q_acc[b] * Qidxs[b];
		if(bonds[b].type == BD_COL)
			DcolNum *= bonds[b].Qdegs[Qidxs[b]];
	}
	if(Qidx[Qoff]){
		int QcolNum = 1;
		int BcolNum;
		for(int b = bondNum - 1; b >= 0; b--)
			if(bonds[b].type == BD_COL)
				QcolNum *= bonds[b].Qnums.size();
			else
				break;
		BcolNum = (RQidx2Blk[Qoff / QcolNum])->Cnum;
		int boff = (RQidx2Blk[Qoff / QcolNum])->offset + RQidx2Off[Qoff / QcolNum] * BcolNum + CQidx2Off[Qoff % QcolNum];
		int cnt = 0;
		vector<int> D_acc(bondNum, 1); 
		for(int b = bondNum	- 1; b > 0; b--)
			D_acc[b - 1] = D_acc[b] * bonds[b].Qdegs[Qidxs[b]];
		for(int b = 0; b < bondNum; b++)
			cnt += (idxs[b] - bonds[b].offsets[Qidxs[b]]) * D_acc[b];
		return elem[boff + (cnt / DcolNum) * BcolNum + cnt % DcolNum];
	}
	else{
		return 0.0;
	}
}

void printRawElem(const SyTensor_t& SyT){
	if(SyT.status & HAVEELEM){
		int bondNum = SyT.bonds.size();
		int colNum = 1;
		for(int b = bondNum - 1; b >= 0; b--)
			if(SyT.bonds[b].type == BD_COL)
				colNum *= SyT.bonds[b].dim;
			else
				break;
		vector<Qnum_t> rowQ;
		vector<Qnum_t> colQ;
		int Rnum = SyT.RBondNum;
		int Cnum = bondNum - SyT.RBondNum;
		vector<int> idxs(Rnum, 0);
		vector<int> qidxs(Rnum, 0);
		int bend;
		while(1){
			Qnum_t qnum;
			for(int b = 0; b < Rnum; b++)
				qnum = qnum * SyT.bonds[b].Qnums[qidxs[b]];
			rowQ.push_back(qnum);
			for(bend = Rnum - 1; bend >= 0; bend--){
				idxs[bend]++;
				if(idxs[bend] < SyT.bonds[bend].offsets[qidxs[bend]] + SyT.bonds[bend].Qdegs[qidxs[bend]])
					break;
				else{
					qidxs[bend]++;
					if(qidxs[bend] < SyT.bonds[bend].Qnums.size())
						break;
					else{
						qidxs[bend] = 0;
						idxs[bend] = 0;
					}
				}
			}
			if(bend < 0)
				break;
		}
		idxs.assign(Cnum, 0);
		qidxs.assign(Cnum, 0);
		while(1){
			Qnum_t qnum;
			for(int b = 0; b < Cnum; b++)
				qnum = qnum * SyT.bonds[Rnum + b].Qnums[qidxs[b]];
			colQ.push_back(qnum);
			for(bend = Cnum - 1; bend >= 0; bend--){
				idxs[bend]++;
				if(idxs[bend] < SyT.bonds[Rnum + bend].offsets[qidxs[bend]] + SyT.bonds[Rnum + bend].Qdegs[qidxs[bend]])
					break;
				else{
					qidxs[bend]++;
					if(qidxs[bend] < SyT.bonds[Rnum + bend].Qnums.size())
						break;
					else{
						qidxs[bend] = 0;
						idxs[bend] = 0;
					}
				}
			}
			if(bend < 0)
				break;
		}
		cout<< "     ";
		for(int q = 0; q < colQ.size(); q++)
			cout<< "   " << setw(2) << colQ[q].getU1() << "," << colQ[q].getPrt();
		cout<< endl << setw(5) << "" << setw(colQ.size() * 7 + 2) <<setfill('-')<<"";
		cout<<setfill(' ');
		idxs.assign(bondNum, 0);
		int cnt = 0;
		int r = 0;
		while(1){
			if(cnt % colNum == 0){
				cout<<"\n    |\n" << setw(2) << rowQ[r].getU1() << "," << rowQ[r].getPrt() << "|";
				r++;
			}
			cout<< setw(7) << setprecision(3) << SyT.at(idxs);
			for(bend = bondNum - 1; bend >= 0; bend--){
				idxs[bend]++;
				if(idxs[bend] < SyT.bonds[bend].dim)
					break;
				else
					idxs[bend] = 0;
			}
			cnt++;
			if(bend < 0)
				break;
		}
		cout <<"\n    |\n";
	}
	else{
		printf("NO ELEMENT IN THE TENSOR!!!\n");
	}
}

ostream& operator<< (ostream& os, SyTensor_t& SyT){
	assert(SyT.status & INIT);
	int row = 0;
	int col = 0;
	for(int i = 0; i < SyT.bonds.size(); i++)
		if(SyT.bonds[i].type == BD_ROW)
			row++;
		else
			col++;
	int layer = max(row, col);
	int nmlen = SyT.name.length() + 2;
	int star = 12 + (14 - nmlen) / 2;
	cout<<endl;
	for(int s = 0; s < star; s++)
		cout << "*";
	cout << " " << SyT.name << " ";
	for(int s = 0; s < star; s++)
		cout<<"*";	
	cout << "\n             ____________\n";
	cout << "            |            |\n";
	int llab = 0;
	int rlab = 0;
	char buf[128];
	for(int l = 0; l < layer; l++){
		if(l < row && l < col){
			if(SyT.status & HAVELABEL){
				llab = SyT.labels[l];
				rlab = SyT.labels[row + l];
			}
			else{
				llab = l;
				rlab = row + l;
			}
			sprintf(buf, "    %5d___|%-4d    %4d|___%-5d\n", llab, SyT.bonds[l].dim, SyT.bonds[row + l].dim, rlab);
			cout<<buf;
		}
		else if(l < row){
			if(SyT.status & HAVELABEL)
				llab = SyT.labels[l];
			else
				llab = l;
			sprintf(buf, "    %5d___|%-4d    %4s|\n", llab, SyT.bonds[l].dim, "");
			cout<<buf;
		}
		else if(l < col){
			if(SyT.status & HAVELABEL)
				rlab = SyT.labels[row + l];
			else
				rlab = row + l;
			sprintf(buf, "    %5s   |%4s    %4d|___%-5d\n", "", "", SyT.bonds[row + l].dim, rlab);
			cout << buf;
		}   
		cout << "            |            |   \n";
	}   
	cout << "            |____________|\n";

	cout << "\n================BONDS===============\n";
	for(int b = 0; b < SyT.bonds.size(); b++){
		cout << SyT.bonds[b];
	}   
	cout<<"\n===============BLOCKS===============\n";
	map<Qnum_t,Block_t>::iterator it; 
	int Rnum, Cnum;
	bool printElem = true;
	for ( it = SyT.blocks.begin() ; it != SyT.blocks.end(); it++ ){
		cout << "--- " << it->first << it->second;
		Rnum = it->second.Rnum;
		Cnum = it->second.Cnum;
		cout << " ---\n\n";

		if((SyT.status & HAVEELEM) && printElem){
			for(int r = 0; r < Rnum; r++){
				for(int c = 0; c < Cnum; c++)
					cout<< setw(7) << setprecision(3) << it->second.elem[r * Cnum + c];
				cout << "\n\n";
			}
		}
	}   
	cout << "Total elemNum: "<<SyT.elemNum<<endl;
	cout << "***************** END ****************\n\n";
	return os;
}
/*------------------- SET ELEMENTS -----------------*/
#include <boost/random.hpp>
using namespace boost;

void SyTensor_t::randomize(){
	mt19937 rng(777);
	uniform_01<mt19937> uni01_sampler(rng);
	assert((status & INIT));   //If not INIT, CANNOT add elements
	for(int i = 0; i < elemNum; i++)
		elem[i] = uni01_sampler();
	status |= HAVEELEM;
}

void SyTensor_t::orthoRand(Qnum_t qnum){
	Block_t& block = blocks[qnum];
	orthoRandomize(block.elem, block.Rnum, block.Cnum);
}

void SyTensor_t::orthoRand(){
	map<Qnum_t,Block_t>::iterator it; 
	for ( it = blocks.begin() ; it != blocks.end(); it++ )
		orthoRandomize(it->second.elem, it->second.Rnum, it->second.Cnum);
}

void SyTensor_t::eye(Qnum_t qnum){
	Block_t& block = blocks[qnum];
	myEye(block.elem, block.Rnum, block.Cnum);
}

void SyTensor_t::eye(){
	map<Qnum_t,Block_t>::iterator it; 
	for ( it = blocks.begin() ; it != blocks.end(); it++ )
		myEye(it->second.elem, it->second.Rnum, it->second.Cnum);
}

void SyTensor_t::elemset(Qnum_t qnum, DOUBLE* elem, int64_t elemNum){
	Block_t& block = blocks[qnum];
	memcpy(block.elem, elem, elemNum * sizeof(DOUBLE));
}

void SyTensor_t::bzero(Qnum_t qnum){
	Block_t& block = blocks[qnum];
	memset(block.elem, 0, block.Rnum * block.Cnum * sizeof(DOUBLE));
}

void SyTensor_t::bzero(){
	memset(elem, 0, elemNum * sizeof(DOUBLE));
}
