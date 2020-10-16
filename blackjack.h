#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define VALUE_NUM 13 // Number of ranks
#define SUITE_NUM 4 // Number of suites

#define MAX_LENGTH 100 // Max line buffer length
#define SUITE_LENGTH 9 // Max string length for suite
#define VALUE_LENGTH 3 // Max string length for rank

#define LOG_FILE "log.txt" // Previuos game log file
#define DAT_FILE "stats.dat" // Overall statistics file

/* Linked list with card structure elements is a "card pool": the cards that are already
been dealt are linked to the list. There are two lists for player and dealer. Initially the list is empty.
suite - card suite, value - card rank
aceOne is used to dynamically adjust the value of Aces (more in sum function) */
typedef struct card_ {
	char suite[SUITE_LENGTH];
	char value[VALUE_LENGTH];
	bool aceOne;
	struct card_* next;
} card;

/* Structure that holds all the length and array limitations, data of possible cards */
typedef struct constLimits {
	const int maxLength;
	const int valueNum;
	const int suiteNum;
	const char value[VALUE_NUM][VALUE_LENGTH];
	const char suite[SUITE_NUM][SUITE_LENGTH];
} constValues;

/* Structure for holding global statistics of games */
typedef struct globalStats {
	int games;
	int win;
	int lose;
	int push;
	int bust;
	int quit;
	int bjPush;
	int blackjack;
	int maxCards;
} stats;

char* readLine(char* userInput, const int size);
bool readOptionNumber(char* userInput, const int size, int* selection);
void printMenu(void);
card* dealCard(card** dealer, card** player, const constValues limit);
card* generateCard(const constValues limit);
void empty(card** ppn);
void printCurrentCards(card** dealer, card** player);
int game(FILE* log, card** dealer, card** player, stats* global, const constValues limits, char* userInput);
int calculateValue(card** ppn);
void pressEnter(char* buffer, const int size);
bool writeLog(FILE* log, card** dealer, card** player, int sumPlayer, int sumDealer);
int viewLog(FILE* log, char* line, const int size);
bool readStats(FILE* data, stats* global);
void showStats(const stats global);
void showAchievements(const stats global);