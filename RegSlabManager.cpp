#include "RegSlabManager.h"


RegularSlabManager::RegularSlabManager(FixSlab * first_slab) {
	first_slab_ = first_slab;
	first_slab_->Lock();
	for (int i = 0; i < first_slab_->GetRegSlabCur(); i++) {
		
		string base(first_slab_->GetName());
		string name = base + "_reg_" + to_string(i);
		if (name.size() > 255) {
			//printf("ERRPR file name too long \n %s\n", name);
			cout << "ERRPR file name too long" << endl << name << endl;
			while (1);
		}
		slab_list_.push_back(RegularSlab());
		slab_list_[i].connect(name.c_str());
		
	}
	first_slab_->Unlock();
}
DataBlock* RegularSlabManager::GetBlock(BlockInfo_t &position) {
	if (position.slab_index > slab_list_.size()) {
		first_slab_->Lock();
		UpdateSlabListLocked();
		first_slab_->Unlock();
		if (position.slab_index > slab_list_.size())
			return NULL;
	}
	else {
		return slab_list_[position.slab_index][position];
	}
}
vector<BlockInfo_t> RegularSlabManager::RequestEmptyBlock(int num, int node_id) {
	first_slab_->Lock();
	UpdateSlabListLocked();

	int max_available = first_slab_->GetMaxBlockSize() - first_slab_->GetCurBlockSize();
	if (first_slab_->GetRegSlabMax() != -1) {
		int max_alloc = (first_slab_->GetRegSlabMax() - first_slab_->GetRegSlabCur())*SLAB_SIZE + max_available;
		if (max_alloc < num) {
			first_slab_->Unlock();
			return vector<BlockInfo_t>();
		}

	}
	while (num > max_available) {
		if (first_slab_->GetRegSlabCur() == first_slab_->GetRegSlabMax()) {
			break;
		}
		AddSlabLocked(SLAB_SIZE);
		max_available += SLAB_SIZE;
	}

	vector<BlockInfo_t> list;
	for (auto & it : slab_list_) {
		it.MetadataLock();
		if (it.header_->in_used_block == it.header_->total_block) {
			it.MetadataUnlock();
			continue;
		}
		vector <BlockInfo_t> ret = it.GetBitMap()->GetBlocks(num);
		for (auto & block : ret) {
			it[block]->inode_id = node_id;
		}
		num -= ret.size();
		list.insert(list.end(), ret.begin(), ret.end());
		it.header_->in_used_block += ret.size();
		it.MetadataUnlock();
		
	}
	first_slab_->SetCurBlockSize(first_slab_->GetCurBlockSize() + list.size());
	first_slab_->Unlock();
	return list;
}

int RegularSlabManager::UpdateSlabListLocked() {
	for (int i = slab_list_.size(); i < first_slab_->GetRegSlabCur(); i++) {
		string base(first_slab_->GetName());
		string name = base + "_reg_" + to_string(i);
		if (name.size() > 255) {
			cout << "ERRPR file name too long" << endl << name << endl;
			while (1);
		}
		slab_list_.push_back(RegularSlab());
		slab_list_[i].connect(name.c_str());

	}
}


int RegularSlabManager::AddSlabLocked(int blocks) {
	slab_list_.push_back(RegularSlab());
	string base(first_slab_->GetName());
	string name = base + "_reg_" + to_string(slab_list_.size() - 1);
	slab_list_[slab_list_.size() - 1].create(name.c_str(), blocks, slab_list_.size() - 1);
	first_slab_->GetRegSlabCur()++;
	first_slab_->SetMaxBlockSize(blocks + first_slab_->GetMaxBlockSize());
}
