#ifndef PAS_DATA_H
#define PAS_DATA_H
#ifndef PAS_STD_DATA
#include <vector>
#include <map>
#include <string>
#include <utility>
/*
template <class T>
using Vector = std::vector<T>;
*/

using hash_type = unsigned;

template <class T>
class Vector{
public:
  using size_type = unsigned;
  using iterator = T*;
private:
  size_type vsize;
  size_type vcapacity;
  char* vdata;
public:
  Vector():vsize(0),vcapacity(1),vdata(new char[sizeof(T)]){}
  
  Vector(size_type capacityr):vsize(0),vcapacity(capacityr),vdata(new char[sizeof(T)*capacityr]){}
  
  Vector(const Vector<T>& other):vsize(other.vsize), vcapacity(other.vcapacity),
  vdata(new char[sizeof(T) * other.vcapacity]){
    std::copy(other.begin(),other.end(),begin());
  }
  
  
  friend void swap ( Vector<T> &one, Vector<T> &other ){
    using std::swap;
    swap(one.vsize,other.vsize);
    swap(one.vcapacity,other.vcapacity);
    swap(one.vdata,other.vdata);
  }
  
  Vector& operator=(Vector other){
    swap(*this, other);
    return *this;
  }
  
  T& push_back(const T& elem){
    if(vsize == vcapacity){
      realloc();
    }
    T* position = reinterpret_cast<T*> (vdata+vsize*sizeof(T));
    new (position) T(elem);
    ++vsize;
    return *position;
  }
  iterator begin() const{
    return data();
  }
  iterator end() const{
    return data()+vsize;
  }
  
  void reserve(size_type num_elem){
    if(vcapacity >= num_elem)
      return;
    char* newdata = new char[sizeof(T)*num_elem];

    for(size_t i = 0;i < vsize;i++)
      new (newdata+i*sizeof(T)) T(data()[i]);
    delete [] vdata;
    vdata = newdata;
    vcapacity = num_elem;
  }
  T& operator[](unsigned index)
  {
    return data()[index];
  }
  inline T* data() const{
    return reinterpret_cast<T*>(vdata);
  }
  
  void resize(size_type num_elem){
    if(num_elem < vsize){
      for(auto it = data()+num_elem; it != end();it++){
	it->~T();
      }
      vsize = num_elem;
      return;
    }
    reserve(num_elem);
    for(size_t i = vsize; i < num_elem;i++)
      new (vdata+i*sizeof(T)) T();
    vsize = num_elem;
  }
  size_type size() const{
    return vsize;
  }
  size_type capacity() const{
    return vcapacity;
  }
  
  ~Vector(){
    
    for(auto &it : *this){
      it.~T();
    }
    delete [] vdata;
  }
private:
  void realloc(){
    reserve(vcapacity*2);
  }
};

template<class T>
hash_type hash_fun(T);

template <class K, class V>
class Hash{
  using size_type = unsigned;
  static constexpr size_type hash_size = 127;
  
  class Bucket{
  public:
    bool inUse = false;
    Vector<V> payload;
    Vector<K> checker;
  };
  
  class iterator{
    typename Vector<Bucket>::iterator bucket_position;
    typename Vector<Bucket>::iterator end;
  public:
    typename Vector<K>::iterator first;
    typename Vector<V>::iterator second;
    iterator& operator++ (){
      ++second;
      ++first;
      if(second == bucket_position->payload.end()){
	++bucket_position;
	while(!bucket_position->inUse){
	  if(bucket_position == end){
	    return *this;
	  }
	  ++bucket_position;
	}
	second = bucket_position->payload.begin();
	first = bucket_position->checker.begin();
      }
      return *this;
    }
    /*
    iterator operator++ (int){
      iterator other = *this;
      ++(*this);
      return other;
    }*/
    //Aqui se pone horrible la notación
    iterator(const typename Vector<K>::iterator pos0,
	     const typename Vector<V>::iterator pos1,
	     const typename Vector<Bucket>::iterator pos2,
	     const typename Vector<Bucket>::iterator pos3
	    ):
    first(pos0),second(pos1),bucket_position(pos2),end(pos3){}
    iterator(){}
    
    std::pair<K*,V*> operator*(){
      return std::pair<K*,V*>(first,second);
    }
    bool operator!=(const iterator& other){
      if(bucket_position == end && other.bucket_position == end){
	return false;
      }
      return second != other.second;
    }
    bool operator==(const iterator& other){
      return !(*this != other);
    }
  };
  
  
public:
  Vector<Bucket> data;
  Hash():data(hash_size){
    data.resize(hash_size);
  }
  iterator begin() {
    for(auto it = data.begin();it != data.end();it++){
      if(it->inUse){
	return iterator(it->checker.begin(),it->payload.begin(), it, &data[hash_size-1]);
      }
    }
    return end();
  }
  iterator end(){
    auto last = hash_size-1;
    return iterator(data[last].checker.end(),
		    data[last].payload.end(),
		    &data[last],&data[last]);
  }
  V& operator[](const K& index){
    V* ptr_to_elem;
    K* ptr_to_key;
    Bucket &bucket = getB(hash_fun(index));
    if(!atBucket(index,bucket,ptr_to_elem,ptr_to_key)){
      bucket.inUse = true;
      bucket.checker.push_back(index);
      bucket.payload.push_back(V());
      return bucket.payload[bucket.payload.size()-1];
    }
    //TODO: bounds
    return *ptr_to_elem;
  }
  iterator find(const K& index){
    V* ptr_to_elem;
    K* ptr_to_key;
    Bucket &bucket = getB(hash_fun(index));
    if(atBucket(index,bucket,ptr_to_elem,ptr_to_key)){
      return iterator(ptr_to_key,ptr_to_elem,&bucket, &data[hash_size-1]);
    }
    return end();
  }
  //void 
private:
  inline Bucket& getB(hash_type index){
    return data[index%hash_size];
  }
  bool atBucket(const K &key, Bucket &bucket, V* &out, K* &keyout){
    
    size_type i = 0;
    for(const auto& it : bucket.checker){
      if (it == key){
	out = &bucket.payload[i];
	keyout = &bucket.checker[i];
	return true;
      }
      i++;
    }
    out = nullptr;
    return false;
  }
  
};



#else
template <class K, class V>
using Hash = std::map<K,V>;



#endif
#endif