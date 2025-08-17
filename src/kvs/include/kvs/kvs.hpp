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
        // Инициализирует хранилище переданным множеством записей. Размер span может быть очень большим.
        // Также принимает абсиракцию часов (Clock) для возможности управления временем в тестах.
        explicit KVStorage(std::span<std::tuple<std::string /*key*/, std::string /*value*/, uint32_t /*ttl*/>> entries, Clock clock = Clock());

        ~KVStorage();

        // Присваивает по ключу key значение value.
        // Если ttl == 0, то время жизни записи - бесконечность, иначе запись должна перестать быть доступной через ttl секунд.
        // Безусловно обновляет ttl записи.
        void set(std::string key, std::string value, uint32_t ttl);

        // Удаляет запись по ключу key.
        // Возвращает true, если запись была удалена. Если ключа не было до удаления, то вернет false.
        bool remove(std::string_view key);

        // Полчует значение по ключу key. Если данного ключа нет, то вернет std::nullopt.
        std::optional<std::string> get(std::string_view key) const;

        // Возвращает следующие count записей начиная с key в порядке лексикографической сортировки ключей.
        // Пример: ("a", "val1"), ("b", "val2"), ("d", "val3"), ("e", "val4")
        // getManySorted("c", 2) -> ("d", "val3"), ("e", "val4")
        std::vector<std::pair<std::string, std::string>> getManySorted(std::string_view key, uint32_t count) const;

        // Удаляет протухшую запись из структуры и возвращает её. Если удалять нечего, то вернет std::nullopt.
        // Если на момент вызоваметодапротухло несколько записей,то можно удалить любую.
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