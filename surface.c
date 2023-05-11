#include "surface.h"

surfacePage_t currentSurfacePage = PAGE_INVALID;

inline void show_score(const score_t *pScore)
{
	if (NULL == pScore)
	{
		exit(-1);
	}

	system("cls");
	puts("╔════════════════╗");
	puts("║     扫雷       ║");
	puts("╠════════════════╣");
	puts("║  本次游玩成绩  ║");
	puts("╠════════════════╣");
	printf("║累计点击次数:%d次║\n", pScore -> clickCount);
	printf("║累计耗时:%5lld秒║\n", (GetTickCount64() - (pScore -> beginTick64)) / 1000 + 1);
	puts("╠════════════════╣");
	puts("╚════════════════╝");
	system("pause");
}

void show_win(const score_t *pScore)
{
	

	system("cls");
	fputs("(华丽的通关，随后渐渐传来一篇掌声...)\n", stdout);
	system("pause");

	system("cls");
	fputs("您以无人能敌的水平清理了这片雷区!\n", stdout);
	system("pause");

	system("cls");
	fputs("感谢您为世界和平做出的贡献!\n", stdout);
	system("pause");

	show_score(pScore);
}

void show_gameover(const score_t *pScore)
{
	system("cls");
	fputs("(Bomb一声巨响——————)\n", stdout);
	system("pause");

	system("cls");
	fputs("“什么B动静？”，巨大的声响吓得您虎躯一震...\n", stdout);
	system("pause");

	system("cls");
	fputs("当您抬头看到头上的那顶光环时，您似乎理解了一切......\n", stdout);
	system("pause");

	show_score(pScore);
}

void show_first_play_menu()
{
	system("cls");

	puts("╔════════════════╗");
	puts("║     扫雷       ║");
	puts("╠════════════════╣");
	puts("║1.开始游戏      ║");
	puts("║2.退出游戏      ║");
	puts("╠════════════════╣");
	puts("╚════════════════╝\n");

	// 设置GUI状态
	currentSurfacePage = PAGE_FIRST_PLAY;
}

void show_next_play_menu()
{
	system("cls");

	puts("╔════════════════╗");
	puts("║     扫雷       ║");
	puts("╠════════════════╣");
	puts("║1.再来一把      ║");
	puts("║2.退出游戏      ║");
	puts("╠════════════════╣");
	puts("╚════════════════╝");

	// 设置GUI状态
	currentSurfacePage = PAGE_NEXT_PLAY;
}

uint32_t get_click(uint32_t *pXPos, uint32_t *pYPos, score_t *pScore)
{
	if (NULL == pScore)
	{
		exit(-1);
	}

	/*
	fputs("提示>11 4代表清理第11行第4列的位置，5 14则代表清理第5行第14列的位置\n", stdout);
	printf("提示>当前点击次数: %d次\n", pScore -> clickCount);
	printf("提示>当前耗时: %lld秒\n", (0 == (pScore -> clickCount))?0:(GetTickCount64() - (pScore -> beginTick64)) / 1000 + 1);
	*/
	fputs("请输入想清理的位置>", stdout);

	fflush(stdin);
	char inputBuffer[128];
	gets_s(inputBuffer, 128);
	
	if (2 != sscanf(inputBuffer, "%d %d", pYPos, pXPos))
	{
		return 0;
	}

#ifdef _DEBUG
	printf("您输入的坐标: X:%d Y:%d\n",*pXPos, *pYPos);
	system("pause");
#endif

	// 点击累计次数加1
	pScore -> clickCount += 1;

	return 1;
}

menuChoice_t get_choice()
{
	uint32_t choiceId = 0;
	fputs("请输入选项>", stdout);

	fflush(stdin);

	char inputBuffer[128];
	gets_s(inputBuffer, 128);

	if (1 != sscanf(inputBuffer, "%d", &choiceId))
	{
		return CHOICE_INVALID;
	}

#ifdef _DEBUG
	printf("您输入的选项Id: %d\n", choiceId);
#endif

	switch (currentSurfacePage)
	{
	case PAGE_FIRST_PLAY:
		switch (choiceId)
		{
		case 1:
			return CHOICE_FIRST_PLAY;
		case 2:
			return CHOICE_EXIT;
		}
		break;
	case PAGE_NEXT_PLAY:
		switch (choiceId)
		{
		case 1:
			return CHOICE_NEXT_PLAY;
		case 2:
			return CHOICE_EXIT;
		}
		break;
	case PAGE_PLAYING:
		switch (choiceId)
		{
		case 1:
			return CHOICE_EXIT;
		case 2:
			return CHOICE_RESTART;
		}
		break;
	}

	return CHOICE_INVALID;
}

void show_already_cleaned()
{
	fputs("提示>这个地方已经被清理过了，找一个其他地方碰碰运气吧!\n", stdout);
	system("pause");
}

void show_invalid_click()
{
	fputs("提示>这个地方超出了雷区的范围!\n", stdout);
	system("pause");
}

void show_illegal_input()
{
	fputs("提示>我没能明白您的意思...\n请尝试换一种正确的方法告诉我您想要做什么:D\n", stdout);
	system("pause");
}

uint32_t show_page(const surfacePage_t page)
{
	switch (currentSurfacePage)
	{
	case PAGE_FIRST_PLAY:
		show_first_play_menu();
		break;
	case PAGE_NEXT_PLAY:
		show_next_play_menu();
		break;
	default:
		return 0;
	}

	return 1;
}

void show_bye()
{
	system("cls");
	puts("╔════════════════╗");
	puts("║     扫雷       ║");
	puts("╠════════════════╣");
	puts("║  感谢您的游玩  ║");
	puts("║     再见       ║");
	puts("╠════════════════╣");
	puts("╚════════════════╝");
	system("pause");
}