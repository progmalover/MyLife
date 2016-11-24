// Homepage: http://weibo.com/stormsnow
// Copyright (c) 2016, gangyewei <gangyewei@aliyun.com>.

// For thread race, we have to devide algorithm and data
//
// Use like this:
// HashMap<UserClass> map;
// map.insert(HashMap<UserClass> &map, );
// map.remove
// map.replace
// map.find
// for iter in map

name space mylife_namespace {

#define HASHMAP_BUCKET_LENGTH 4
#define HASHMAP_LENGTH

// temprary, we use int as hash key

template <class T>
class HashMap {
public:
    get();
    set();
    remove();
private:
    std::vector<int, void*> map;
};



}