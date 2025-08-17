#include <gtest/gtest.h>
#include "kvs.hpp"
#include <chrono>

namespace {
	class Clock {
	public:
		int64_t increaseTime(int64_t seconds) {
			_startedTimeSeconds += seconds;
			return _startedTimeSeconds;
		}

		int64_t now() const {
			return _startedTimeSeconds;
		}
	private:
		inline static int64_t _startedTimeSeconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	};


    using KV = kvs::KVStorage<Clock>;
}

// ��������� ���������� ���������� ������� � ������ ������ now() ������ Clock
TEST(ClockTest, IncreaseTimeWorks) {
    Clock clock;

    int64_t initialTime = clock.now();
    ASSERT_GT(initialTime, 0);

    int64_t incrementFirst = 10;
    int64_t timeAfterFirstIncrease = clock.increaseTime(incrementFirst);

    ASSERT_EQ(timeAfterFirstIncrease, initialTime + incrementFirst);
    ASSERT_EQ(clock.now(), initialTime + incrementFirst);

    int64_t incrementSecond = 5;
    int64_t timeAfterSecondIncrease = clock.increaseTime(incrementSecond);

    ASSERT_EQ(timeAfterSecondIncrease, initialTime + incrementFirst + incrementSecond);
    ASSERT_EQ(clock.now(), timeAfterSecondIncrease);
}

// ��������� ������������ ������ ������� set � get, ������ kvs::KVStorage
TEST(KVStorageTest, SetAndGet) {
    Clock testClock;
    KV storage({}, testClock);

    const std::string key = "a";
    const std::string value = "val1";
    const uint32_t ttl = 0;

    storage.set(key, value, ttl);

    auto retrievedValue = storage.get(key);

    ASSERT_TRUE(retrievedValue.has_value());
    EXPECT_EQ(retrievedValue.value(), value);
}

// ��������� ������������ ������ ������ set ��� ����������
TEST(KVStorageTest, OverwriteValue) {
    Clock testClock;
    KV storage({}, testClock);

    const std::string key = "a";
    const std::string initialValue = "val1";
    const std::string updatedValue = "val2";
    const uint32_t ttl = 0;

    storage.set(key, initialValue, ttl);

    storage.set(key, updatedValue, ttl);

    auto retrievedValue = storage.get(key);

    ASSERT_TRUE(retrievedValue.has_value());
    EXPECT_EQ(retrievedValue.value(), updatedValue);
}


// ��������� ������������ ������ ������ remove ��� �������� ������������� �����
TEST(KVStorageTest, RemoveExistingKey) {
    Clock testClock;
    KV storage({}, testClock);

    const std::string key = "a";
    const std::string value = "val1";
    const uint32_t ttl = 0;

    storage.set(key, value, ttl);

    EXPECT_TRUE(storage.remove(key));
    EXPECT_FALSE(storage.get(key).has_value());
}

// ��������� ������������ ������ ������ remove ��� �������� ��������������� �����
TEST(KVStorageTest, RemoveNonExistingKey) {
    Clock testClock;
    KV storage({}, testClock);

    const std::string nonExistingKey = "val1";

    EXPECT_FALSE(storage.remove(nonExistingKey));
}

// ��������� ������������ ������ ��������� ��������� ������� ������ kvs::KVStorage
TEST(KVStorageTest, Expiration) {
    Clock testClock;
    KV storage({}, testClock);

    const std::string key = "a";
    const std::string value = "val1";
    const uint32_t ttlSeconds = 5;

    storage.set(key, value, ttlSeconds);

    testClock.increaseTime(10);

    auto result = storage.get(key);

    EXPECT_FALSE(result.has_value());
}

// ��������� ������������ ������ ��������� ��������� ������� ������ kvs::KVStorage, ��� ������������ ���������
TEST(KVStorageTest, NoExpirationTTLZero) {
    Clock testClock;
    KV storage({}, testClock);

    const std::string key = "a";
    const std::string value = "val1";
    const uint32_t ttl = 0;

    storage.set(key, value, ttl);

    testClock.increaseTime(1000);

    auto result = storage.get(key);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), value);
}


// ��������� ������������ ������ ������ getManySorted
TEST(KVStorageTest, GetManySorted) {
    Clock testClock;
    KV storage({}, testClock);

    storage.set("a", "val1", 0);
    storage.set("b", "val2", 0);
    storage.set("d", "val3", 0);
    storage.set("e", "val4", 0);

    auto results = storage.getManySorted("c", 2);

    ASSERT_EQ(results.size(), 2);

    EXPECT_EQ(results[0].first, "d");
    EXPECT_EQ(results[0].second, "val3");
    EXPECT_EQ(results[1].first, "e");
    EXPECT_EQ(results[1].second, "val4");
}

// ��������� ������������ ������ ������ removeOneExpiredEntry
TEST(KVStorageTest, RemoveOneExpiredEntry) {
    Clock testClock;
    KV storage({}, testClock);

    storage.set("a", "val1", 3);
    storage.set("b", "val2", 10);

    testClock.increaseTime(5);

    auto expiredEntry = storage.removeOneExpiredEntry();
    ASSERT_TRUE(expiredEntry.has_value());
    EXPECT_EQ(expiredEntry->first, "a");
    EXPECT_EQ(expiredEntry->second, "val1");

    auto expiredEntry2 = storage.removeOneExpiredEntry();
    EXPECT_FALSE(expiredEntry2.has_value());
}