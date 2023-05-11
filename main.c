#include "stdafx.h"
#include "surface.h"

extern surfacePage_t currentSurfacePage;

char *hashStatusToText[] = {
	"   ",
	" 1 ",
	" 2 ",
	" 3 ",
	" 4 ",
	" 5 ",
	" 6 ",
	" 7 ",
	" 8 ",
	" ? ",
	" X ",
	" E "
};

posStatus_t hashUint32ToStatus[] = {
	POS_EMPTY,
	POS_1,
	POS_2,
	POS_3,
	POS_4,
	POS_5,
	POS_6,
	POS_UNKNOWN,
	POS_MINE
};

mineMap_t *pMineMap = NULL;
score_t glScore = {0};
uint32_t mineNum = 0;
uint32_t glIsOutRoundCleaned = 0;

uint32_t initialize_score(score_t *pScore)
{
	if (NULL == pScore)
	{
		exit(-1);
	}

	pScore -> clickCount = 0;
	pScore -> beginTick64 = 0;
	pScore -> endTick64 = 0;
}


/**
	@fn 刷新棋盘
	@return 始终返回1
**/
uint32_t update_mine_map()
{
	// 清屏
	system("cls");

	const uint32_t width = pMineMap -> pMapRect -> width;
	const uint32_t heigh = pMineMap -> pMapRect -> width;

	const uint32_t col = width + 1;
	const uint32_t row = heigh + 1;

	for (uint32_t i = 0; i < width; i++)
	{
		printf("%3d", i + 1);
	}
	fputs("\n", stdout);

	fputs("+", stdout);
	for (uint32_t i = 0; i < width; i++)
	{
		fputs("---", stdout);
	}
	fputs("+\n", stdout);

	for (uint32_t yPos = 1; yPos < col; yPos++)
	{

		fputs("|", stdout);

		for (uint32_t xPos = 1; xPos < row; xPos++)
		{
			fputs(hashStatusToText[get_pos_status(pMineMap -> pMapRect, pMineMap -> pUserBoard, xPos, yPos)], stdout);
		}

		printf("|%d\n", yPos);
		
	}

	fputs("+", stdout);
	for (uint32_t i = 0; i < width; i++)
	{
		fputs("---", stdout);
	}
	fputs("+\n", stdout);
	

	fputs("这是MineBoard状态:\n", stdout);

	fputs("+", stdout);
	for (uint32_t i = 0; i < width; i++)
	{
		fputs("---", stdout);
	}
	fputs("+\n", stdout);

	for (uint32_t yPos = 1; yPos < col; yPos++)
	{
		fputs("|", stdout);

		for (uint32_t xPos= 1; xPos < row; xPos++)
		{
			fputs(hashStatusToText[get_pos_status(pMineMap -> pMapRect, pMineMap -> pMineBoard, xPos, yPos)], stdout);
		}

		printf("|%d\n", yPos);
	}

	fputs("+", stdout);
	for (uint32_t i = 0; i < width; i++)
	{
		fputs("---", stdout);
	}
	fputs("+\n", stdout);

	fputs("提示>11 4代表清理第11行第4列的位置，5 14则代表清理第5行第14列的位置\n", stdout);
	printf("提示>本局雷数: %d枚\n",  (0 == glScore.clickCount)?COL:mineNum);
	printf("提示>当前耗时: %lld秒\n", (0 == glScore.clickCount)?0:(GetTickCount64() - (glScore.beginTick64)) / 1000 + 1);
	printf("提示>当前点击次数: %d次\n", glScore.clickCount);

	return 1;
}

/**
	@fn 分配Board的内存并进行初始化
	@param pRect 层的尺寸，如果传入空指针将会中断运行
	#param defaultValue 用于填充层的值
	@return 返回一个指向Board_t结构的指针，如果失败返回NULL
**/
uint32_t *create_board(const rect_t *pRect, const uint32_t defaultValue)
{
	if (NULL == pRect)
	{
		exit(-1);
	}

	uint32_t width = pRect -> width;
	uint32_t heigh = pRect -> heigh;  
	
	// 分配内存
	uint32_t *pBoard;
	pBoard = calloc(heigh * width, sizeof(uint32_t));
	if(NULL == pBoard)
	{
		return NULL;
	}

	// 填充
	for (uint32_t i = 0; i < width; i++)
	{
		for (uint32_t j = 0; j < heigh; j++)
		{
			(pBoard + (width * i))[j] = defaultValue;
		}
	}
	return pBoard;
}

