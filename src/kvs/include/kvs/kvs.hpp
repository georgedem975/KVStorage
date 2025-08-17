#pragma once

#include <span>
#include <tuple>
#include <stdint.h>
#include <string>
#include <optional>
#include <vector>
#include <unordered_map>
#include <algorithm>

#ifdef KVS_ENABLE_LOGS
    #define KVS_LOG(x) std::cout << x << std::endl
#else
    #define KVS_LOG(x) do {} while(0)
#endif

namespace kvs {
    template <typename Clock>
    class KVStorage {
    public:
        // �������������� ��������� ���������� ���������� �������. ������ span ����� ���� ����� �������.
        // ����� ��������� ���������� ����� (Clock) ��� ����������� ���������� �������� � ������.
        explicit KVStorage(std::span<std::tuple<std::string /*key*/, std::string /*value*/, uint32_t /*ttl*/>> entries, Clock clock = Clock());

        ~KVStorage();

        // ����������� �� ����� key �������� value.
        // ���� ttl == 0, �� ����� ����� ������ - �������������, ����� ������ ������ ��������� ���� ��������� ����� ttl ������.
        // ���������� ��������� ttl ������.
        void set(std::string key, std::string value, uint32_t ttl);

        // ������� ������ �� ����� key.
        // ���������� true, ���� ������ ���� �������. ���� ����� �� ���� �� ��������, �� ������ false.
        bool remove(std::string_view key);

        // ������� �������� �� ����� key. ���� ������� ����� ���, �� ������ std::nullopt.
        std::optional<std::string> get(std::string_view key) const;

        // ���������� ��������� count ������� ������� � key � ������� ������������������ ���������� ������.
        // ������: ("a", "val1"), ("b", "val2"), ("d", "val3"), ("e", "val4")
        // getManySorted("c", 2) -> ("d", "val3"), ("e", "val4")
        std::vector<std::pair<std::string, std::string>> getManySorted(std::string_view key, uint32_t count) const;

        // ������� ��������� ������ �� ��������� � ���������� �. ���� ������� ������, �� ������ std::nullopt.
        // ���� �� ������ �������������������� ��������� �������,�� ����� ������� �����.
        std::optional<std::pair<std::string, std::string>> removeOneExpiredEntry();

    private:
        struct Value { 
            std::string value; 
            std::optional<int64_t> timeExpired; 
        };

        struct Hash { 
            using is_transparent = void;

            size_t operator()(std::string_view s) const {
                return std::hash<std::string_view>{}(s);
            } 
        };

        struct Equal { 
            using is_transparent = void; 
            
            bool operator()(std::string_view a, std::string_view b) const {
                return a == b;
            }
        };

    private:
        std::unordered_map<std::string, Value, Hash, Equal> _map;
        Clock _clock;
    };
}

#include "kvs.inl"