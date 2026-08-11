#pragma once
#include "String.hpp"

namespace Genie {
	template <typename T> struct List {
		struct Node {
			Node* m_pNext;
			Node* m_pPrev;
			T value;
		};
		void* m_Heap;
		void* m_pFree;
		void* m_pBlocks;
		unsigned int m_nObjectSize;
		unsigned int m_nBlockSize;
		Node* m_pHead;
		Node* m_pTail;
		unsigned int m_nElements;

		inline void RemoveHead() {
			Node* oldHead = m_pHead;
			m_pHead = oldHead->m_pNext;
			if (m_pHead == nullptr) {
				m_pTail = nullptr;
			}
			else {
				m_pHead->m_pPrev = nullptr;
			}
			FreeNode(oldHead);
		}

		inline void FreeNode(Node* pNode) {
			pNode->~Node();
			if (pNode != nullptr) {
				*reinterpret_cast<void**>(pNode) = m_pFree;
				m_pFree = pNode;
			}
			m_nElements--;
		}
	};
};

static_assert(sizeof(Genie::List<const Genie::String>) == 0x20);