/**
	@fn 分配棋盘结构的内存
	@param pRect 指定棋盘结构的形状，传入空指针将中断运行
	@return 返回一个指向mineMap_t结构的指针，如果失败返回NULL
**/
mineMap_t *create_map(const rect_t *pRect)
{
	mineMap_t *pMineMap = NULL;

	// 分配MineMap内存
	pMineMap = malloc(sizeof(mineMap_t));
	if(NULL == pMineMap)
	{
		// MineMap内存分配失败
		exit(-1);
	}

	// 分配Rect内存
	pMineMap -> pMapRect = malloc(sizeof(rect_t));
	if(NULL == pMineMap -> pMapRect)
	{
		// Rect内存分配失败
		return NULL;
	}

	// 生成pRect指向矩形的副本
	memcpy(pMineMap -> pMapRect, pRect, sizeof(rect_t));

	pMineMap -> pMineBoard = create_board(pMineMap -> pMapRect, POS_EMPTY);
	if(NULL == pMineMap -> pMineBoard)
	{
		return NULL;
	}

	pMineMap -> pUserBoard = create_board(pMineMap -> pMapRect, POS_UNKNOWN);
	if (NULL == pMineMap -> pUserBoard)
	{
		return NULL;
	}

	return pMineMap;
}

/**
	@fn 随机埋雷
	@return 函数始终返回1
**/
uint32_t bury_mine(const rect_t *const pRect, uint32_t *const pBoard, const uint32_t xClickPos, const uint32_t yClickPos)
{
	// 检查指针合法性
	if(NULL == pRect || NULL == pBoard)
	{
		exit(-1);
	}

	uint32_t width = pRect -> width;
	uint32_t heigh = pRect -> heigh;

	// 设置随机数种子
	srand((unsigned int)time(NULL));

#ifdef NO_REGULARLY_RANDOM

	struct {
		uint32_t xPos;
		uint32_t yPos;
	} minePosArray[MINE_NUM] = { 0 };

	for (uint32_t i = 0; i < MINE_NUM; i ++)
	{
		uint32_t res = 0;
		while (res)
		{
			uint32_t xPos = (rand() % width) + 1;
			uint32_t yPos = (rand() % heigh) + 1;

			if(xPos == xClickPos && yPos == yClickPos)
			{
				// 与用户点击位置重复，重新生成
				continue;
			}

			uint32_t isRepeat = 0;

			for (uint32_t j = 0; j < i; j++)
			{
				if (xPos == minePosArray[j].xPos || yPos == minePosArray[j].yPos)
				{
					isRepeat = !isRepeat;
					break;
				}
			}

			if (!isRepeat)
			{
				set_pos_status(pRect, pBoard, xPos, yPos, POS_MINE);
				mineNum += 1;
				res = !res;
			}
		}
	}

#else

	for (uint32_t xPos = 1; xPos < width + 1; xPos++)
	{
		// i * sizeof(uint32_t) 代表访问pArray这个二维数组的第i行
		// rand() % (pRect -> width + 1) 代表的则是二维数组的随机列
		// 所以这句话的意思是访问第i行的随机一个列并赋值
		uint32_t yPos = (rand() % heigh) + 1;

		// 如果用户点击的坐标与生成的地雷的坐标相同则重新生成地雷的位置
		if (xClickPos == xPos)
		{
			while (yClickPos == yPos)
			{
				yPos = (rand() % heigh) + 1;
			}
		}

		set_pos_status(pRect, pBoard, xPos, yPos, POS_MINE);
		mineNum += 1;
	}

#endif

	return 1;
}

