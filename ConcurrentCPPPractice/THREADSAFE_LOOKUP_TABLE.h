//  线程安全的查询表
//  THREADSAFE_LOOKUP_TABLE.h
//  ConcurrentCPPPractice
//
//  Created by travis on 2019/11/20.
//  Copyright © 2019 travis. All rights reserved.
//

#ifndef THREADSAFE_LOOKUP_TABLE_h
#define THREADSAFE_LOOKUP_TABLE_h
#include<list>
#include <boost/thread/thread.hpp>


template<typename Key,typename Value,typename Hash = std::hash<Key>>
class threadsafe_lookup_table{
private:
    typedef std::pair<Key,Value> bucket_value;
    typedef std::list<bucket_value> bucket_data;
    typedef typename bucket_data::iterator bucket_iterator;
    
    //桶的实例类
    class bucket_type{
    private:
        //存放数据的list
        bucket_data data;
        mutable boost::shared_mutex mutex;  //每一个桶都有一个shared_mutex来允许并发访问
        
        //确定数据是否在桶中
        bucket_iterator find_entry_for(Key const& key) const{ //2
            return std::find_if(data.begin(), data.end(), [&](bucket_value const& item){return item.first==key;});
        }
        
    public:
        //获取桶中对应key的value值
        //若寻找的key不在data中则返回一个默认值
        Value value_for(Key const& key,Value const& default_value) const{
            boost::shared_lock<boost::shared_mutex> lock(mutex); //共享只读所有权
            bucket_iterator const found_entry = find_entry_for(key);
            return (found_entry==data.end())?default_value:found_entry->second;
        }
        
        //更新或添加映射关系
        void add_or_update_mapping(Key const& key,Value const& value){
            std::unique_lock<boost::shared_mutex> lock(mutex); //获取唯一读写权
            bucket_iterator const found_entry = find_entry_for(key);
            if(found_entry==data.end()){ //添加mapping
                data.push_back(bucket_value(key,value));
            }
            else{
                //更新mapping
                found_entry->second = value;
            }
        }
        
        //移除mapping
        void remove_mappint(Key const& key){
            std::unique_lock<boost::shared_mutex> lock(mutex); //获取唯一写权
            bucket_iterator const found_entry = find_entry_for(key);
            if(found_entry!=data.end()){
                data.erase(found_entry);
            }
        }
    };
    
    std::vector<std::unique_ptr<bucket_type>> buckets; // 保存桶
    Hash hasher;
    
    bucket_type& get_bucket(Key const& key) const{ //无锁调用 因为桶的数量是固定的
        std::size_t const bucket_index = hasher(key)%buckets.size();
        return *buckets[bucket_index];
    }
    
public:
    typedef Key key_type;
    typedef Value mapped_type;
    
    typedef Hash hash_type;
    
    //constructor
    //hash表在有质数个桶的时候工作效率最高
    threadsafe_lookup_table(unsigned num_buckets = 19,Hash const& hasher_ = Hash()):buckets(num_buckets),hasher(hasher_){
        for(unsigned i = 0;i<num_buckets;++i){
            buckets[i].reset(new bucket_type);
        }
    }
    
    threadsafe_lookup_table( threadsafe_lookup_table const& other) = delete;
    threadsafe_lookup_table& operator=(threadsafe_lookup_table const& other) = delete;
    
    //封装具体方法
    Value value_for(Key const& key,Value const& default_value = Value()) const{
        return get_bucket(key).value_for(key,default_value);//无锁调用
    }
    
    void add_or_update_mapping(Key const& key,Value const& value){
        get_bucket(key).add_or_update_mappint(key,value);//无锁调用
    }
    
    void remove_mapping(Key const& key){
        get_bucket(key).remove_mapping(key); //无锁调用
    }
    
    //获取整个threadsafe_lookup_table作为一个std::map<>
    //需要获取当前状态快照，此时要求锁住整个容器，需要锁住所有的桶
    //为避免出现死锁的情况，需要每次以相同的顺序进行上锁
    std::map<Key,Value> get_map() const{
        std::vector<std::unique_lock<boost::shared_mutex>> locks;
        for(unsigned i = 0;i<buckets.size();++i){ //按顺序上锁
            locks.push_back(std::unique_lock<boost::shared_mutex>(buckets[i].mutex));
        }
        
        std::map<Key,Value> res;
        for(unsigned i = 0;i<buckets.size();++i){
            for(bucket_iterator it = buckets[i].data.begin();it!=buckets.data.end();++it){
                res.insert(*it);
            }
        }
        return res;
    }
    
};

#endif /* THREADSAFE_LOOKUP_TABLE_h */
