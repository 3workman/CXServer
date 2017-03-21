#include "stdafx.h"
#include "UnitTest.h"
#include <string>

namespace unittest
{
	struct ListNode
	{
		std::string			file;
		int					line = 0;
		UnitTest::TestFunc	func = nullptr;
		ListNode*			next = nullptr;
	};
	ListNode*	g_head = nullptr;
	ListNode**	g_tail = &g_head;

	UnitTest::UnitTest(const char* file, int line, TestFunc testFunc)
	{
		auto node = new ListNode;
		node->file.assign(file);
		node->line = line;
		node->func = testFunc;
		*g_tail = node;
		g_tail = &node->next;
	}
	void UnitTest::RunAllTests()
	{
		auto current = g_head;
		g_head = nullptr;
		g_tail = &g_head;

		while (current)
		{
			printf("\nUnitTest => %s(%d)\n", current->file.c_str(), current->line);
			current->func();
			auto temp = current;
			current = current->next;
			delete temp;
		}
	}
}