/**
	@fn 初始化用户层(O(n^2), n = 16 * 16) 
**/
uint32_t initialize_user_board(const mineMap_t *pMineMap)
{
	if (NULL == pMineMap)
	{
		exit(-1);
	}

	uint32_t mineNum = 0;

	for (uint32_t xPos = pMineMap -> pMapRect -> width; xPos > 0; xPos--)
	{
		for (uint32_t yPos = pMineMap -> pMapRect -> heigh; yPos > 0; yPos--)
		{
			if(get_pos_status(pMineMap -> pMapRect, pMineMap -> pMineBoard, xPos, yPos) != POS_MINE)
			{
				mineNum = get_around_mine_num(pMineMap, xPos, yPos);
				set_pos_status(pMineMap -> pMapRect, pMineMap -> pMineBoard, xPos, yPos, hashUint32ToStatus[mineNum]);
			}
		}
	}
	return 1;
}

/**
	@fn 判断坐标是否越界
**/
int32_t is_out_of_range(const rect_t *pRect, uint32_t xPos, uint32_t yPos)
{
	if (NULL == pRect)
	{
		exit(-1);
	}

	if (xPos < 1 || xPos > pRect -> width || yPos < 1 || yPos > pRect -> heigh)
	{
		// 坐标越界
		return 1;
	}

	return 0;
}


uint32_t set_pos_status(const rect_t *const pRect, uint32_t *const pBoard, const uint32_t xPos, const uint32_t yPos, const posStatus_t posStatus)
{
	if (NULL == pRect || NULL == pBoard)
	{
		exit(-1);
	}

	if (is_out_of_range(pRect, xPos, yPos))
	{
		// 坐标越界
		exit(-1);
	}

	(pBoard + (yPos - 1) * (pRect -> width))[xPos - 1] = posStatus;

	return 1;
}


posStatus_t get_pos_status(const rect_t *pRect, const uint32_t *const pBoard, const uint32_t xPos, const uint32_t yPos)
{
	if (NULL == pRect || NULL == pBoard)
	{
		exit(-1);
	}

	if (is_out_of_range(pRect, xPos, yPos))
	{
		// 坐标越界

#ifdef _DEBUG
		printf("[警告]越界的坐标无法获取状态\n");
#endif

		return POS_INVALID;
	}

	return (pBoard + (yPos - 1) * (pRect -> width))[xPos - 1];
}

uint32_t get_around_mine_num(const mineMap_t *const pMineMap, const uint32_t xCenterPos, const uint32_t yCenterPos)
{
	if (NULL == pMineMap)
	{
		return -1;
	}

	uint32_t count = 0;

	uint32_t posList[8][2];

	posList[0][0] = xCenterPos - 1;
	posList[0][1] = yCenterPos - 1;

	posList[1][0] = xCenterPos - 1;
	posList[1][1] = yCenterPos;

	posList[2][0] = xCenterPos - 1;
	posList[2][1] = yCenterPos + 1;

	posList[3][0] = xCenterPos;
	posList[3][1] = yCenterPos - 1;

	posList[4][0] = xCenterPos;
	posList[4][1] = yCenterPos + 1;

	posList[5][0] = xCenterPos + 1;
	posList[5][1] = yCenterPos - 1;

	posList[6][0] = xCenterPos + 1;
	posList[6][1] = yCenterPos;

	posList[7][0] = xCenterPos + 1;
	posList[7][1] = yCenterPos + 1;

	uint32_t mineNumCount = 0;

	for (uint32_t i = 0; i < 8; i++)
	{
		if(POS_MINE == get_pos_status(pMineMap -> pMapRect, pMineMap -> pMineBoard, posList[i][0], posList[i][1]))
		{
			mineNumCount += 1;
		}
	}

	return mineNumCount;
}

uint32_t clean_mine(const mineMap_t *pMineMap, uint32_t xPos, uint32_t yPos)
{
	if (NULL == pMineMap)
	{
		exit(-1);
	}

	if (POS_MINE == get_pos_status(pMineMap -> pMapRect, pMineMap -> pMineBoard, xPos, yPos))
	{
		// 踩到雷了(悲
		return 0;
	}

#ifdef CLEAN_OUT_ROUND
	if (!glIsOutRoundCleaned)
	{
		clean_out_round(pMineMap);
	}
#endif

	clean_empty(pMineMap, xPos, yPos, DIRECTION_RIGHT | DIRECTION_UP | DIRECTION_LEFT | DIRECTION_DOWN);

	return 1;
}

