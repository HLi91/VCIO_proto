#include "HashManager.h"

HashManager::HashManager(FixSlab* first_slab) {
	first_slab_ = first_slab;
	hash_size_ = first_slab_->GetHashTableRegion(hash_);
}
int HashManager::Insert(string key, int val) {
	return Insert(HashFunction(key), val);
}
/*assume no collision*/
int HashManager::Insert(int hashed_key, int val) {
	hash_[hashed_key].key_ = hashed_key;
	hash_[hashed_key].value_ = val;
	return 0;
}
/*a fake hash function, assume the input is always a string of int*/
int HashManager::HashFunction(string key) {
	return atoi(key.c_str());
}
int HashManager::Erase(string key) {
	int hashed_key = HashFunction(key);
	hash_[hashed_key].key_ = 0;
}
int HashManager::find(string key) {
	int hashed_key = HashFunction(key);
	if (hash_[hashed_key].key_ != 0) return 1;
	return 0;
}
int HashManager::size() {
	return 0;
}
int& HashManager::at(string key) {
	int hashed_key = HashFunction(key);
	hash_[hashed_key].key_ = hashed_key;
	return hash_[hashed_key].value_;
}
int& HashManager::operator[](string key) {
	return at(key);
}