#pragma once

#include "stdafx.h"

/**
	@fn 显示通关界面
**/
void show_win(const score_t *pScore);

/**
	@fn 显示失败界面
**/
void show_gameover(const score_t *pScore);

/**
	@fn 显示开始菜单
**/
void show_first_play_menu();

/**
	@fn 显示下一轮游玩菜单
**/
void show_next_play_menu();

/**
	@fn 获取玩家的一次点击, clickCount + 1
	@return 始终返回1
**/
uint32_t get_click(uint32_t *pXPos, uint32_t *pYPos, score_t *pScore);

/**
	@fn 获取选项
	@return 返回一个menuChoice_t变量，表示玩家输入的选项
**/
menuChoice_t get_choice();

/**
	@fn 显示已清理的提示信息
**/
void show_already_cleaned();

/**
	@fn 显示无效点击的提示信息
**/
void show_invalid_click();

/**
	@fn 显示无效输入的提示信息
**/
void show_illegal_input();

/**
	@fn 显示无效输入的提示信息
	@page 被显示的页面
	@return 传入一个有效页返回1，如果页是无效的返回0
**/
uint32_t show_page(const surfacePage_t page);

/**
	@fn 显示游玩结束页面
**/
void show_bye();