inline void clean_pos(const mineMap_t* const pMineMap, const uint32_t xPos, const uint32_t yPos)
{
	posStatus_t posStatus = get_pos_status(pMineMap->pMapRect, pMineMap->pMineBoard, xPos, yPos);
	if (posStatus != POS_MINE)
	{
		if (POS_UNKNOWN == get_pos_status(pMineMap->pMapRect, pMineMap->pUserBoard, xPos, yPos))
		{
			set_pos_status(pMineMap->pMapRect, pMineMap->pUserBoard, xPos, yPos, posStatus);
		}
	}
}

uint32_t clean_out_round(const mineMap_t* const pMineMap)
{
	if (NULL == pMineMap)
	{
		exit(-1);
	}

	const uint32_t width = pMineMap -> pMapRect -> width;
	const uint32_t heigh = pMineMap -> pMapRect -> width;
	uint32_t x = 1;
	uint32_t y = 1;
	for (; x < width; x++)
	{
		clean_pos(pMineMap, x, y);
	}

	for (; y < heigh; y++)
	{
		clean_pos(pMineMap, x, y);
	}

	for (; x > 1; x--)
	{
		clean_pos(pMineMap, x, y);
	}

	for (; y > 1; y--)
	{
		clean_pos(pMineMap, x, y);
	}

	glIsOutRoundCleaned = 1;
	return 1;
}

uint32_t clean_empty(const mineMap_t *const pMineMap, const uint32_t xPos, const uint32_t yPos, const direction_t direction)
{
	if (is_out_of_range(pMineMap -> pMapRect, xPos, yPos))
	{
		// 触碰到边界后停止递归
		return 1;
	}

	if (POS_UNKNOWN != get_pos_status(pMineMap -> pMapRect, pMineMap -> pUserBoard, xPos, yPos))
	{
		// 触碰到已经清扫过的鸽子
		return 1;
	}

	posStatus_t posStatus = get_pos_status(pMineMap -> pMapRect, pMineMap -> pMineBoard, xPos, yPos);

	// 将mineBoard层指定坐标显示到userBoard层
	set_pos_status(pMineMap -> pMapRect, pMineMap -> pUserBoard, xPos, yPos, posStatus);

	if (posStatus != POS_EMPTY)
	{
		// 触碰到附近有雷的格子后停止递归
		return 1;
	}

	if (direction & DIRECTION_RIGHT)
	{
		clean_empty(pMineMap, xPos + 1, yPos, DIRECTION_RIGHT);
		clean_empty(pMineMap, xPos, yPos - 1, DIRECTION_UP);
		clean_empty(pMineMap, xPos, yPos + 1, DIRECTION_DOWN);
	}

	if (direction & DIRECTION_LEFT)
	{
		clean_empty(pMineMap, xPos - 1, yPos, DIRECTION_LEFT);
		clean_empty(pMineMap, xPos, yPos - 1, DIRECTION_UP);
		clean_empty(pMineMap, xPos, yPos + 1, DIRECTION_DOWN);
	}

	if (direction & DIRECTION_UP)
	{
		clean_empty(pMineMap, xPos, yPos - 1, DIRECTION_UP);
		clean_empty(pMineMap, xPos - 1, yPos, DIRECTION_LEFT);
		clean_empty(pMineMap, xPos + 1, yPos, DIRECTION_RIGHT);
	}

	if (direction & DIRECTION_DOWN)
	{
		clean_empty(pMineMap, xPos, yPos + 1, DIRECTION_DOWN);
		clean_empty(pMineMap, xPos - 1, yPos, DIRECTION_LEFT);
		clean_empty(pMineMap, xPos + 1, yPos, DIRECTION_RIGHT);
	}

	return 1;
}

/**
	@fn 初始化一些资源
	@return 始终返回1
**/
int initialize_resource()
{
	rect_t boardRect = {COL, ROW};
	pMineMap = create_map(&boardRect);
	if (NULL == pMineMap)
	{
		pause_and_exit("[错误]内存初始化失败\n", -1);
	}
	return 1;
}
	
