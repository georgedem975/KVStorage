#include "kvs.hpp"
#pragma once

namespace kvs {
    template<typename Clock>
    inline KVStorage<Clock>::KVStorage(std::span<std::tuple<std::string, std::string, uint32_t>> entries, Clock clock) : _clock(clock) {
        KVS_LOG("[kvs::KVStorage] constructor");
        const auto now = _clock.now();
        for (const auto& [key, value, ttl] : entries) {
            _map[key] = Value{ value, (ttl == 0) ? std::nullopt : std::optional<int64_t>(now + ttl) };
        }
    }

    template<typename Clock>
    inline KVStorage<Clock>::~KVStorage() {
        KVS_LOG("[kvs::KVStorage] destructor");
    }

    template<typename Clock>
    inline void KVStorage<Clock>::set(std::string key, std::string value, uint32_t ttl) {
        KVS_LOG("[kvs::KVStorage] method: set; key: " << key << "; value: " << value << "; ttl: " << ttl);
        _map[key] = Value{ value, (ttl == 0) ? std::nullopt : std::optional<int64_t>(_clock.now() + ttl) };
    }

    template<typename Clock>
    inline bool KVStorage<Clock>::remove(std::string_view key) {
        KVS_LOG("[kvs::KVStorage] method: remove; key: " << key);
        auto it = _map.find(key);
        if (it != _map.end()) {
            KVS_LOG("[kvs::KVStorage] method: remove; message: value { "
                << it->second.value
                << "; " << (it->second.timeExpired.has_value() ? std::to_string(it->second.timeExpired.value()) : "inf")
                << " } found");
            _map.erase(std::string(key));
            return true;
        }
        KVS_LOG("[kvs::KVStorage] method: remove; message: value not found by key");
        return false;
    }

    template<typename Clock>
    inline std::optional<std::string> KVStorage<Clock>::get(std::string_view key) const {
        KVS_LOG("[kvs::KVStorage] method: get; key: " << key);
        auto it = _map.find(key);
        if (it == _map.end()) {
            KVS_LOG("[kvs::KVStorage] method: get; message: value not found by key");
            return std::nullopt;
        }

        const auto& value = it->second;

        if (value.timeExpired.has_value() && value.timeExpired.value() < _clock.now()) {
            KVS_LOG("[kvs::KVStorage] method: get; message: the value storage time has expired");
            return std::nullopt;
        }

        KVS_LOG("[kvs::KVStorage] method: get; message: value { " 
            << value.value << "; " 
            << (it->second.timeExpired.has_value() ? std::to_string(it->second.timeExpired.value()) : "inf") 
            << " } found");

        return value.value;
    }

    template<typename Clock>
    inline std::vector<std::pair<std::string, std::string>> KVStorage<Clock>::getManySorted(std::string_view key, uint32_t count) const {
        KVS_LOG("[kvs::KVStorage] method: getManySorted; key: " << key << "; count: " << count);

        std::vector<std::string> keys;
        keys.reserve(_map.size());
        
        const auto now = _clock.now();
        for (const auto& [k, v] : _map) {
            if (v.timeExpired.has_value() && v.timeExpired.value() < now) continue;
            keys.push_back(k);
        }

        std::sort(keys.begin(), keys.end());
        auto it = std::upper_bound(keys.begin(), keys.end(), key);
        
        std::vector<std::pair<std::string, std::string>> answear;
        while (it != keys.end() && answear.size() < count) {
            KVS_LOG("[kvs::KVStorage] method: getManySorted; message: append value { " << *it << "; " << _map.at(*it).value << " } to answear");
            answear.push_back({ *it, _map.at(*it).value });
            ++it;
        }
        return answear;
    }

    template<typename Clock>
    inline std::optional<std::pair<std::string, std::string>> KVStorage<Clock>::removeOneExpiredEntry() {
        KVS_LOG("[kvs::KVStorage] method: removeOneExpired");
        const auto now = _clock.now();
        for (const auto& [k, v] : _map) {
            if (v.timeExpired.has_value() && v.timeExpired.value() < now) {
                std::optional<std::pair<std::string, std::string>> result({ k, v.value });
                KVS_LOG("[kvs:KVStorage] method: removeOneExpired; message: found expired, key: " << k);
                this->remove(k);
                return result;
            }
        }

        KVS_LOG("[kvs::KVStorage] method: removeOneExpired; message: no expired records found");
        
        return std::nullopt;
    }
}