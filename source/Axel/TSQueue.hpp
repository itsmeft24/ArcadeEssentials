#pragma once
#include <mutex>
#include <queue>

namespace axel::util {
	template<typename T>
	class TSQueue {
		std::mutex m; std::queue<T> q;
	public:
		inline void push(T v) {
			std::lock_guard g(m);
			q.push(std::move(v));
		}
		inline bool pop(T& out) {
			std::lock_guard g(m);
			if (q.empty()) {
				return false;
			}
			out = std::move(q.front());
			q.pop();
			return true;
		}
	};
};