/**
	@fn 检查是否清理完所有地雷
	@return 如果地雷清理干净返回1，否则返回0
**/
uint32_t is_mine_all_cleaned(const mineMap_t *const pMineMap)
{
	const uint32_t width = pMineMap -> pMapRect -> width;
	const uint32_t heigh = pMineMap -> pMapRect -> heigh;

	uint32_t unknownNum = 0;
	for (uint32_t yPos = 1; yPos <= heigh; yPos++)
	{
		for (uint32_t xPos = 1; xPos <= width; xPos++)
		{
			if (POS_UNKNOWN == get_pos_status(pMineMap -> pMapRect, pMineMap -> pUserBoard, xPos, yPos))
			{
				/*
				if(POS_MINE != get_pos_status(pMineMap -> pMapRect, pMineMap -> pMineBoard, xPos, yPos))
				{
					return 0;
				}
				*/
				unknownNum += 1;
				if (unknownNum > mineNum)
				{
					return 0;
				}
			}
		}
	}
	return 1;
}

/**
	@fn 游戏主体循环
	@return 如果游戏通关返回1，
			中途失败则返回0
**/
uint32_t game_process()
{
	uint32_t xPos, yPos = 0;

	update_mine_map();

	// 获取玩家的点击，并据此随机埋雷
	while (!get_click(&xPos, &yPos, &glScore))
	{
		show_illegal_input();
		update_mine_map();
	}

	bury_mine(pMineMap -> pMapRect, pMineMap -> pMineBoard, xPos, yPos);

	update_mine_map();
	
	// 在随机埋雷后初始化雷周围的数字
	initialize_user_board(pMineMap);

	// 扫雷
	uint32_t isFailed = clean_mine(pMineMap, xPos, yPos);

	// 刷新棋盘
	update_mine_map();

	uint32_t isAllCleaned = is_mine_all_cleaned(pMineMap);

	glScore.beginTick64 = GetTickCount64();

	while (!isAllCleaned)
	{
		if (get_click(&xPos, &yPos, &glScore))
		{
			if (is_out_of_range(pMineMap -> pMapRect, xPos, yPos))
			{
				show_invalid_click();
			}
			else if (POS_UNKNOWN != get_pos_status(pMineMap -> pMapRect, pMineMap -> pUserBoard, xPos, yPos))
			{
				show_already_cleaned();
			}
			else
			{
				isFailed = clean_mine(pMineMap, xPos, yPos);

				if (0 == isFailed)
				{
					return 0;
				}

				isAllCleaned = is_mine_all_cleaned(pMineMap);
			}

		}
		else
		{
			// 刷新棋盘
			show_illegal_input();
		}


		// 刷新棋盘
		update_mine_map();
	}

	glScore.endTick64 = GetTickCount64();

	// 通关
	// 地雷全部清理完毕，游戏结束
	return 1;
}

uint32_t is_invalid_choice(menuChoice_t choice)
{
	return choice == CHOICE_INVALID;
}

menuChoice_t get_choice_until_no_invalid(const surfacePage_t page)
{
	menuChoice_t choice = get_choice();
	while (is_invalid_choice(choice))
	{
		show_illegal_input();
		show_page(page);
		choice = get_choice();
	}

	return choice;
}

/**
	@fn 入口函数
**/
int main(int argc, char **argv)
{
	show_first_play_menu();

	
	//menuChoice_t choice = get_choice();

	menuChoice_t choice = get_choice_until_no_invalid(currentSurfacePage);

	while (choice != CHOICE_EXIT)
	{
		switch(choice)
		{
		case CHOICE_FIRST_PLAY:
		case CHOICE_NEXT_PLAY:
			initialize_resource();
			initialize_score(&glScore);
			if (game_process())
			{
				show_win(&glScore);
			}
			else
			{
				show_gameover(&glScore);
			}
			break;
		/*
		case CHOICE_INVALID:
			show_illegal_input();
			break;
		default:
			show_illegal_input();
			break;
		*/
		}

		show_next_play_menu();

		choice = get_choice_until_no_invalid(currentSurfacePage);
	}

	show_bye();
	
	return 0;
}